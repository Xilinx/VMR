#include "utt_assert_SDRs.h"

/*Macro defined in vmc_sensor.c*/
#define SENSOR_RESP_BUFFER_SIZE 512

/* Number of records for v70 */
#define V70_NUM_BOARD_INFO_SENSORS      (11)
#define V70_NUM_TEMPERATURE_SENSORS     (3)
#define V70_NUM_SC_VOLTAGE_SENSORS      (2)
#define V70_NUM_SYSMON_VOLTAGE_SENSORS  (1)
#define V70_NUM_SC_CURRENT_SENSORS      (3)
#define V70_NUM_POWER_SENSORS           (1)

#define REPOSITORY_VERSION (1)

/*To Print the SDR values while executing test, uncomment below macro*/
//#define PRINT_SDR

extern Assert_Sdr_t assertSdrData[];

extern int get_sem_lock();
extern void ConfigureV70Platform();
void assert_Sdr(u8 *respBuffer, u8 sdrOffset);

extern void V70_Monitor_Sensors();

/*Setup function called before executing each testcase.
 * This is feature in cmocka*/
static int setup(void **state)
{
	V70_Init();
	Init_Asdm();
	return 0;
}

static void test_Asdm_Get_BoardInfoSDR(void **state) {
	(void) state; /* unused */

	s8 ret = -1;
	u8 reqBuffer[2] = {0};
	u8 respBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 respSize = 0;
	u16 sdrSize = 0;

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	reqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC0 - Board Info SDR */
	reqBuffer[1] = BoardInfoSDR;

	/*Test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	sdrSize = ((respBuffer[3] << 8) | respBuffer[2]);

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true(ret == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == BoardInfoSDR);  /*Repository Type*

	/*set return value for mock function*/
	will_return_always(__wrap_xQueueSemaphoreTake, 1);
	will_return_always(__wrap_xQueueGenericSend, 1);

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	reqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC0 - Board Info SDR */
	reqBuffer[1] = BoardInfoSDR;

	/*test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	/*Assert Asdm_Get_Sensor_Repository API response*/
	assert_true(ret == 0);
	assert_true(((respSize -1) >= (sdrSize - 8)) && ((respSize - 1) <= sdrSize));
	assert_true(get_sem_lock() == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == BoardInfoSDR);                 /*Repository Type*/
	assert_true(respBuffer[2] == REPOSITORY_VERSION);           /*Repository Version*/
	assert_true(respBuffer[3] == V70_NUM_BOARD_INFO_SENSORS);   /*No. of Records*/
	assert_Sdr(&respBuffer[0], eBoardInfoSDR_offset);
}


static void test_Asdm_Get_TemperatureSDR(void **state) {
	(void) state; /* unused */

	s8 ret = -1;
	u8 reqBuffer[2] = {0};
	u8 respBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 respSize = 0;
	u16 sdrSize = 0;

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	reqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC1 - Temperature SDR */
	reqBuffer[1] = TemperatureSDR;

	/*Test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	sdrSize = ((respBuffer[3] << 8) | respBuffer[2]);

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true(ret == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == TemperatureSDR);   /*Repository Type*/

	/*set return value for mock function*/
	will_return_always(__wrap_xQueueSemaphoreTake, 1);
	will_return_always(__wrap_xQueueGenericSend, 1);

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	reqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC1 - Temperature SDR */
	reqBuffer[1] = TemperatureSDR;

	ConfigureV70Platform();
	V70_Monitor_Sensors();
	Asdm_Update_Sensors();

	/*Test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	/*Assert Asdm_Get_Sensor_Repository API Response*/
	assert_true(ret == 0);
	assert_true(((respSize -1) >= (sdrSize - 8)) && ((respSize - 1) <= sdrSize));
	assert_true(get_sem_lock() == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == TemperatureSDR);               /*Repository Type*/
	assert_true(respBuffer[2] == REPOSITORY_VERSION);           /*Repository Version*/
	assert_true(respBuffer[3] == V70_NUM_TEMPERATURE_SENSORS);  /*No. of Records*/
	assert_Sdr(&respBuffer[0], eTemperatureSDR_offset);
}

static void test_Asdm_Get_VoltageSDR(void **state) {
	(void) state; /* unused */

	s8 ret = -1;
	u8 reqBuffer[2] = {0};
	u8 respBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 respSize = 0;
	u16 sdrSize = 0;

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	reqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC2 - Voltage SDR */
	reqBuffer[1] = VoltageSDR;

	/*Test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	sdrSize = ((respBuffer[3] << 8) | respBuffer[2]);

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true(ret == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == VoltageSDR);   /*Repository Type*/

	/*set return value for mock function*/
	will_return_always(__wrap_xQueueSemaphoreTake, 1);
	will_return_always(__wrap_xQueueGenericSend, 1);

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	reqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC2 - Voltage SDR */
	reqBuffer[1] = VoltageSDR;

	V70_Monitor_Sensors();
	Asdm_Update_Sensors();

	/*Test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	/*Assert Asdm_Get_Sensor_Repository API response*/
	assert_true(ret == 0);
	assert_true(((respSize -1) >= (sdrSize - 8)) && ((respSize - 1) <= sdrSize));
	assert_true(get_sem_lock() == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == VoltageSDR);                                                          /*Repository Type*/
	assert_true(respBuffer[2] == REPOSITORY_VERSION);                                                  /*Repository Version*/
	assert_true(respBuffer[3] == (V70_NUM_SC_VOLTAGE_SENSORS + V70_NUM_SYSMON_VOLTAGE_SENSORS));       /*No. of Records*/
	assert_Sdr(&respBuffer[0], eVoltageSDR_offset);
}

static void test_Asdm_Get_CurrentSDR(void **state) {
	(void) state; /* unused */

	s8 ret = -1;
	u8 reqBuffer[2] = {0};
	u8 respBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 respSize = 0;
	u16 sdrSize = 0;

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	reqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC3 - current SDR */
	reqBuffer[1] = CurrentSDR;

	/*test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	sdrSize = ((respBuffer[3] << 8) | respBuffer[2]);

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true(ret == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == CurrentSDR);   /*Repository Type*/

	/*set return value for mock function*/
	will_return_always(__wrap_xQueueSemaphoreTake, 1);
	will_return_always(__wrap_xQueueGenericSend, 1);

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	reqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC3 - Current SDR */
	reqBuffer[1] = CurrentSDR;

	V70_Monitor_Sensors();
	Asdm_Update_Sensors();

	/*Test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	/*Assert Asdm_Get_Sensor_Repository API response*/
	assert_true(ret == 0);
	assert_true(((respSize -1) >= (sdrSize - 8)) && ((respSize - 1) <= sdrSize));
	assert_true(get_sem_lock() == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == CurrentSDR);                            /*Repository Type*/
	assert_true(respBuffer[2] == REPOSITORY_VERSION);                    /*Repository Version*/
	assert_true(respBuffer[3] == V70_NUM_SC_CURRENT_SENSORS);            /*No. of Records*/
	assert_Sdr(&respBuffer[0], eCurrentSDR_offset);
}

static void test_Asdm_Get_PowerSDR(void **state) {
	(void) state; /* unused */

	s8 ret = -1;
	u8 reqBuffer[2] = {0};
	u8 respBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 respSize = 0;
	u16 sdrSize = 0;

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	reqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC4 - Power SDR */
	reqBuffer[1] = PowerSDR;

	/*test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	sdrSize = ((respBuffer[3] << 8 )| respBuffer[2]);

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true(ret == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == PowerSDR);     /*Repository Type*/

	/*set return value for mock function*/
	will_return_always(__wrap_xQueueSemaphoreTake, 1);
	will_return_always(__wrap_xQueueGenericSend, 1);

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	reqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC4 - Power SDR */
	reqBuffer[1] = PowerSDR;

	ConfigureV70Platform();
	V70_Monitor_Sensors();
	Asdm_Update_Sensors();

	/*Test function*/
	ret = Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize);

	/*Assert Asdm_Get_Sensor_Repository API response*/
	assert_true(ret == 0);
	assert_true(((respSize -1) >= (sdrSize - 8)) && ((respSize - 1) <= sdrSize));
	assert_true(get_sem_lock() == 0);
	assert_true(respBuffer[0] == Asdm_CC_Operation_Success);
	assert_true(respBuffer[1] == PowerSDR);                 /*Repository Type*/
	assert_true(respBuffer[2] == REPOSITORY_VERSION);       /*Repository Version*/
	assert_true(respBuffer[3] == V70_NUM_POWER_SENSORS);    /*No. of Records*/
	assert_Sdr(&respBuffer[0], ePowerSDR_offset);
}

int main(void) {
	const struct CMUnitTest tests[] = {
			cmocka_unit_test_setup(test_Asdm_Get_BoardInfoSDR, setup),
			cmocka_unit_test_setup(test_Asdm_Get_TemperatureSDR, setup),
			cmocka_unit_test_setup(test_Asdm_Get_VoltageSDR, setup),
			cmocka_unit_test_setup(test_Asdm_Get_CurrentSDR, setup),
			cmocka_unit_test_setup(test_Asdm_Get_PowerSDR, setup)
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

void assert_Sdr(u8 *respBuffer, u8 sdrOffset)
{
	u16 respIndex = 1;
	u8 len = 0;
	u8 snsrlen = 0;
	u8 tmp[50];
	u8 *snsrvalue;
	u8 threshold_support;
	u8 idx = 0;
	Asdm_Header_t sdrHeader;

	sdrHeader.repository_type = respBuffer[respIndex++];
	sdrHeader.repository_version = respBuffer[respIndex++];
	sdrHeader.no_of_records = respBuffer[respIndex++];
	sdrHeader.no_of_bytes = respBuffer[respIndex++];

	for(idx = 1; idx <= sdrHeader.no_of_records; idx++)
	{
#ifdef PRINT_SDR
		printf("***********************SDR-%d************************\n", idx);
		printf("Sensor ID : %d\n", respBuffer[respIndex]);
#endif
		assert_true(respBuffer[respIndex++] == assertSdrData[sdrOffset].sensor_id);

		len = (respBuffer[respIndex++] & 0x3F);
#ifdef PRINT_SDR
		printf("Sensor Name type length : %d\n", len);
#endif
		assert_true(len == assertSdrData[sdrOffset].sensor_name_type_length);

		memcpy(&tmp[0], &respBuffer[respIndex], len);
#ifdef PRINT_SDR
		printf("Sensor Name : %s\n", tmp);
#endif
		assert_string_equal(tmp, assertSdrData[sdrOffset].sensor_name);
		respIndex += len;

		snsrlen = (respBuffer[respIndex++] & 0x3F);
#ifdef PRINT_SDR
		printf("Sensor value type length : %d\n", snsrlen);
#endif
		assert_true(snsrlen == assertSdrData[sdrOffset].sensor_value_type_length);

		snsrvalue = malloc(snsrlen);
		if(snsrlen != 0)
		{
			if(sdrHeader.repository_type == BoardInfoSDR)
			{
				memcpy(&tmp[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
				printf("Sensor value : %s\n", tmp);
#endif
				assert_string_equal(tmp, assertSdrData[sdrOffset].sensor_value);
			}
			else
			{
				memcpy(&snsrvalue[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
				if(snsrlen == 2)
				{
					printf("Sensor value : %d\n", (*snsrvalue | (*(snsrvalue + 1) << 8)));
				}
				else if(snsrlen == 4)
				{
					printf("Sensor value : %d\n", (*snsrvalue | (*(snsrvalue + 1) << 8) | (*(snsrvalue + 2) << 16) | (*(snsrvalue + 3) << 24)));
				}
#endif
				assert_memory_equal(snsrvalue, assertSdrData[sdrOffset].sensor_value, snsrlen);
			}
			respIndex += snsrlen;
		}

		len = (respBuffer[respIndex++] & 0x3F);
		if(len != 0)
		{
#ifdef PRINT_SDR
			printf("Sensor base unit length : %d\n", len);
#endif
			assert_true(len == assertSdrData[sdrOffset].sensor_base_unit_type_length);

			memcpy(&tmp[0], &respBuffer[respIndex], len);
#ifdef PRINT_SDR
			printf("Sensor base unit : %s\n", tmp);
#endif
			assert_string_equal(tmp, assertSdrData[sdrOffset].sensor_base_unit);
			respIndex += len;
		}
#ifdef PRINT_SDR
		printf("Sensor unit modifier : %x \n", respBuffer[respIndex]);
#endif
		assert_memory_equal(&respBuffer[respIndex++], &assertSdrData[sdrOffset].sensor_unit_modifier_byte, 1);

		threshold_support = respBuffer[respIndex++];
#ifdef PRINT_SDR
		printf("Threshold support byte : %x\n", threshold_support);
#endif
		assert_true(threshold_support == assertSdrData[sdrOffset].threshold_support_byte);

		if(threshold_support & Lower_Fatal_Threshold)
		{
			memcpy(&snsrvalue[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
			printf("Lower fatal limit :%d\n", *snsrvalue);
#endif
			assert_memory_equal(snsrvalue, assertSdrData[sdrOffset].lower_fatal_limit, snsrlen);
			respIndex += snsrlen;
		}

		if(threshold_support & Lower_Critical_Threshold)
		{
			memcpy(&snsrvalue[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
			printf("Lower critical limit :%d\n", *snsrvalue);
#endif
			assert_memory_equal(snsrvalue, assertSdrData[sdrOffset].lower_critical_limit, snsrlen);
			respIndex += snsrlen;
		}

		if(threshold_support & Lower_Warning_Threshold)
		{
			memcpy(&snsrvalue[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
			printf("Lower warning limit :%d\n", *snsrvalue);
#endif
			assert_memory_equal(snsrvalue, assertSdrData[sdrOffset].lower_warning_limit, snsrlen);
			respIndex += snsrlen;
		}

		if(threshold_support & Upper_Fatal_Threshold)
		{
			memcpy(&snsrvalue[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
			printf("Upper fatal limit :%d\n", *snsrvalue);
#endif
			assert_memory_equal(snsrvalue, assertSdrData[sdrOffset].upper_fatal_limit, snsrlen);
			respIndex += snsrlen;
		}

		if(threshold_support & Upper_Critical_Threshold)
		{
			memcpy(&snsrvalue[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
			printf("Upper Critical limit :%d\n", *snsrvalue);
#endif
			assert_memory_equal(snsrvalue, assertSdrData[sdrOffset].upper_critical_limit, snsrlen);
			respIndex += snsrlen;
		}

		if(threshold_support & Upper_Warning_Threshold)
		{
			memcpy(&snsrvalue[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
			printf("Upper Warning limit :%d\n", *snsrvalue);
#endif
			assert_memory_equal(snsrvalue, assertSdrData[sdrOffset].upper_warning_limit, snsrlen);
			respIndex += snsrlen;
		}

#ifdef PRINT_SDR
		printf("Sensor status : %x\n", respBuffer[respIndex]);
#endif
		assert_true(respBuffer[respIndex++] == assertSdrData[sdrOffset].sensor_status);

		if(threshold_support & Sensor_Avg_Val_Support)
		{
			memcpy(&snsrvalue[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
			if(snsrlen == 2)
			{
				printf("Sensor Avg value : %d\n", (*snsrvalue | (*(snsrvalue + 1) << 8)));
			}
			else if(snsrlen == 4)
			{
				printf("Sensor Avg value : %d\n", (*snsrvalue | (*(snsrvalue + 1) << 8) | (*(snsrvalue + 2) << 16) | (*(snsrvalue + 3) << 24)));
			}
#endif
			assert_memory_equal(snsrvalue, assertSdrData[sdrOffset].sensorAverageValue, snsrlen);
			respIndex += snsrlen;
		}

		if(threshold_support & Sensor_Max_Val_Support)
		{
			memcpy(&snsrvalue[0], &respBuffer[respIndex], snsrlen);
#ifdef PRINT_SDR
			if(snsrlen == 2)
			{
				printf("Sensor Max value : %d\n", (*snsrvalue | (*(snsrvalue + 1) << 8)));
			}
			else if(snsrlen == 4)
			{
				printf("Sensor Max value : %d\n", (*snsrvalue | (*(snsrvalue + 1) << 8) | (*(snsrvalue + 2) << 16) | (*(snsrvalue + 3) << 24)));
			}
#endif
			assert_memory_equal(snsrvalue, assertSdrData[sdrOffset].sensorMaxValue, snsrlen);
			respIndex += snsrlen;
		}
		sdrOffset++;
		free(snsrvalue);
	}
}
