#include <string.h>
#include "ch.h"
#include "hal.h"
#include "eeprom.h"
#include "eeprom_lld.h"

void EEPROM__init(EEPROMDriver_t *eeprom) {
	osalDbgCheck(eeprom != NULL);
	osalDbgCheck(eeprom->state == EEPROM_NINIT);
	eeprom->state = EEPROM_READY;
}

/**
 *
 * @param eeprom
 */
void EEPROM__acquireDevice(EEPROMDriver_t *eeprom) {
	osalDbgCheck(eeprom != NULL);
	osalMutexLock(&eeprom->mutex);
}

/**
 *
 * @param eeprom
 */
void EEPROM__releaseDevice(EEPROMDriver_t *eeprom) {
	osalDbgCheck(eeprom != NULL);
	osalMutexUnlock(&eeprom->mutex);
}

/**
 *
 */
void EEPROM__read(EEPROMDriver_t *eeprom, uint32_t addr, uint8_t *data, uint32_t size){
	eepromRead(eeprom, addr, data, size);
}

/**
 *
 */
void EEPROM__write(EEPROMDriver_t *eeprom, uint32_t addr, uint8_t *data, uint32_t size){
	writeRegistersToEeprom(eeprom, addr, data, size);
}
