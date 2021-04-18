#ifndef MSP_H
#define MSP_H

#include "ch.h"
#include "pools.h"


void MSP__createMspFrame(poolStreamObject_t *streamObject, uint8_t cmd, uint32_t argc, const uint32_t *argv);

#endif