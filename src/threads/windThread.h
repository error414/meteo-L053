#ifndef WIND_THREAD_H
#define WIND_THREAD_H

#include "hal.h"


#define WIND_NAME "Wind"

typedef struct {
	const uint32_t hwId;
	uint32_t windLine;
	uint16_t interval; //second
} wind__threadConfig_t;

void Wind__thread_init(wind__threadConfig_t *cfg);
void Wind__thread_start(void);

#endif