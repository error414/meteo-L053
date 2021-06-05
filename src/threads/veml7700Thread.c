#include "ch.h"
#include "hal.h"
#include "main.h"
#include "veml7700Thread.h"
#include "veml7700.h"
#include "bh1750.h"
#include "pools.h"
#include "msp.h"
#include "hwListThread.h"
#include "scheduleListThread.h"

#define VEML7700_LIGHT 0

static VEML7700_HandleTypedef VEML7700_dev;
static VEML7700__threadConfig_t *veml7700ThreadCfg;

static hw_t veml7700HW;
static schedule_t veml7700Schedule;

static THD_WORKING_AREA(VEML7700VA, 160);
static THD_FUNCTION(VEML7700Thread, arg) {
	(void) arg;
	chRegSetThreadName("Veml7700");

	uint32_t streamBuff[1];

	///////////////////////////////////////////////////////////////
	// REGISTER SCHEDULE
	///////////////////////////////////////////////////////////////
	veml7700Schedule.id       = veml7700ThreadCfg->hwId;
	veml7700Schedule.name     = VEML7700_NAME;
	veml7700Schedule.interval = &veml7700ThreadCfg->interval;
	veml7700Schedule.tp       = chThdGetSelfX();

	(void) chMBPostTimeout(&registerScheduleMail, (msg_t) &veml7700Schedule, TIME_IMMEDIATE);
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	veml7700HW.id         = veml7700ThreadCfg->hwId;
	veml7700HW.type       = VALUE_TYPE_SENSOR;
	veml7700HW.name       = VEML7700_NAME;
	veml7700HW.status     = VEML7700_init(&VEML7700_dev) ? HW_STATUS_OK : HW_STATUS_ERROR;

	veml7700HW.values[VEML7700_LIGHT].formatter = VALUE_FORMATTER_NONE;
	veml7700HW.values[VEML7700_LIGHT].name = "Lux";

	(void) chMBPostTimeout(&registerHwMail, (msg_t) &veml7700HW, TIME_IMMEDIATE); // after this must be access bmp280HW atomically
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	while (true) {
		if(veml7700HW.status == HW_STATUS_ERROR){
            VEML7700_init(&VEML7700_dev); // try reconfigure
            chThdSleepMilliseconds(100);
		}

		if(VEML7700_getALSLux(&VEML7700_dev)){
			chSysLock();
			streamBuff[0] = veml7700HW.values[VEML7700_LIGHT].value = (uint32_t)(VEML7700_dev.lux);
							veml7700HW.status = HW_STATUS_OK;
			chSysUnlock();

			poolStreamObject_t* messagePoolObject = (poolStreamObject_t *) chPoolAlloc(&streamMemPool);
			if (messagePoolObject) {
				MSP__createMspFrame(messagePoolObject, (uint8_t)veml7700HW.id, 1, (uint32_t*)&streamBuff);
				chMBPostTimeout(&streamMail, (msg_t) messagePoolObject, TIME_IMMEDIATE);
			}

		}else{
			veml7700HW.status = HW_STATUS_ERROR;
		}

		//load new configuration if needed
		thread_t *tp = chMsgWaitTimeout(veml7700ThreadCfg->interval * 1000);
		if(tp){
			veml7700ThreadCfg->interval = (uint16_t)chMsgGet(tp);
			chMsgRelease(tp, MSG_OK);
		}
	}
}

/**
 *
 */
void Veml7700__thread_init(VEML7700__threadConfig_t *cfg) {
	VEML7700_init_default_params(&VEML7700_dev, cfg->driver);
	VEML7700_dev.checkI2cFunc = cfg->checkI2cFunc;
	veml7700ThreadCfg = cfg;

	if(cfg->enablePinLine != LINE_NO_PIN){
		palSetLineMode(cfg->enablePinLine, PAL_STM32_MODE_OUTPUT);
		palSetLine(cfg->enablePinLine);
	}
}

/**
 *
 */
void Veml7700__thread_start(void) {
	chThdCreateStatic(VEML7700VA, sizeof(VEML7700VA), THREAD_PRIORITY_VEML7700, VEML7700Thread,NULL);
}