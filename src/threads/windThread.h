#ifndef WIND_THREAD_H
#define WIND_THREAD_H

#include "hal.h"


#define WIND_NAME "Wind"

typedef struct {
	uint32_t hwId;
	uint32_t windLine;
	uint32_t interval;
} wind__threadConfig_t;

void Wind__thread_init(const wind__threadConfig_t *cfg);
void Wind__thread_start(void);

#endif