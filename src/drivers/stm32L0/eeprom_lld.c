#include "ch.h"
#include "hal.h"
#include "eeprom_lld.h"

EEPROMDriver_t EEPROMD1;

void eepromProgram(EEPROMDriver_t *EEPROMD, uint32_t addr, uint8_t data);
msg_t eepromWaitForReady();

void unlockEeprom(EEPROMDriver_t *EEPROMD);
void lockEeprom(EEPROMDriver_t *EEPROMD, eepromState_t state);

/**
 *
 */
void writeRegistersToEeprom(EEPROMDriver_t *EEPROMD, uint32_t addr, uint8_t *data, uint32_t size){
	chSysLock();
	osalDbgAssert(EEPROMD->state == EEPROM_READY, "not ready");
	EEPROMD->state = EEPROM_ACTIVE_WRITE;
	chSysUnlock();

	uint8_t * p_tFlashRegs;
	p_tFlashRegs = data;   /* Point to data to write */
	unlockEeprom(EEPROMD); /* Unlock the EEPROM */

	FLASH->PECR = FLASH->PECR & ~(FLASH_PECR_ERASE | FLASH_PECR_DATA); /* Reset the ERASE and DATA  bits in the FLASH_PECR register to disable any residual erase */

	for(uint32_t i = 0; i < size; i++ ){
		osalDbgAssert(EEPROMD->state == EEPROM_UNLOCKED, "eeprom failed");
		eepromProgram(EEPROMD, DATA_EEPROM_BASE + addr + i, *(p_tFlashRegs + i)); /* Increase eeprom address by 4 for each word to write.  */
	}

	lockEeprom(EEPROMD, EEPROM_READY); /* Lock the EEPROM */
}

/**
 *
 * @param addr
 * @param data
 * @param size
 */
void eepromRead(EEPROMDriver_t *EEPROMD, uint32_t addr, uint8_t *data, uint32_t size){
	chSysLock();
	osalDbgAssert(EEPROMD->state == EEPROM_READY, "not ready");

	if(eepromWaitForReady() != MSG_OK){
		EEPROMD->state = EEPROM_FAILED;
		chSysUnlock();
		return;
	}

	for(uint32_t i = 0; i < size; i++ ){
		*data = *(__IO uint8_t *)(DATA_EEPROM_BASE + addr + i);
		data++;
	}
	chSysUnlock();
}

/**
 *
 * @param addr
 * @param data
 * @return
 */
void eepromProgram(EEPROMDriver_t *EEPROMD, uint32_t addr, uint8_t data){
	osalDbgAssert(EEPROMD->state == EEPROM_UNLOCKED, "not locked");

	chSysLock();
	if(eepromWaitForReady() != MSG_OK){
		EEPROMD->state = EEPROM_FAILED;
		chSysUnlock();
		return;
	}

	*(__IO uint8_t *)(addr) = data; /* write data to EEPROM */
	chSysUnlock();
}

/**
 *
 */
msg_t eepromWaitForReady(){
	systime_t start = osalOsGetSystemTimeX();
	systime_t end = osalTimeAddX(start, OSAL_MS2I(1000));
	while ((FLASH->SR & FLASH_SR_BSY) != 0){
		if (!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end)) {
			return MSG_TIMEOUT;
		}
	}

	return MSG_OK;
}

/**
 *
 */
void lockEeprom(EEPROMDriver_t *EEPROMD, eepromState_t state){
	osalDbgAssert(EEPROMD->state == EEPROM_UNLOCKED, "unexpected state");

	if(eepromWaitForReady() != MSG_OK){
		EEPROMD->state = EEPROM_FAILED;
		return;
	}

	chSysLock();
	FLASH->PECR = FLASH->PECR & ~(FLASH_PECR_ERRIE | FLASH_PECR_EOPIE); /* disable flash interrupts */
	FLASH->PECR = FLASH->PECR | FLASH_PECR_PELOCK; /* Lock memory with PELOCK */
	EEPROMD->state = state;
	chSysUnlock();
}

/**
 *
 */
void unlockEeprom(EEPROMDriver_t *EEPROMD){
	osalDbgAssert(EEPROMD->state == EEPROM_ACTIVE_WRITE, "unexpected state");

	if(eepromWaitForReady() != MSG_OK){
		EEPROMD->state = EEPROM_FAILED;
		return;
	}

	/* If PELOCK is locked */
	if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0){
		chSysLock();
		FLASH->PEKEYR = FLASH_PEKEY1; /* Unlock PELOCK */
		FLASH->PEKEYR = FLASH_PEKEY2;
		EEPROMD->state = EEPROM_UNLOCKED;
		chSysUnlock();
	}

	//FLASH->PECR = FLASH->PECR | (FLASH_PECR_ERRIE | FLASH_PECR_EOPIE); /* enable flash interrupts */
}

