#ifndef POWER_THREAD_H
#define POWER_THREAD_H

#include "hal.h"

#define ADC_POWER_GRP_CHARGE_NUM_CHANNELS 2
#define ADC_POWER_GRP_CHARGEBUF_DEPTH 1

#define POWER_NAME "Power"

typedef struct {
	uint32_t hwId;
	const ADCConversionGroup *adcGroup;
	ADCDriver *adcDriver;
	uint32_t interval;
	uint32_t chrgInfoLine;
	uint32_t stdbyInfoLine;
	uint32_t chargeEnLine;
} power__threadConfig_t;

void Power__thread_init(const power__threadConfig_t *cfg);
void Power__thread_start(void);

#endif