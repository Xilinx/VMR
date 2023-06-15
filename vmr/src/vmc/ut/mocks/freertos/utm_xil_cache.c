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



/*****************************Internal functions*******************************/
/* Used to assert lock value in testcase*/
void Xil_DCacheFlushRange(INTPTR opstartadr, u32 len)
{
	return;
}