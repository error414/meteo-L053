#ifndef BH1750_THREAD_H
#define BH1750_THREAD_H

#include "hal.h"

#define BH11750_NAME "BH1750"

typedef struct {
	const uint32_t hwId;
	I2CDriver *driver;
	uint32_t enablePinLine;
	uint16_t interval; //second
} BH1750__threadConfig_t;

void Bh1750__thread_init(BH1750__threadConfig_t *cfg);
void Bh1750__thread_start(void);

#endif