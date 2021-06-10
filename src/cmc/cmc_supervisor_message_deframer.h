/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* � Copyright 2019 Xilinx, Inc. All rights reserved.                                           */
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
 *  $Change: 2928154 $
 *  $Date: 2020/06/30 $
 *  $Revision: #10 $
 *
 */




#ifndef _CMC_SUPERVISOR_DEFRAMER_H_
#define _CMC_SUPERVISOR_DEFRAMER_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"

	typedef enum
	{
		DEFRAMER_START_DEFRAMER_ESCAPE,
		DEFRAMER_START_OF_FRAME,
		DEFRAMER_ESCAPE,
		DEFRAMER_END_OF_FRAME

	}SENSOR_SUPERVISOR_DEFRAMER_STATE;


	typedef struct SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE
	{
		bool			HaveFrame;
		SENSOR_SUPERVISOR_DEFRAMER_STATE	State;
		uint32_t		iWrite;
		char			Element[FRAMER_MAX_CHARACTERS];

	} SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE;

	void cmc_sensor_supervisor_DeFramer_Initialize(SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pContext);
	bool cmc_sensor_supervisor_DeFramer_TryAdd(SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pContext, char AnyCharacter);
	bool cmc_sensor_supervisor_DeFramer_GetPayload(SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pContext, char** ppPayload, uint32_t* pPayloadSize);
	void cmc_sensor_supervisor_DeFramer_Reset(SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pContext);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_SUPERVISOR_DEFRAMER_H_ */











