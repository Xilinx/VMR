/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* © Copyright 2019 Xilinx, Inc. All rights reserved.                                           */
/*                                                                                              */
/* This file contains confidential and proprietary information of Xilinx, Inc.                  */
/* and is protected under U.S. and international copyright and other intellectual               */
/* property laws.                                                                               */
/*                                                                                              */
/*                                                                                              */
/* DISCLAIMER                                                                                   */
/*                                                                                              */
/* This disclaimer is not a license and does not grant any rights to the materials              */
/* distributed herewith. Except as otherwise provided in a valid license issued                 */
/* to you by Xilinx, and to the maximum extent permitted by applicable law:                     */
/*                                                                                              */
/* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS,                          */
/* AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED,                 */
/* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,                    */
/* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and                                 */
/*                                                                                              */
/* (2) Xilinx shall not be liable (whether in contract or tort, including negligence,           */
/* or under any other theory of liability) for any loss or damage of any kind or                */
/* nature related to, arising under or in connection with these materials,                      */
/* including for any direct, or any indirect, special, incidental, or consequential             */
/* loss or damage (including loss of data, profits, goodwill, or any type of loss or            */
/* damage suffered as a result of any action brought by a third party) even if such             */
/* damage or loss was reasonably foreseeable or Xilinx had been advised of the                  */
/* possibility of the same.                                                                     */
/*                                                                                              */
/*                                                                                              */
/* CRITICAL APPLICATIONS                                                                        */
/*                                                                                              */
/* Xilinx products are not designed or intended to be fail-safe, or for use in                  */
/* any application requiring fail-safe performance, such as life-support or safety              */
/* devices or systems, Class III medical devices, nuclear facilities, applications              */
/* related to the deployment of airbags, or any other applications that could lead              */
/* to death, personal injury, or severe property or environmental damage (individually          */
/* and collectively, "Critical Applications"). Customer assumes the sole risk and               */
/* liability of any use of Xilinx products in Critical Applications, subject                    */
/* only to applicable laws and regulations governing limitations on product liability.          */
/*                                                                                              */
/*                                                                                              */
/* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT ALL TIMES.     */
/*                                                                                              */
/*----------------------------------------------------------------------------------------------*/


/* COMMON_DIST */


/*
 *
 *  RCS Keyword Metadata
 *
 *  $Change: 3097075 $
 *  $Date: 2021/01/14 $
 *  $Revision: #4 $
 *
 */

#include "cmc_transport_suc.h"

/* handle to the watchpoint context */
extern CMC_WATCHPOINT_CONTEXT_TYPE                      WatchpointContext;


 /***************************************************************/
 /* Local Functions                                             */
 /***************************************************************/

 /* inline functions */
static inline uint16_t Read_U16_LE(uint8_t* buf)
{
    return (buf[0] << 0) | (buf[1] << 8);
}


static inline void Write_U16_LE(uint16_t data, uint8_t* buf)
{
    buf[0] = data >> 0;
    buf[1] = data >> 8;
}


static inline uint16_t Read_U16_BE(uint8_t* buf)
{
    return (buf[0] << 8) | (buf[1] << 0);
}


static inline void Write_U16_BE(uint16_t data, uint8_t* buf)
{
    buf[0] = data >> 8;
    buf[1] = data >> 0;
}



/***************************************************************/
/* Global Functions                                            */
/***************************************************************/

void TRANSPORT_CMC_SUC_Initialize(void* pUserContext)
{
    TRANSPORT_CMC_SUC_CONTEXT_TYPE* pContext = (TRANSPORT_CMC_SUC_CONTEXT_TYPE*)pUserContext;

    /* RX Flow */
    pContext->rxFlow.idx = 0;
    pContext->rxFlow.state = IDLE;
    cmcMemSet(pContext->rxFlow.header, '\0', SUC_TRANSPORT_HEADER_LEN);
    pContext->rxFlow.packet.dataLen = 0;
    cmcMemSet(pContext->rxFlow.packet.data, '\0', SUC_TRANSPORT_MAX_PACKET_LEN);

    /* TX Flow */
    pContext->txFlow.idx = 0;
    pContext->txFlow.state = IDLE;
    cmcMemSet(pContext->txFlow.header, '\0', SUC_TRANSPORT_HEADER_LEN);
    pContext->txFlow.packet.dataLen = 0;
    cmcMemSet(pContext->txFlow.packet.data, '\0', SUC_TRANSPORT_MAX_PACKET_LEN);
}


bool TRANSPORT_CMC_SUC_DeframeTryAdd(void* pUserContext, char AnyCharacter)
{
    TRANSPORT_CMC_SUC_CONTEXT_TYPE* pContext = (TRANSPORT_CMC_SUC_CONTEXT_TYPE*)pUserContext;
    bool ret = false;

    uint8_t ch = (uint8_t)AnyCharacter;

    switch (pContext->rxFlow.state)
    {
    case IDLE:
        if (ch == SUC_TRANSPORT_START_BYTE)
        {
            pContext->rxFlow.header[0] = SUC_TRANSPORT_START_BYTE;
            pContext->rxFlow.state = RX_HEADER;
            pContext->rxFlow.idx = 1;
        }
        else
        {
            Watch_Inc(&WatchpointContext, W_SUC_TRANSPORT_RX_INVALID_START_BYTE);
        }
        break;

    case RX_HEADER:
        pContext->rxFlow.header[pContext->rxFlow.idx++] = ch;
        if (pContext->rxFlow.idx == SUC_TRANSPORT_HEADER_LEN)
        {
            uint16_t crc = pContext->pFN_Calculate_CRC16_CCITT(0, 0, (char*)pContext->rxFlow.header, SUC_TRANSPORT_HEADER_LEN);
            if (crc == 0)
            {
                SUC_TRANSPORT_FLAGS_TYPE flags;
                cmcMemCopy((char*)&flags, (char*)&pContext->rxFlow.header[SUC_TRANSPORT_HDR_FLAGS_OFF], sizeof(uint8_t));
                unsigned int id = flags.streamId;
                unsigned int len = Read_U16_LE(&pContext->rxFlow.header[SUC_TRANSPORT_HDR_LENGTH_OFF]);

                /*
                 * valid packet, now check that it can be handled.  If it is intended
                 * for a stream already processing or there's no receiver, drop it.
                 */
                if ((flags.version == SUC_TRANSPORT_VERSION) &&
                    (id == SUC_TRANSPORT_STREAM_ID) &&
                    (len <= SUC_TRANSPORT_MAX_PACKET_LEN))
                {
                    pContext->rxFlow.packet.dataLen = len;
                    pContext->rxFlow.idx = 0;
                    if (len != 0)
                    {
                        pContext->rxFlow.state = RX_PAYLOAD;
                    }
                    else
                    {
                        /* deal with zero payload message*/
                        pContext->rxFlow.state = RX_CRC;
                    }
                }
                else
                {
                    Watch_Inc(&WatchpointContext, W_SUC_TRANSPORT_RX_FAILED_VALIDATION);
                    pContext->rxFlow.idx = len + SUC_TRANSPORT_PAYLOAD_CRC_LEN;
                    pContext->rxFlow.state = RX_DROP;
                }
            }
            else
            {
                /* failed CRC check */
                Watch_Inc(&WatchpointContext, W_SUC_TRANSPORT_RX_HEADER_CRC_FAILED);
                pContext->rxFlow.state = IDLE;
            }
        }
        break;

    case RX_PAYLOAD:
        pContext->rxFlow.packet.data[pContext->rxFlow.idx++] = ch;
        if (pContext->rxFlow.idx >= pContext->rxFlow.packet.dataLen)
        {
            pContext->rxFlow.idx = 0;
            pContext->rxFlow.state = RX_CRC;
        }
        break;

    case RX_CRC:
        /* re-use header for CRC capture */
        pContext->rxFlow.header[pContext->rxFlow.idx++] = ch;
        if (pContext->rxFlow.idx == SUC_TRANSPORT_PAYLOAD_CRC_LEN)
        {
            uint16_t crc = pContext->pFN_Calculate_CRC16_CCITT(0, 0, (char*)&pContext->rxFlow.packet.data, pContext->rxFlow.packet.dataLen);

            if (Read_U16_BE(&pContext->rxFlow.header[0]) == crc)
            {
                /* copy into the deframer */
                cmcMemCopy(pContext->pDeframerContext->Element, (char*)&pContext->rxFlow.packet.data, pContext->rxFlow.packet.dataLen);

                /* set return to show message now available */
                ret = true;
            }
            else
            {
                /* failed CRC payload check */
                Watch_Inc(&WatchpointContext, W_SUC_TRANSPORT_RX_PAYLOAD_CRC_FAILED);
            }
            pContext->rxFlow.state = IDLE;
        }
        break;

    case RX_DROP:
        pContext->rxFlow.idx--;
        if (pContext->rxFlow.idx == 0)
        {
            pContext->rxFlow.state = IDLE;
        }
        break;

    default:
        /* invalid state */
        Watch_Inc(&WatchpointContext, W_SUC_TRANSPORT_RX_INVALID_STATE);
        break;
    }

    return ret;
}


void TRANSPORT_CMC_SUC_FrameMessage(void* pUserContext, uint8_t* pMessage, uint8_t* pMessageLength, uint8_t* pFramedMessage, uint8_t* pFramedMessageLength)
{
    TRANSPORT_CMC_SUC_CONTEXT_TYPE* pContext = (TRANSPORT_CMC_SUC_CONTEXT_TYPE*)pUserContext;
   
    /* CMC data and length passed in */
    pContext->txFlow.packet.dataLen = *pMessageLength;
    cmcMemCopy((char*)pContext->txFlow.packet.data, (char*)pMessage, *pMessageLength);

    /* build up the header */
    pContext->txFlow.header[SUC_TRANSPORT_HDR_START_OFF] = SUC_TRANSPORT_START_BYTE;
    Write_U16_LE(pContext->txFlow.packet.dataLen, &pContext->txFlow.header[SUC_TRANSPORT_HDR_LENGTH_OFF]);

    SUC_TRANSPORT_FLAGS_TYPE flags;
    flags.streamId = SUC_TRANSPORT_STREAM_ID;
    flags.reserved = SUC_TRANSPORT_RESERVED;
    flags.version = SUC_TRANSPORT_VERSION;
    cmcMemCopy((char*)&pContext->txFlow.header[SUC_TRANSPORT_HDR_FLAGS_OFF], (char*)&flags, sizeof(uint8_t));

    /* Calculate the CRC */
    uint16_t crc = pContext->pFN_Calculate_CRC16_CCITT(pContext, 0, (char*)pContext->txFlow.header, SUC_TRANSPORT_HEADER_LEN - SUC_TRANSPORT_HDR_CRC_LEN);
    Write_U16_BE(crc, &pContext->txFlow.header[SUC_TRANSPORT_HDR_CRC_OFF]);

    pContext->txFlow.idx = 0;
    pContext->txFlow.state = TX_HEADER;

    uint32_t bytesInFrame = *pMessageLength + SUC_TRANSPORT_HEADER_LEN + SUC_TRANSPORT_PAYLOAD_CRC_LEN;
    uint32_t pos;

    for (pos = 0; pos < bytesInFrame; pos++)
    {
        switch (pContext->txFlow.state)
        {

        case TX_HEADER:

            pFramedMessage[pos] = pContext->txFlow.header[pContext->txFlow.idx++];

            if (pContext->txFlow.idx == SUC_TRANSPORT_HEADER_LEN)
            {
                pContext->txFlow.idx = 0;
                if (pContext->txFlow.packet.dataLen > 0)
                {
                    pContext->txFlow.state = TX_PAYLOAD;
                }
                else
                {
                    Write_U16_LE(0, &pContext->txFlow.header[0]);
                    pContext->txFlow.state = TX_CRC;
                }
            }
            break;

        case TX_PAYLOAD:

            pFramedMessage[pos] = pContext->txFlow.packet.data[pContext->txFlow.idx++];

            if (pContext->txFlow.idx == pContext->txFlow.packet.dataLen)
            {
                uint16_t crc = pContext->pFN_Calculate_CRC16_CCITT(0, 0, (char*)pContext->txFlow.packet.data, pContext->txFlow.packet.dataLen);
                Write_U16_BE(crc, &pContext->txFlow.header[0]);
                pContext->txFlow.idx = 0;
                pContext->txFlow.state = TX_CRC;
            }
            break;

        case TX_CRC:

            pFramedMessage[pos] = pContext->txFlow.header[pContext->txFlow.idx++];

            if (pContext->txFlow.idx == SUC_TRANSPORT_PAYLOAD_CRC_LEN)
            {
                /* return the total bytes in the frame now */
                *pFramedMessageLength = bytesInFrame;

                /* all bytes have been trasnmitted */
                pContext->txFlow.state = IDLE;
            }
            break;

        case IDLE:
        default:
            /* shouldn't get into this state */
            Watch_Inc(&WatchpointContext, W_SUC_TRANSPORT_TX_INVALID_STATE);
            break;
        }
    }
}


void TRANSPORT_CMC_SUC_DeframeReset(void* pUserContext)
{
    TRANSPORT_CMC_SUC_CONTEXT_TYPE* pContext = (TRANSPORT_CMC_SUC_CONTEXT_TYPE*)pUserContext;

    pContext->rxFlow.state = IDLE;
    pContext->rxFlow.idx = 0;
}


uint8_t* TRANSPORT_CMC_SUC_DeframeGetMessage(void* pUserContext)
{
    TRANSPORT_CMC_SUC_CONTEXT_TYPE* pContext = (TRANSPORT_CMC_SUC_CONTEXT_TYPE*)pUserContext;

    return((uint8_t * )pContext->pDeframerContext->Element);
}


void TRANSPORT_CMC_SUC_SendByteSequenceBlocking(void* pUserContext, char* pData, uint32_t SequenceLength)
{
    TRANSPORT_CMC_SUC_CONTEXT_TYPE* pContext = (TRANSPORT_CMC_SUC_CONTEXT_TYPE*)pUserContext;

    PERIPHERAL_AXI_UART_LITE_SATELLITE_SendByteSequenceBlocking(pContext->pUartContext, pData, SequenceLength);
}
