#ifndef EEPROM_H
#define EEPROM_H

#include "eeprom_lld.h"

void EEPROM__init(EEPROMDriver_t *eeprom);
void EEPROM__acquireDevice(EEPROMDriver_t *eeprom);
void EEPROM__releaseDevice(EEPROMDriver_t *eeprom);
void EEPROM__read(EEPROMDriver_t *eeprom, uint32_t addr, uint8_t *data, uint32_t size);
void EEPROM__write(EEPROMDriver_t *eeprom, uint32_t addr, uint8_t *data, uint32_t size);

#endif