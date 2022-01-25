#ifndef INCLUDE_GPIO_H_
#define INCLUDE_GPIO_H_

#define GPIO_PIN_SPI_FLASH_CS				(9u)
#define GPIO_PIN_FRU_EEPROM_WP				(12u)
#define GPIO_PIN_VPD_EEPROM_WP				(13u)
#define GPIO_PIN_VPD_EEPROM_SMBUS_SELECT	(25u)

#define DEASSERTED 0
#define ASSERTED 1

#define LPD_GPIO_PORT	0
#define PMC_GPIO_PORT	1

#define OUTPUT_PIN	1
#define INPUT_PIN	0

u8 GPIOInit(void);



u8 GPIOReadInput(uint32_t port, uint32_t theBit, uint32_t *value);
u8 GPIOWriteOutput(uint32_t port, uint32_t theBit, uint32_t value);


#endif

