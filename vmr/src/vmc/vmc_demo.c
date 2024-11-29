/******************************************************************************
* Copyright (C) 2024 AMD, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#include "cl_uart_rtos.h"
#include "cl_main.h"

/* VMC Header files */
#include "vmc_api.h"
#include "vmc_main.h"

/*
 * Menu thread stack size
 */
#define MENU_THREADSTACKSIZE    2048

extern void AsdmSensor_Display(void);

#define demoMenu_task_PRIORITY	tskIDLE_PRIORITY + 1

/** Maximum menu levels that are supported in xmenu */
#define MAX_MENU_LEVELS     3

#define CHAR_a	(0x61)

/*! \struct TestMenu
 * \brief This structure contains all  test details
 *
 * This structure is used to hold a set of tests in a single instance
 * Same structure is used to hold both the main menu as well  as sub menu details.
 */
typedef struct TestMenu
{
    char *TestName;                     /*!< Test menu name/Test name */
    struct TestMenu *Tests;             /*!< Test menu structure */
    void (*FpTestFunction)(void);       /*!< Function pointer to the test */
}TestMenu;

static void App_SetLogLevel(void);

/**
 *  @brief Table for Log Level Setting
 *
 *  - Available Logging levels
 */
char *LogLevels[] =
{
    " VERBOSE",
    " DEBUG",
    " INFO",
    " WARN",
    " ERROR",
    NULL
};

TestMenu EEPROMTests[] =
{
 {" Test EEPROM read/write", NULL, EepromTest},
 {" Dump EEPROM contents", NULL, EepromDump},
 {NULL, NULL, NULL}
};

TestMenu TestsMenu[] =
{
    {"Set Log Level", NULL, App_SetLogLevel},
    {"EEPROM Test (U005)", EEPROMTests, NULL},
    {"Asdm Sensor Read", NULL, AsdmSensor_Display},
    {"Get Board Info", NULL, BoardInfoTest},
    {"Sensor Data Read", NULL, SensorData_Display},
    {NULL, NULL, NULL}
};

static void App_SetLogLevel(void)
{
    u8 EntryIndex = 0;
    long Choice = 0;
    char UserInput = 0;
    u32 receivedBytes = 0;

    VMC_DMO("Current log level is: %d %s\n\r\n\r", (unsigned int)VMC_GetLogLevel(), LogLevels[VMC_GetLogLevel()]);

    for(EntryIndex = 0; LogLevels[EntryIndex] != NULL; EntryIndex++)
    {
    	VMC_DMO("%d %s\n\r", (unsigned int)EntryIndex, LogLevels[EntryIndex]);
    }
    VMC_DMO(" (q) quit\n\r\n\r");
    while (1)
    {
        if (VMC_User_Input_Read(&UserInput, &receivedBytes) == UART_SUCCESS)
        {
        	if(receivedBytes != 1)
        		continue;
        }
        else
        	continue;

        /* user pressed 'q' to quite the menu */
        if (UserInput == 'q')
        {
            break;
        } /* user pressed ENTER */
        else if (UserInput == ' ')
        {
        	VMC_DMO("\n\r");
        }
        else
        {
            /* Get the test number that is entered in string format */
            Choice = atol(&UserInput);
            VMC_SetLogLevel(Choice);
            break;
        }
    }
}


/*
 *  ======== Menu Task ========
 */
void cl_uart_demo_func(void *task_args)
{
    u8 TestIndex = 0;
    u8 MenuLevel = 0;
    long Choice = 0;
    char UserInput = 0;
    TestMenu *pTestMenuLevel[MAX_MENU_LEVELS];
    TestMenu *pTestMenu;
    u32 receivedBytes = 0;

    /*
     * Change the log level from none to inform to enable the debug port.
     */
    VMC_SetLogLevel(VMC_LOG_LEVEL_INFO);

	pTestMenu = TestsMenu;

    pTestMenuLevel[MenuLevel] = pTestMenu;
    while(1)
    {
        VMC_DMO("\n\r------------------------------------------------------------------------------------\n\r");
        if(MenuLevel == 0)
        {
        	VMC_DMO("                          PERIPHERAL TESTS MAIN MENU                            \n\r");
        }
        else
        {
        	VMC_DMO("                          PERIPHERAL TESTS SUB MENU                            \n\r");
        }
        VMC_DMO("------------------------------------------------------------------------------------\n\r");


        for(TestIndex = 0; pTestMenu[TestIndex].TestName != NULL; TestIndex++)
        {
            if(TestIndex < 9) /* to print menu items from 1 to 9 */
            	VMC_DMO("%d %s\n\r", (unsigned int)TestIndex + 1, pTestMenu[TestIndex].TestName);
            else
            {
            	/* Continue printing menu items using lower case letters starting with 'a'. */
                VMC_DMO("%c %s\n\r", (unsigned int)((TestIndex - 9)+ CHAR_a), pTestMenu[TestIndex].TestName);
            }
        }
        VMC_DMO("------------------------------------------------------------------------------------\n\r");
        if(MenuLevel != 0)
        {
        	VMC_DMO(" (q) quit\n\r\n\r");
        }
        VMC_DMO("Enter Option: ");
        /* Get the command line input with Upper case letters */
        if (VMC_User_Input_Read(&UserInput, &receivedBytes) == UART_SUCCESS)
        {
        	if(receivedBytes != 1)
        		continue;
        }
        else
        	continue;

        VMC_DMO("%c\n\r", UserInput);

        /* user pressed 'q' to quite the menu */
        if (UserInput == 'q')
        {
            /* If user pressed 'Q' at the sub menu level, move to upper menu. */
            if(MenuLevel != 0)
            {
                pTestMenu = pTestMenuLevel[--MenuLevel];
            }
            else
            {
            	VMC_ERR("\n\rInvalid Option, Re-enter\n\r");
            }
        } /* user pressed ENTER */
        else if (UserInput == ' ')
        {
        	VMC_DMO("\n\r");
        }
        else
        {
            /* Get the test number that is entered in string format */
            if(UserInput >= 'a' &&  UserInput <= 'f' )
            {

                Choice = (UserInput + 10) - 0x61;
                if(pTestMenu[Choice-1].FpTestFunction != NULL)
                {
                	VMC_DMO("\n\r");
                    pTestMenu[Choice - 1].FpTestFunction();
                }
                if(pTestMenu[Choice - 1].Tests != NULL)
                {
                    /* Assign sub menu tests pointer */
                    pTestMenu = pTestMenu[Choice - 1].Tests;
                    /* Move to sub menu */
                    pTestMenuLevel[++MenuLevel] = pTestMenu;
                }
            }
            else if(UserInput >= '1' &&  UserInput <= '9'){
                Choice = atol(&UserInput);

                VMC_DMO("%d\n\r",Choice);

                if(pTestMenu[Choice-1].FpTestFunction != NULL)
                {
                	VMC_DMO("\n\r");
                    pTestMenu[Choice - 1].FpTestFunction();
                }
                if(pTestMenu[Choice - 1].Tests != NULL)
                {
                    /* Assign sub menu tests pointer */
                    pTestMenu = pTestMenu[Choice - 1].Tests;
                    /* Move to sub menu */
                    pTestMenuLevel[++MenuLevel] = pTestMenu;
                }

            }

            else
            {
                /* entered option is not valid */
            	VMC_ERR("\n\rInvalid Option, Re-Enter\n\r");
            }
        }
    }
    return;
}

int cl_uart_demo_init(void)
{
	return 0;
}


