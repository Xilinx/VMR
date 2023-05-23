#include "utt_assert_SDRs.h"

/*Macro defined in vmc_sensor.c*/
#define SENSOR_RESP_BUFFER_SIZE 512

/* Number of records for v80 */
#define V80_NUM_BOARD_INFO_SENSORS      ( 11 )
#define V80_NUM_TEMPERATURE_SENSORS     ( 9 )
#define V80_NUM_SC_VOLTAGE_SENSORS      ( 10 )
#define V80_NUM_SC_CURRENT_SENSORS      ( 10 )
#define V80_NUM_POWER_SENSORS           ( 1 )

#define REPOSITORY_VERSION              ( 1 )

/*To Print the SDR values while executing test, uncomment below macro*/
//#define PRINT_SDR

extern Assert_Sdr_t pxAssertSdrDataV80[];

extern int get_sem_lock( );
extern void vConfigureV80Platform( );
void assert_Sdr( u8 *pucRespBuffer, u8 ucSdrOffset );

extern void V80_Monitor_Sensors( );

/*Setup function called before executing each testcase.
 * This is feature in cmocka*/
static int ulTestcaseSetup( void **state )
{
    printf( "setup start\n\r"  );
	ucV80Init( );
    printf( "setup ucV80Init done\n\r"  );
	Init_Asdm( );

    printf( "setup Init_Asdm done:\n\r"  );
	return 0;
}

static void vTestAsdmGetBoardInfoSDR( void **state ) 
{
	( void ) state; /* unused */

	s8 scRet = -1;
	u8 ucReqBuffer[2] = {0};
	u8 pucRespBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 usRespSize = 0;
	u16 usSdrSize = 0;
    
    vConfigureV80Platform( );

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	ucReqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC0 - Board Info SDR */
	ucReqBuffer[1] = BoardInfoSDR;

	/*Test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	usSdrSize = ( ( pucRespBuffer[3] << 8 ) | pucRespBuffer[2] );

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true( scRet == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == BoardInfoSDR );  /*Repository Type*

	/*set return value for mock function*/
	will_return_always( __wrap_xQueueSemaphoreTake, 1 );
	will_return_always( __wrap_xQueueGenericSend, 1 );

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	ucReqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC0 - Board Info SDR */
	ucReqBuffer[1] = BoardInfoSDR;

	/*test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	/*Assert Asdm_Get_Sensor_Repository API response*/
	assert_true( scRet == 0 );
	assert_true( ( ( usRespSize -1 ) >= ( usSdrSize - 8 ) ) && ( ( usRespSize - 1 ) <= usSdrSize ) );
	assert_true( get_sem_lock( ) == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == BoardInfoSDR );                 /*Repository Type*/
	assert_true( pucRespBuffer[2] == REPOSITORY_VERSION );           /*Repository Version*/
	assert_true( pucRespBuffer[3] == V80_NUM_BOARD_INFO_SENSORS );   /*No. of Records*/
	assert_Sdr( &pucRespBuffer[0], eBoardInfoSDR_V80_offset );
}


static void vTestAsdmGetTemperatureSDR( void **state ) {
	( void ) state; /* unused */

	s8 scRet = -1;
	u8 ucReqBuffer[2] = {0};
	u8 pucRespBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 usRespSize = 0;
	u16 usSdrSize = 0;

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	ucReqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC1 - Temperature SDR */
	ucReqBuffer[1] = TemperatureSDR;

	/*Test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	usSdrSize = ( ( pucRespBuffer[3] << 8 ) | pucRespBuffer[2] );

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true( scRet == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == TemperatureSDR );   /*Repository Type*/

	/*set return value for mock function*/
	will_return_always( __wrap_xQueueSemaphoreTake, 1 );
	will_return_always( __wrap_xQueueGenericSend, 1 );

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	ucReqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC1 - Temperature SDR */
	ucReqBuffer[1] = TemperatureSDR;

	vV80MonitorSensors( );
	Asdm_Update_Sensors( );

	/*Test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	/*Assert Asdm_Get_Sensor_Repository API Response*/
	assert_true( scRet == 0 );
	assert_true( ( (  usRespSize -1 ) >= ( usSdrSize - 8 ) ) && ( ( usRespSize - 1 ) <= usSdrSize ) );
	assert_true( get_sem_lock( ) == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == TemperatureSDR );               /*Repository Type*/
	assert_true( pucRespBuffer[2] == REPOSITORY_VERSION );           /*Repository Version*/
	assert_true( pucRespBuffer[3] == V80_NUM_TEMPERATURE_SENSORS );  /*No. of Records*/
	assert_Sdr( &pucRespBuffer[0], eTemperatureSDR_V80_offset );
}

static void test_Asdm_Get_VoltageSDR( void **state ) {
	( void ) state; /* unused */

	s8 scRet = -1;
	u8 ucReqBuffer[2] = {0};
	u8 pucRespBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 usRespSize = 0;
	u16 usSdrSize = 0;

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	ucReqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC2 - Voltage SDR */
	ucReqBuffer[1] = VoltageSDR;

	/*Test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	usSdrSize = ( ( pucRespBuffer[3] << 8 ) | pucRespBuffer[2] );

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true( scRet == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == VoltageSDR );   /*Repository Type*/

	/*set return value for mock function*/
	will_return_always( __wrap_xQueueSemaphoreTake, 1 );
	will_return_always( __wrap_xQueueGenericSend, 1 );

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	ucReqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC2 - Voltage SDR */
	ucReqBuffer[1] = VoltageSDR;

	vV80MonitorSensors( );
	Asdm_Update_Sensors( );

	/*Test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	/*Assert Asdm_Get_Sensor_Repository API response*/
	assert_true( scRet == 0 );
	assert_true( ( (  usRespSize -1 ) >= ( usSdrSize - 8 ) ) && ( ( usRespSize - 1 ) <= usSdrSize ) );
	assert_true( get_sem_lock( ) == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == VoltageSDR );                                                          /*Repository Type*/
	assert_true( pucRespBuffer[2] == REPOSITORY_VERSION );                                                  /*Repository Version*/
	assert_true( pucRespBuffer[3] == ( V80_NUM_SC_VOLTAGE_SENSORS ) );       /*No. of Records*/
	assert_Sdr( &pucRespBuffer[0], eVoltageSDR_V80_offset );
}

static void test_Asdm_Get_CurrentSDR ( void **state ) {
	( void ) state; /* unused */

	s8 scRet = -1;
	u8 ucReqBuffer[2] = {0};
	u8 pucRespBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 usRespSize = 0;
	u16 usSdrSize = 0;

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	ucReqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC3 - current SDR */
	ucReqBuffer[1] = CurrentSDR;

	/*test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	usSdrSize = ( ( pucRespBuffer[3] << 8 ) | pucRespBuffer[2] );

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true( scRet == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == CurrentSDR );   /*Repository Type*/

	/*set return value for mock function*/
	will_return_always( __wrap_xQueueSemaphoreTake, 1 );
	will_return_always( __wrap_xQueueGenericSend, 1 );

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	ucReqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC3 - Current SDR */
	ucReqBuffer[1] = CurrentSDR;

	vV80MonitorSensors( );
	Asdm_Update_Sensors( );

	/*Test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	/*Assert Asdm_Get_Sensor_Repository API response*/
	assert_true( scRet == 0 );
	assert_true( ( (  usRespSize -1 ) >= ( usSdrSize - 8 ) ) && ( ( usRespSize - 1 ) <= usSdrSize ) );
	assert_true( get_sem_lock( ) == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == CurrentSDR );                            /*Repository Type*/
	assert_true( pucRespBuffer[2] == REPOSITORY_VERSION );                    /*Repository Version*/
	assert_true( pucRespBuffer[3] == V80_NUM_SC_CURRENT_SENSORS );            /*No. of Records*/
	assert_Sdr( &pucRespBuffer[0], eCurrentSDR_V80_offset );
}

static void test_Asdm_Get_PowerSDR( void **state ) {
	( void ) state; /* unused */

	s8 scRet = -1;
	u8 ucReqBuffer[2] = {0};
	u8 pucRespBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
	u16 usRespSize = 0;
	u16 usSdrSize = 0;

	/* API ID : 0x01 - Asdm_Get_SDR_Size */
	ucReqBuffer[0] = ASDM_CMD_GET_SIZE;
	/* Record Type : 0xC4 - Power SDR */
	ucReqBuffer[1] = PowerSDR;

	/*test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	usSdrSize = ( ( pucRespBuffer[3] << 8 ) | pucRespBuffer[2] );

	/*Assert Asdm_Get_SDR_Size API response*/
	assert_true( scRet == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == PowerSDR );     /*Repository Type*/

	/*set return value for mock function*/
	will_return_always( __wrap_xQueueSemaphoreTake, 1 );
	will_return_always( __wrap_xQueueGenericSend, 1 );

	/* API ID : 0x02 - Asdm_Get_Sensor_Repository */
	ucReqBuffer[0] = ASDM_CMD_GET_SDR;
	/* Record Type : 0xC4 - Power SDR */
	ucReqBuffer[1] = PowerSDR;

	vV80MonitorSensors( );
	Asdm_Update_Sensors( );

	/*Test function*/
	scRet = Asdm_Process_Sensor_Request( &ucReqBuffer[0], &pucRespBuffer[0], &usRespSize );

	/*Assert Asdm_Get_Sensor_Repository API response*/
	assert_true( scRet == 0 );
	assert_true( ( (  usRespSize -1 ) >= ( usSdrSize - 8 ) ) && ( ( usRespSize - 1 ) <= usSdrSize ) );
	assert_true( get_sem_lock( ) == 0 );
	assert_true( pucRespBuffer[0] == Asdm_CC_Operation_Success );
	assert_true( pucRespBuffer[1] == PowerSDR );                 /*Repository Type*/
	assert_true( pucRespBuffer[2] == REPOSITORY_VERSION );       /*Repository Version*/
	assert_true( pucRespBuffer[3] == V80_NUM_POWER_SENSORS );    /*No. of Records*/
	assert_Sdr( &pucRespBuffer[0], ePowerSDR_V80_offset );
}

int main( void ) 
{
	const struct CMUnitTest tests[] = 
    {
			cmocka_unit_test_setup( vTestAsdmGetBoardInfoSDR, ulTestcaseSetup ),
			cmocka_unit_test_setup( vTestAsdmGetTemperatureSDR, ulTestcaseSetup ),
			cmocka_unit_test_setup( test_Asdm_Get_VoltageSDR, ulTestcaseSetup ),
			cmocka_unit_test_setup( test_Asdm_Get_CurrentSDR, ulTestcaseSetup ),
			cmocka_unit_test_setup( test_Asdm_Get_PowerSDR, ulTestcaseSetup )
	};
	return cmocka_run_group_tests( tests, NULL, NULL );
}

void assert_Sdr( u8 *pucRespBuffer, u8 ucSdrOffset )
{
	u16 usRespIndex = 1;
	u8 ucLen = 0;
	u8 ucSnsrlen = 0;
	u8 ucTmp[50];
	u8 *pucSnsrvalue;
	u8 ucThresholdSupport;
	u8 ucIdx = 0;
	Asdm_Header_t xSdrHeader;

	xSdrHeader.repository_type = pucRespBuffer[usRespIndex++];
	xSdrHeader.repository_version = pucRespBuffer[usRespIndex++];
	xSdrHeader.no_of_records = pucRespBuffer[usRespIndex++];
	xSdrHeader.no_of_bytes = pucRespBuffer[usRespIndex++];

	for( ucIdx = 1; ucIdx <= xSdrHeader.no_of_records; ucIdx++ )
	{
#ifdef PRINT_SDR
		printf( "***********************SDR-%d************************\n", ucIdx );
		printf( "Sensor ID : %d\n", pucRespBuffer[usRespIndex] );
        printf( "ssertSdrDataV80[ucSdrOffset].sensor_id : %d\n", pxAssertSdrDataV80[ucSdrOffset].sensor_id );
#endif
		assert_true( pucRespBuffer[usRespIndex++] == pxAssertSdrDataV80[ucSdrOffset].sensor_id );

		ucLen = ( pucRespBuffer[usRespIndex++] & 0x3F );
#ifdef PRINT_SDR
		printf( "Sensor Name type length : %d\n", ucLen );
#endif
		assert_true( ucLen == pxAssertSdrDataV80[ucSdrOffset].sensor_name_type_length );

		memcpy( &ucTmp[0], &pucRespBuffer[usRespIndex], ucLen );
#ifdef PRINT_SDR
		printf( "Sensor Name : %s\n", ucTmp );
#endif
		assert_string_equal( ucTmp, pxAssertSdrDataV80[ucSdrOffset].sensor_name );
		usRespIndex += ucLen;

		ucSnsrlen = ( pucRespBuffer[usRespIndex++] & 0x3F );
#ifdef PRINT_SDR
		printf( "Sensor value type length : %d\n", ucSnsrlen );
#endif
		assert_true( ucSnsrlen == pxAssertSdrDataV80[ucSdrOffset].sensor_value_type_length );

		pucSnsrvalue = malloc( ucSnsrlen );
		if( ucSnsrlen != 0 )
		{
			if( xSdrHeader.repository_type == BoardInfoSDR )
			{
				memcpy( &ucTmp[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
				printf( "Sensor value : %s\n", ucTmp );
#endif
				assert_string_equal( ucTmp, pxAssertSdrDataV80[ucSdrOffset].sensor_value );
			}
			else
			{
				memcpy( &pucSnsrvalue[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
				if( ucSnsrlen == 2 )
				{
					printf( "Sensor value : %d\n", ( *pucSnsrvalue | ( *( pucSnsrvalue + 1 ) << 8 ) ) );
				}
				else if( ucSnsrlen == 4 )
				{
                    printf( "ucSnsrlen : %d\n\r", ucSnsrlen );
                    printf( "data : %x %x %x %x\n\r", pucRespBuffer[usRespIndex], pucRespBuffer[usRespIndex+1], 
                                                pucRespBuffer[usRespIndex+2], pucRespBuffer[usRespIndex+3] );
					printf( "Sensor value : %d\n", ( *pucSnsrvalue | ( *( pucSnsrvalue + 1 ) << 8 ) | ( *( pucSnsrvalue + 2 ) << 16 ) | ( *( pucSnsrvalue + 3 ) << 24 ) ) );
				}
#endif
				assert_memory_equal( pucSnsrvalue, pxAssertSdrDataV80[ucSdrOffset].sensor_value, ucSnsrlen );
			}
			usRespIndex += ucSnsrlen;
		}

		ucLen = ( pucRespBuffer[usRespIndex++] & 0x3F );
		if( ucLen != 0 )
		{
#ifdef PRINT_SDR
			printf( "Sensor base unit length : %d\n", ucLen );
#endif
			assert_true( ucLen == pxAssertSdrDataV80[ucSdrOffset].sensor_base_unit_type_length );

			memcpy( &ucTmp[0], &pucRespBuffer[usRespIndex], ucLen );
#ifdef PRINT_SDR
			printf( "Sensor base unit : %s\n", ucTmp );
#endif
			assert_string_equal( ucTmp, pxAssertSdrDataV80[ucSdrOffset].sensor_base_unit );
			usRespIndex += ucLen;
		}
#ifdef PRINT_SDR
		printf( "Sensor unit modifier : %x \n", pucRespBuffer[usRespIndex] );
#endif
		assert_memory_equal( &pucRespBuffer[usRespIndex++], &pxAssertSdrDataV80[ucSdrOffset].sensor_unit_modifier_byte, 1 );

		ucThresholdSupport = pucRespBuffer[usRespIndex++];
#ifdef PRINT_SDR
		printf( "Threshold support byte : %x\n", ucThresholdSupport );
#endif
		assert_true( ucThresholdSupport == pxAssertSdrDataV80[ucSdrOffset].threshold_support_byte );

		if( ucThresholdSupport & Lower_Fatal_Threshold )
		{
			memcpy( &pucSnsrvalue[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
			printf( "Lower fatal limit :%d\n", *pucSnsrvalue );
#endif
			assert_memory_equal( pucSnsrvalue, pxAssertSdrDataV80[ucSdrOffset].lower_fatal_limit, ucSnsrlen );
			usRespIndex += ucSnsrlen;
		}

		if( ucThresholdSupport & Lower_Critical_Threshold )
		{
			memcpy( &pucSnsrvalue[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
			printf( "Lower critical limit :%d\n", *pucSnsrvalue );
#endif
			assert_memory_equal( pucSnsrvalue, pxAssertSdrDataV80[ucSdrOffset].lower_critical_limit, ucSnsrlen );
			usRespIndex += ucSnsrlen;
		}

		if( ucThresholdSupport & Lower_Warning_Threshold )
		{
			memcpy( &pucSnsrvalue[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
			printf( "Lower warning limit :%d\n", *pucSnsrvalue );
#endif
			assert_memory_equal( pucSnsrvalue, pxAssertSdrDataV80[ucSdrOffset].lower_warning_limit, ucSnsrlen );
			usRespIndex += ucSnsrlen;
		}

		if( ucThresholdSupport & Upper_Fatal_Threshold )
		{
			memcpy( &pucSnsrvalue[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
			printf( "Upper fatal limit :%d\n", *pucSnsrvalue );
#endif
			assert_memory_equal( pucSnsrvalue, pxAssertSdrDataV80[ucSdrOffset].upper_fatal_limit, ucSnsrlen );
			usRespIndex += ucSnsrlen;
		}

		if( ucThresholdSupport & Upper_Critical_Threshold )
		{
			memcpy( &pucSnsrvalue[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
			printf( "Upper Critical limit :%d\n", *pucSnsrvalue );
#endif
			assert_memory_equal( pucSnsrvalue, pxAssertSdrDataV80[ucSdrOffset].upper_critical_limit, ucSnsrlen );
			usRespIndex += ucSnsrlen;
		}

		if( ucThresholdSupport & Upper_Warning_Threshold )
		{
			memcpy( &pucSnsrvalue[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
			printf( "Upper Warning limit :%d\n", *snsrvalue );
#endif
			assert_memory_equal( pucSnsrvalue, pxAssertSdrDataV80[ucSdrOffset].upper_warning_limit, ucSnsrlen );
			usRespIndex += ucSnsrlen;
		}

#ifdef PRINT_SDR
		printf( "Sensor status : %x\n", pucRespBuffer[usRespIndex] );
#endif
		assert_true( pucRespBuffer[usRespIndex++] == pxAssertSdrDataV80[ucSdrOffset].sensor_status );

		if( ucThresholdSupport & Sensor_Avg_Val_Support )
		{
			memcpy( &pucSnsrvalue[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
			if( ucSnsrlen == 2 )
			{
				printf( "Sensor Avg value : %d\n", ( *snsrvalue | ( *( snsrvalue + 1 ) << 8 ) ) );
			}
			else if( ucSnsrlen == 4 )
			{
				printf( "Sensor Avg value : %d\n", ( *snsrvalue | ( *( snsrvalue + 1 ) << 8 ) | ( *( snsrvalue + 2 ) << 16 ) | ( *( snsrvalue + 3 ) << 24 ) ) );
			}
#endif
			assert_memory_equal( pucSnsrvalue, pxAssertSdrDataV80[ucSdrOffset].sensorAverageValue, ucSnsrlen );
			usRespIndex += ucSnsrlen;
		}

		if( ucThresholdSupport & Sensor_Max_Val_Support )
		{
			memcpy( &pucSnsrvalue[0], &pucRespBuffer[usRespIndex], ucSnsrlen );
#ifdef PRINT_SDR
			if( ucSnsrlen == 2 )
			{
				printf( "Sensor Max value : %d\n", ( *snsrvalue | ( *( snsrvalue + 1 ) << 8 ) ) );
			}
			else if( ucSnsrlen == 4 )
			{
				printf( "Sensor Max value : %d\n", ( *snsrvalue | ( *( snsrvalue + 1 ) << 8 ) | ( *( snsrvalue + 2 ) << 16 ) | ( *( snsrvalue + 3 ) << 24 ) ) );
			}
#endif
			assert_memory_equal( pucSnsrvalue, pxAssertSdrDataV80[ucSdrOffset].sensorMaxValue, ucSnsrlen );
			usRespIndex += ucSnsrlen;
		}
		ucSdrOffset++;
		free( pucSnsrvalue );
	}
}
