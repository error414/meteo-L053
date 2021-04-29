#ifndef RAIN_THREAD_H
#define RAIN_THREAD_H

#include "hal.h"

#define ADC_RAIN_GRP_CHARGE_NUM_CHANNELS 1
#define ADC_RAIN_GRP_CHARGEBUF_DEPTH 1

#define RAIN_NAME "Rain"

typedef struct {
	uint32_t hwId;
	const ADCConversionGroup *adcGroup;
	ADCDriver *adcDriver;
	uint32_t interval;
} rain__threadConfig_t;

void Rain__thread_init(const rain__threadConfig_t *cfg);
void Rain__thread_start(void);

#endif