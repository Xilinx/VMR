#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "xsysmonpsv.h"
#include "xsysmonpsv_lowlevel.h"
#include "xil_assert.h"
#include "xstatus.h"

int __wrap_XSysMonPsv_ReadTempProcessed(XSysMonPsv *InstancePtr,
                 XSysMonPsv_TempType Type, float *Val)
{
    if (InstancePtr == NULL || Val == NULL) {
        return -XST_FAILURE;
    }
    
    *Val = (float)38.5;
    return XST_SUCCESS;
}

int __wrap_XSysMonPsv_ReadSupplyProcessed(XSysMonPsv *InstancePtr, int Supply, float *Val)
{
    if (InstancePtr == NULL) {
        return -XST_FAILURE;
    }

    *Val = (float)5.5;
    return XST_SUCCESS;
}

int __wrap_XSysMonPsv_Init(XSysMonPsv *InstancePtr, XScuGic *IntcInst)
{
    if (InstancePtr == NULL || IntcInst == NULL) {
        return -XST_FAILURE;
    }

    return XST_SUCCESS;
}
