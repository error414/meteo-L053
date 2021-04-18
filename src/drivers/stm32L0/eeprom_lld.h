#ifndef EEPROM_LLD_H
#define EEPROM_LLD_H

#ifndef DATA_EEPROM_BASE
#error "DATA_EEPROM_BASE must be defined"
#endif

#ifndef DATA_EEPROM_END
#error "DATA_EEPROM_END must be defined"
#endif

#define FLASH_PEKEY1        0x89ABCDEF
#define FLASH_PEKEY2        0x02030405

typedef enum {
	EEPROM_NINIT            = 0,
	EEPROM_READY            = 1,
	EEPROM_ACTIVE_WRITE     = 2,
	EEPROM_UNLOCKED         = 3,
	EEPROM_FAILED           = 4,
} eepromState_t;


typedef struct {
	eepromState_t             state;
	mutex_t                   mutex;
} EEPROMDriver_t;

extern EEPROMDriver_t EEPROMD1;

void writeRegistersToEeprom(EEPROMDriver_t *EEPROMD, uint32_t addr, uint8_t *data, uint32_t size);
void eepromRead(EEPROMDriver_t *EEPROMD, uint32_t addr, uint8_t *data, uint32_t size);


#endif

