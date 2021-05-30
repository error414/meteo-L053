#ifndef AS3935_THREAD_H
#define AS3935_THREAD_H

#include "hal.h"
#define AS3935_NAME "AS3935"

typedef struct {
	const uint32_t hwId;
	I2CDriver *driver;
	uint16_t interval; //second
	uint32_t interruptLine;
	bool (*checkI2cFunc)(I2CDriver *driver);
	uint8_t i2cAddr;
} As3935__threadConfig_t;

void As3935__thread_init(As3935__threadConfig_t *cfg);
void As3935__thread_start(void);
bool As3935__thread_autoTune(BaseSequentialStream *outStream);

#endif