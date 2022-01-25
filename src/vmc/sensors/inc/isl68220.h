


#ifndef ISL68220_H_
#define ISL68220_H_
 
#include <stdint.h>
#include <stdbool.h>
#include "xil_types.h"

#define ISL68220_SLV_ADDR		(0x60)

#define ISL68220_PAGE_REGISTER                  0x00
#define ISL68220_OPERATION                      0x01
#define ISL68220_ON_OFF_CONFIG_REGISTER         0x02

//#define ISL68220_SELECT_VDDQ_0_1                0x00
//#define ISL68220_SELECT_VDDQ_2_3                0x01
//
//#define VOUT_MARGIN_HIGH_MULTIPLIER             (float)(1.05)
//#define VOUT_MARGIN_LOW_MULTIPLIER              (float)(0.95)
//#define VOUT_OV_FAULT_LIMIT_MULTIPLIER          (float)(1.10)
//#define VOUT_UV_FAULT_LIMIT_MULTIPLIER          (float)(0.90)

//Commands for ISL68220 device.
#define ISL68220_PAGE_REGISTER                  0x00
#define ISL68220_OPERATION                      0x01
#define ISL68220_SELECT_PAGE_VCCINT             0x00

#define ISL68220_SELECT_PAGE_VCCINT_BRAM        0x01

#define ISL68220_WRITE_PROTECT                  0x10

#define ISL68220_VOUT_COMMAND                   0x21

#define ISL68220_VOUT_MAX                       0x24
#define ISL68220_VOUT_MARGIN_HIGH               0x25
#define ISL68220_VOUT_MARGIN_LOW                0x26
#define ISL68220_VOUT_MIN                       0x2B
#define ISL68220_VOUT_OV_FAULT_LIMIT            0x40
#define ISL68220_VOUT_UV_FAULT_LIMIT            0x44
#define ISL68220_STATUS_WORD                    0x79
#define ISL68220_OUTPUT_VOLTAGE_REGISTER        0x8B
#define ISL68220_OUTPUT_CURRENT_REGISTER    	0x8C
#define ISL68220_READ_POWERSTAGE_TEMPERATURE    0x8D
#define ISL68220_READ_INTERNAL_TEMPERATURE   	0x8E
#define ISL68220_READ_PIN_TEMPERATURE    		0x8F
#define ISL68220_RESTORE_CONFIG_REGISTER        0xF2



// ID of VCCINT config settings.
enum ISL68220_CONFIG{
	CONFIG_R_0,
	CONFIG_R_162,
	CONFIG_R_316,
	CONFIG_R_487,
	CONFIG_R_681,
	CONFIG_R_887,
	CONFIG_R_1130,
	CONFIG_R_1370,
	CONFIG_R_1650,
	CONFIG_R_1960,
	CONFIG_R_2320,
	CONFIG_R_2670,
	CONFIG_R_3090,
	CONFIG_R_3570,
	CONFIG_R_4120,
	CONFIG_R_4640,
};

/**
* @brief This function is used to read any register on the ISL68220
* @param[in] i2c                - I2C Handle
* @param[in] SlaveAddr          - Slave address of ISL68220
* @param[in] register_address   - Register address to be read
* @param[out]register_content   - Register content
*
* @return    true    - Success
* @return    false   - Failed
*
* @note      None
**
******************************************************************************/
bool ISL68220_set_vccint_config(u8 busnum, uint8_t SlaveAddr, uint8_t vccint_opt);

bool ISL68220_read_one_byte_register(u8 busnum, uint8_t SlaveAddr, uint8_t register_address, uint8_t *register_content);

bool ISL68220_read_register(u8 busnum, uint8_t SlaveAddr, uint8_t register_address, uint8_t *register_content);

bool ISL68220_VOUT_write(u8 busnum, uint8_t slave_addr, uint8_t *register_content);

bool ISL68220_VOUT_MARGIN_HIGH_write(u8 busnum, uint8_t slave_addr, uint8_t *register_content);

bool ISL68220_VOUT_OV_FAULT_LIMIT_write(u8 busnum, uint8_t slave_addr, uint8_t *register_content);

bool ISL68220_APPLY_SETTINGS_write(u8 busnum, uint8_t slave_addr, uint8_t *register_content);

bool ISL68220_VOUT_UV_FAULT_LIMIT_write(u8 busnum, uint8_t slave_addr, uint8_t *register_content);

bool ISL68220_VOUT_MARGIN_LOW_write(u8 busnum, uint8_t slave_addr, uint8_t *register_content);

bool ISL68220_set_vout_max(u8 busnum, uint8_t slave_addr);

bool ISL68220_set_vout_min(u8 busnum, uint8_t slave_addr);

bool ISL68220_write_register(u8 busnum, uint8_t SlaveAddr, uint8_t register_address, uint8_t *register_content);

bool ISL68220_Read_VCCINT_Voltage(u8 busnum, uint8_t SlaveAddr, uint16_t *RegisterValue);

bool ISL68220_Read_VCCINT_Current(u8 busnum,uint8_t SlaveAddr, float *RegisterValue);

bool ISL68220_Read_VCCINT_BRAM_Voltage(u8 busnum, uint8_t SlaveAddr, uint16_t *RegisterValue);

bool ISL68220_Read_VCCINT_BRAM_Current(u8 busnum,uint8_t SlaveAddr, float *RegisterValue);

bool ISL68220_Read_VCCINT_Temperature(u8 busnum,uint8_t SlaveAddr, float *RegisterValue);

bool ISL68220_Read_VCCINT_BRAM_Temperature(u8 busnum,uint8_t SlaveAddr, float *RegisterValue);

bool ISL68220_Read_Temperature(u8 busnum,uint8_t SlaveAddr, u8 tempReg, float *RegisterValue);

#endif /* ISL68220_H_ */
