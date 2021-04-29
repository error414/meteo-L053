#ifndef MAIN_H
#define MAIN_H

#include "hal.h"
#include "scheduleListThread.h"

#define NEWLINE_STR                     "\r\n"
#define NEWLINE_STR_SIMPLE              "\n"

//THREADS PRIORITY
#define THREAD_PRIORITY_BMP280          NORMALPRIO + 10
#define THREAD_PRIORITY_BH1750          NORMALPRIO + 10
#define THREAD_PRIORITY_ML8511          NORMALPRIO + 10
#define THREAD_PRIORITY_ADCPOWER        NORMALPRIO + 10
#define THREAD_PRIORITY_WIND            NORMALPRIO + 10
#define THREAD_PRIORITY_HC12            NORMALPRIO + 10
#define THREAD_PRIORITY_RAIN            NORMALPRIO + 10

#define THREAD_PRIORITY_REGISTER_HW        NORMALPRIO + 20
#define THREAD_PRIORITY_REGISTER_SCH        NORMALPRIO + 20

#define CONFIG_BASE_ADDR 0
#define VERSION 1
#define DEFAULT_INTERVAL 10

#define BMP280_HW_ID 1
#define BH1750_HW_ID 2
#define ML8511_HW_ID 3
#define POWER_HW_ID 4
#define WIND_HW_ID 5
#define RAIN_HW_ID 6

typedef struct {
	uint8_t version;
	uint8_t interval[SCHEDULE_LIST_SIZE];
} appConfiguration_t;

extern appConfiguration_t appConfiguration;
void checkI2CCondition(I2CDriver *driver);

#endif