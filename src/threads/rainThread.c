#include "ch.h"
#include "hal.h"
#include "main.h"
#include "rainThread.h"
#include "hwListThread.h"
#include "scheduleListThread.h"
#include "pools.h"
#include "msp.h"

#define RAIN_STATUS 0

static const rain__threadConfig_t *rainThreadCfg;
static adcsample_t adcRainSamples[ADC_RAIN_GRP_CHARGE_NUM_CHANNELS * ADC_RAIN_GRP_CHARGEBUF_DEPTH];
static hw_t rainHW;
static schedule_t   rainSchedule;
static uint16_t     interval;

void Rain__thread_setInterval(uint16_t i);

static THD_WORKING_AREA(RAINVA, 70);
static THD_FUNCTION(rainThread, arg) {
	(void) arg;
	chRegSetThreadName("Rain");
	msg_t msg;

	///////////////////////////////////////////////////////////////
	// REGISTER SCHEDULE
	///////////////////////////////////////////////////////////////
	rainSchedule.id = rainThreadCfg->hwId;
	rainSchedule.name = RAIN_NAME;
	rainSchedule.interval = &interval;
	rainSchedule.setInterval = &Rain__thread_setInterval;

	(void) chMBPostTimeout(&registerScheduleMail, (msg_t) &rainSchedule, TIME_IMMEDIATE);
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	rainHW.id = rainThreadCfg->hwId;
	rainHW.type = VALUE_TYPE_SENSOR;
	rainHW.name = RAIN_NAME;
	rainHW.status = HW_STATUS_UNKNOWN;

	rainHW.values[RAIN_STATUS].formatter = VALUE_FORMATTER_BOOL;
	rainHW.values[RAIN_STATUS].name = "Rain status";

	(void) chMBPostTimeout(&registerHwMail, (msg_t) &rainHW, TIME_IMMEDIATE);
	///////////////////////////////////////////////////////////////

	uint32_t streamBuff[1];
	while (true) {

		adcAcquireBus(rainThreadCfg->adcDriver);
		adcStart(rainThreadCfg->adcDriver, NULL);
		msg = adcConvert(rainThreadCfg->adcDriver, rainThreadCfg->adcGroup, adcRainSamples, ADC_RAIN_GRP_CHARGEBUF_DEPTH);
		adcStop(rainThreadCfg->adcDriver);
		adcReleaseBus(rainThreadCfg->adcDriver);

		if(msg == MSG_OK){
			chSysLock();
			streamBuff[0] = rainHW.values[RAIN_STATUS].value = adcRainSamples[0] < 3400 ? 1 : 0;
			rainHW.status = HW_STATUS_OK;
			chSysUnlock();

			poolStreamObject_t* messagePoolObject = (poolStreamObject_t *) chPoolAlloc(&streamMemPool);
			if (messagePoolObject) {
				MSP__createMspFrame(messagePoolObject, (uint8_t)rainHW.id, 1, (uint32_t*)&streamBuff);
				chMBPostTimeout(&streamMail, (msg_t) messagePoolObject, TIME_IMMEDIATE);
			}

		}else{
			rainHW.status = HW_STATUS_ERROR;
		}

		chThdSleepMilliseconds(interval);
	}
}

/**
 *
 */
void Rain__thread_init(const rain__threadConfig_t *cfg) {
	osalDbgCheck(cfg->adcGroup->num_channels == ADC_RAIN_GRP_CHARGE_NUM_CHANNELS);
	rainThreadCfg = cfg;
	interval = rainThreadCfg->interval;
}

/**
 *
 */
void Rain__thread_start(void) {
	chThdCreateStatic(RAINVA, sizeof(RAINVA), THREAD_PRIORITY_RAIN, rainThread,NULL);
}

/**
 *
 */
void Rain__thread_setInterval(uint16_t i) {
	interval = i;
}
