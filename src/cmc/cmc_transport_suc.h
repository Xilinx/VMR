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


#ifndef _CMC_TRANSPORT_SUC_H_
#define _CMC_TRANSPORT_SUC_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_entry_point_build_profile_transport_sensor.h"
#include "cmc_peripheral_axi_uart_lite_satellite.h"
#include "cmc_supervisor_message_deframer.h"
#include "cmc_user_supplied_environment.h"


    /* Layout of a Transport Header */
    #define SUC_TRANSPORT_HDR_START_OFF       (0)
    #define SUC_TRANSPORT_HDR_LENGTH_OFF      (1)  /* Little Endian for Header*/
    #define SUC_TRANSPORT_HDR_FLAGS_OFF       (3)
    #define SUC_TRANSPORT_HDR_CRC_OFF         (4)  /* Big Endian for CRC*/
    #define SUC_TRANSPORT_HDR_CRC_LEN         (2)
    #define SUC_TRANSPORT_HEADER_LEN          (6)

    /* Start of Packet Indicator */
    #define SUC_TRANSPORT_START_BYTE          (0x7e)
    #define SUC_TRANSPORT_VERSION             (1)
    #define SUC_TRANSPORT_RESERVED            (0)
    #define SUC_TRANSPORT_PAYLOAD_CRC_LEN     (2)

    /* Packet size based on current CMC */
    #define SUC_TRANSPORT_MAX_PACKET_LEN      (255)

    /* Stream ID for CMC */
    #define SUC_TRANSPORT_STREAM_ID           (2)

    /* Overall overhead associated with the transport mechanism */
    #define SUC_TRANSPORT_FORMAT_LEN          (SUC_TRANSPORT_HEADER_LEN + SUC_TRANSPORT_PAYLOAD_CRC_LEN)



    /* Receiver and transmitter state */
    typedef enum
    {
        IDLE,
        RX_HEADER,   /* Reading header */
        RX_PAYLOAD,  /* Reading payload */
        RX_CRC,      /* Reading CRC */
        RX_DROP,     /* Dropping packet (no, or invalid receiver) */
        TX_HEADER,   /* Writing header */
        TX_PAYLOAD,  /* Writing payload */
        TX_CRC       /* Writing CRC */
    } SUC_TRANSPORT_STATE_TYPE;


    /* Flags */
    typedef struct SUC_TRANSPORT_FLAGS_TYPE
    {
        uint8_t streamId : 2;
        uint8_t reserved : 4;
        uint8_t version : 2;
    } SUC_TRANSPORT_FLAGS_TYPE;


    /* Packet */
    typedef struct SUC_TRANSPORT_PACKET_TYPE
    {
        /* The amount of data */
        unsigned int dataLen;
        /*The data buffer */
        uint8_t data[SUC_TRANSPORT_MAX_PACKET_LEN];
    } SUC_TRANSPORT_PACKET_TYPE;


    typedef struct SUC_TRANSPORT_FLOW_TYPE
    {
        unsigned int idx;
        SUC_TRANSPORT_STATE_TYPE state;
        uint8_t header[SUC_TRANSPORT_HEADER_LEN];
        SUC_TRANSPORT_PACKET_TYPE packet;
    } SUC_TRANSPORT_FLOW_TYPE;



    typedef struct TRANSPORT_CMC_SUC_CONTEXT_TYPE
    {
        /* conifgured within the profile */
        SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pDeframerContext;
        PERIPHERAL_AXI_UART_LITE_SATELLITE_CONTEXT_TYPE* pUartContext;
        USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT;

        /* internal */
        SUC_TRANSPORT_FLOW_TYPE rxFlow;
        SUC_TRANSPORT_FLOW_TYPE txFlow;

    } TRANSPORT_CMC_SUC_CONTEXT_TYPE;


    void TRANSPORT_CMC_SUC_Initialize(void* pUserContext);
    void TRANSPORT_CMC_SUC_FrameMessage(void* pUserContext, uint8_t* pMessage, uint8_t* pMessageLength, uint8_t* pFramedMessage, uint8_t* pFramedMessageLength);
    bool TRANSPORT_CMC_SUC_DeframeTryAdd(void* pUserContext, char AnyCharacter);
    void TRANSPORT_CMC_SUC_DeframeReset(void* pUserContext);
    uint8_t* TRANSPORT_CMC_SUC_DeframeGetMessage(void* pUserContext);
    void TRANSPORT_CMC_SUC_SendByteSequenceBlocking(void* pUserContext, char* pData, uint32_t SequenceLength);


#ifdef __cplusplus
}
#endif


#endif /* _CMC_TRANSPORT_SUC_H_ */


