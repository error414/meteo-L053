#include "ch.h"
#include "hal.h"
#include "main.h"
#include "bh1750Thread.h"
#include "bh1750.h"
#include "pools.h"
#include "msp.h"
#include "hwListThread.h"
#include "scheduleListThread.h"

#define BH1750_LIGHT 0

static BH1750_HandleTypedef BH1750_dev;
static BH1750__threadConfig_t *bh1750ThreadCfg;

static hw_t bh1750HW;
static schedule_t   bh1750Schedule;

static THD_WORKING_AREA(BH1750VA, 140);
static THD_FUNCTION(BH1750Thread, arg) {
	(void) arg;
	chRegSetThreadName("Bh1750");

	uint32_t streamBuff[1];

	///////////////////////////////////////////////////////////////
	// REGISTER SCHEDULE
	///////////////////////////////////////////////////////////////
	bh1750Schedule.id       = bh1750ThreadCfg->hwId;
	bh1750Schedule.name     = BH11750_NAME;
	bh1750Schedule.interval = &bh1750ThreadCfg->interval;
	bh1750Schedule.tp       = chThdGetSelfX();

	(void) chMBPostTimeout(&registerScheduleMail, (msg_t) &bh1750Schedule, TIME_IMMEDIATE);
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	bh1750HW.id         = bh1750ThreadCfg->hwId;
	bh1750HW.type       = VALUE_TYPE_SENSOR;
	bh1750HW.name       = BH11750_NAME;
	bh1750HW.status     = HW_STATUS_UNKNOWN;

	bh1750HW.values[BH1750_LIGHT].formatter = VALUE_FORMATTER_NONE;
	bh1750HW.values[BH1750_LIGHT].name = "Lux";

	BH1750_init(&BH1750_dev);

	(void) chMBPostTimeout(&registerHwMail, (msg_t) &bh1750HW, TIME_IMMEDIATE); // after this must be access bmp280HW atomically
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	while (true) {
		checkI2CCondition(bh1750ThreadCfg->driver);

		if(bh1750HW.status == HW_STATUS_ERROR){
			BH1750_init(&BH1750_dev); // try reconfigure
		}

		if(BH1750_get_lumen(&BH1750_dev)){
			chSysLock();
			streamBuff[0] = bh1750HW.values[BH1750_LIGHT].value = (uint32_t)(BH1750_dev.value);
							bh1750HW.status = HW_STATUS_OK;
			chSysUnlock();

			poolStreamObject_t* messagePoolObject = (poolStreamObject_t *) chPoolAlloc(&streamMemPool);
			if (messagePoolObject) {
				MSP__createMspFrame(messagePoolObject, (uint8_t)bh1750HW.id, 1, (uint32_t*)&streamBuff);
				chMBPostTimeout(&streamMail, (msg_t) messagePoolObject, TIME_IMMEDIATE);
			}

		}else{
			bh1750HW.status = HW_STATUS_ERROR;
		}

		//load new configuration if needed
		thread_t *tp = chMsgWaitTimeout(bh1750ThreadCfg->interval * 1000);
		if(tp){
			bh1750ThreadCfg->interval = (uint16_t)chMsgGet(tp);
			chMsgRelease(tp, MSG_OK);
		}
	}
}

/**
 *
 */
void Bh1750__thread_init(BH1750__threadConfig_t *cfg) {
	BH1750_init_default_params(&BH1750_dev, cfg->driver, true);
	bh1750ThreadCfg = cfg;
}

/**
 *
 */
void Bh1750__thread_start(void) {
	chThdCreateStatic(BH1750VA, sizeof(BH1750VA), THREAD_PRIORITY_BH1750, BH1750Thread,NULL);
}