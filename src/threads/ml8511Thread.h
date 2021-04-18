#ifndef ML8511_THREAD_H
#define ML8511_THREAD_H

#include "hal.h"
#include "ml8511.h"

#define ML8511_NAME "ML8511"

typedef struct {
	uint32_t hwId;
	ADCDriver *adcDriver;
	const ADCConversionGroup *adcGroup;
	uint32_t driverEnableLine;
	uint32_t enablePinLine;
	uint16_t interval; //ms
} ML8511__threadConfig_t;

void Ml8511__thread_init(const ML8511__threadConfig_t *cfg);
void Ml8511__thread_start(void);

#endif