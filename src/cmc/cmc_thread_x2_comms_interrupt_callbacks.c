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
 *  $Change: 2762437 $
 *  $Date: 2020/01/27 $
 *  $Revision: #6 $
 *
 */


#include "cmc_thread_x2_comms.h"


#define REQUEST_RESET_MAGIC_NUMBER      (0x52)  /* ASCII 'R' */



//NOTE - This will be called in INTERRUPT context
void CMC_X2_WriteRequestCallback(void* callbackContext, uint8_t* pData, uint32_t numBytes)
{
    CMC_X2_COMMS_CONTEXT* pContext = (CMC_X2_COMMS_CONTEXT*)callbackContext;
    CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pVirtualRegisters = &(pContext->virtualRegisterSet);
    uint32_t selectedRegisterAddress;
   
    
    uint8_t* pPayload;
    uint32_t payloadLength;
    uint32_t numBytesWritten = 0;

    if (numBytes > 0)
    {
        selectedRegisterAddress = pData[0];

        CMC_X2_VirtualRegisterSet_SetSelectedRegisterAddress(pVirtualRegisters, selectedRegisterAddress);

        if (numBytes > 1)
        {
            pPayload = &pData[1];
            payloadLength = numBytes - 1;

            CMC_X2_VirtualRegisterSet_Write(pVirtualRegisters, selectedRegisterAddress, pPayload, payloadLength, &numBytesWritten);



            //raise an FSM event (if the WRITE was to one of the register we were interested in)
            //NOTE - we're only raising this event if DATA was written to the register 
            //       i.e. we are not raising the event if the I2C write was only to select the address of that register 
            //       (e.g. in preparation for a read of that register)
            CMC_X2_RaiseWriteEvent(pContext, selectedRegisterAddress);


        }          
    }
}





//NOTE - This will be called in INTERRUPT context
void CMC_X2_ReadRequestCallback(void* callbackContext, uint8_t* pBuffer, uint32_t bufferSize, uint32_t* pNumValidBytes)
{
    CMC_X2_COMMS_CONTEXT* pContext = (CMC_X2_COMMS_CONTEXT*)callbackContext;
    CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pVirtualRegisters = &(pContext->virtualRegisterSet);
    uint32_t selectedRegisterAddress;
    uint32_t numBytesRead = 0;


    CMC_X2_VirtualRegisterSet_GetSelectedRegisterAddress(pVirtualRegisters, &selectedRegisterAddress);


    CMC_X2_VirtualRegisterSet_Read(pVirtualRegisters, selectedRegisterAddress, pBuffer, bufferSize, &numBytesRead);


    if (numBytesRead != 0)
    {
        *pNumValidBytes = numBytesRead;

        // raise an FSM event (if the READ was from one of the registers we were interested in)
        CMC_X2_RaiseReadEvent(pContext, selectedRegisterAddress);
    }
    else
    {
        //give back a single byte of 0xFF
        pBuffer[0] = 0xFF;
        *pNumValidBytes = 1;
    }
}



        




void CMC_X2_RaiseReadEvent(CMC_X2_COMMS_CONTEXT* pContext, uint8_t virtualRegisterAddress)
{
    bool bOKToRaiseEvent = false;
    CMC_X2_FSM_EVENT event = CMC_X2_MAX_EVENTS;


    switch (virtualRegisterAddress)
    {
        case(CMC_X2_VIRTUAL_REG_STATUS):
        {     
            event = EVENT_X2_READ_STATUS;
            bOKToRaiseEvent = true;
            break;
        }


        case(CMC_X2_VIRTUAL_REG_REQUEST_LENGTH):
        {
            event = EVENT_X2_READ_REQUEST_LENGTH;
            bOKToRaiseEvent = true;
            break;
        }


        case(CMC_X2_VIRTUAL_REG_REQUEST_DATA):
        {
            event = EVENT_X2_READ_REQUEST_DATA;
            bOKToRaiseEvent = true;
            break;
        }


        default:
        {
            bOKToRaiseEvent = false;
            break;
        }
    }


    if (bOKToRaiseEvent)
    {
        CMC_X2_FSM_AddEvent(pContext, event);
    }
}












void CMC_X2_RaiseWriteEvent(CMC_X2_COMMS_CONTEXT* pContext, uint8_t virtualRegisterAddress)
{
    
    bool bOKToRaiseEvent = false;
    CMC_X2_FSM_EVENT event = CMC_X2_MAX_EVENTS;


    switch (virtualRegisterAddress)
    {
        case(CMC_X2_VIRTUAL_REG_REQUEST_RESET):
        {     
            if (pContext->virtualRegisterSet.Request_Reset == REQUEST_RESET_MAGIC_NUMBER)
            {
                pContext->virtualRegisterSet.Request_Reset = 0x00; //clear the magic number...
                event = EVENT_RESET_REQUEST;
                bOKToRaiseEvent = true;
            }
          
            break;
        }


        case(CMC_X2_VIRTUAL_REG_RESPONSE_LENGTH):
        {
            event = EVENT_X2_WROTE_RESPONSE_LENGTH;
            bOKToRaiseEvent = true;
            break;
        }


        case(CMC_X2_VIRTUAL_REG_RESPONSE_DATA):
        {
            event = EVENT_X2_WROTE_RESPONSE_DATA;
            bOKToRaiseEvent = true;
            break;
        }

        
        default:
        {
            bOKToRaiseEvent = false;
            break;
        }
    }


    if (bOKToRaiseEvent)
    {
        CMC_X2_FSM_AddEvent(pContext, event);
    }
}



