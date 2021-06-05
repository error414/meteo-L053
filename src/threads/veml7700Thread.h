#ifndef VEML7700_THREAD_H
#define VEML7700_THREAD_H

#include "hal.h"

#define VEML7700_NAME "VEML7700"

typedef struct {
	const uint32_t hwId;
	I2CDriver *driver;
	uint32_t enablePinLine;
	uint16_t interval; //second
	bool (*checkI2cFunc)(I2CDriver *driver);
} VEML7700__threadConfig_t;

void Veml7700__thread_init(VEML7700__threadConfig_t *cfg);
void Veml7700__thread_start(void);

#endif