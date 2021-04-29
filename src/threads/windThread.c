#include "ch.h"
#include "hal.h"
#include "main.h"
#include "windThread.h"
#include "hwListThread.h"

#define WIND_VOLTAGE_SOLAR 0

static const wind__threadConfig_t *windThreadCfg;
static hw_t windHW;
static schedule_t   windSchedule;
static uint16_t     interval;
volatile uint32_t windSpeedCount = 0;

void Wind__thread_setInterval(uint16_t i);
void Wind__thread_increaseWindCount(void);

/*static void Power__chrg_it(void *arg);
static void Power__stdby_it(void *arg);*/

static THD_WORKING_AREA(WINDVA, 70);
static THD_FUNCTION(windThread, arg) {
	(void) arg;
	chRegSetThreadName("Wind");

	///////////////////////////////////////////////////////////////
	// REGISTER SCHEDULE
	///////////////////////////////////////////////////////////////
	windSchedule.id = windThreadCfg->hwId;
	windSchedule.name = WIND_NAME;
	windSchedule.interval = &interval;
	windSchedule.setInterval = &Wind__thread_setInterval;

	(void) chMBPostTimeout(&registerScheduleMail, (msg_t) &windSchedule, TIME_IMMEDIATE);
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	windHW.id = windThreadCfg->hwId;
	windHW.type = VALUE_TYPE_SENSOR;
	windHW.name = WIND_NAME;
	windHW.status = HW_STATUS_UNKNOWN;

	windHW.values[WIND_VOLTAGE_SOLAR].formatter = VALUE_FORMATTER_100;
	windHW.values[WIND_VOLTAGE_SOLAR].name = "Wind speed";
	(void) chMBPostTimeout(&registerHwMail, (msg_t) &windHW, TIME_IMMEDIATE);
	///////////////////////////////////////////////////////////////

	palEnableLineEvent(windThreadCfg->windLine, PAL_EVENT_MODE_FALLING_EDGE);
	palSetLineCallback(windThreadCfg->windLine, (void*)&Wind__thread_increaseWindCount, NULL);

	systime_t lastRunTime = chVTGetSystemTime();
	while (true) {
		if(windSpeedCount > 0){
			windHW.status = HW_STATUS_OK;
		}

		chSysLock();
		uint32_t time = chVTGetSystemTimeX() - lastRunTime;
		windHW.values[WIND_VOLTAGE_SOLAR].value = (uint32_t)(((1200.0 * windSpeedCount) / (float)time)) * 100;
		lastRunTime = chVTGetSystemTimeX();
		windSpeedCount = 0;
		chSysUnlock();

		chThdSleepMilliseconds(interval);
	}
}

/**
 *
 */
void Wind__thread_init(const wind__threadConfig_t *cfg) {
	windThreadCfg = cfg;
	interval = windThreadCfg->interval;
}

/**
 *
 */
void Wind__thread_start(void) {
	chThdCreateStatic(WINDVA, sizeof(WINDVA), THREAD_PRIORITY_WIND, windThread,NULL);
}

/**
 *
 */
void Wind__thread_setInterval(uint16_t i) {
	interval = i;
}

/**
 *
 */
void Wind__thread_increaseWindCount() {
	windSpeedCount++;
}