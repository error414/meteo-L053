#include "ch.h"
#include "main.h"
#include "ml8511Thread.h"
#include "ml8511.h"
#include "pools.h"
#include "msp.h"
#include "hwListThread.h"
#include "scheduleListThread.h"

#define ML8511_UV 0

static ML8511__threadConfig_t *ml8511ThreadCfg;
static ml8511_cfg_t driverCfg;
static hw_t ml8511HW;
static schedule_t   ml8511Schedule;

void Ml8511__thread_setInterval(uint16_t i);

static THD_WORKING_AREA(ML8511VA, 40);
static THD_FUNCTION(ML8511Thread, arg) {
	(void) arg;
	chRegSetThreadName("Ml8511");

	uint32_t streamBuff[1];
	float uvLight;

	driverCfg.adcGroup         = ml8511ThreadCfg->adcGroup;
	driverCfg.adcDriver        = ml8511ThreadCfg->adcDriver;
	driverCfg.enableLine       = ml8511ThreadCfg->driverEnableLine;

	///////////////////////////////////////////////////////////////
	// REGISTER SCHEDULE
	///////////////////////////////////////////////////////////////
	ml8511Schedule.id       = ml8511ThreadCfg->hwId;
	ml8511Schedule.name     = ML8511_NAME;
	ml8511Schedule.interval = &ml8511ThreadCfg->interval;
	ml8511Schedule.tp       = chThdGetSelfX();

	(void) chMBPostTimeout(&registerScheduleMail, (msg_t) &ml8511Schedule, TIME_IMMEDIATE);
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	ml8511HW.id = ml8511ThreadCfg->hwId;
	ml8511HW.type = VALUE_TYPE_SENSOR;
	ml8511HW.name = ML8511_NAME;
	ml8511HW.status = HW_STATUS_UNKNOWN;

	ml8511HW.values[ML8511_UV].formatter = VALUE_FORMATTER_100;
	ml8511HW.values[ML8511_UV].name = "UV";

	(void) chMBPostTimeout(&registerHwMail, (msg_t) &ml8511HW, TIME_IMMEDIATE); // after this must be access bmp280HW atomically
	///////////////////////////////////////////////////////////////

	ML8511_init(&driverCfg);
	while (true) {
		uvLight = ML8511_getUV();
		if(uvLight > 0.0f){
			chSysLock();
			streamBuff[0] = ml8511HW.values[ML8511_UV].value = (uint32_t)(uvLight * 100);
			ml8511HW.status = HW_STATUS_OK;
			chSysUnlock();

			poolStreamObject_t* messagePoolObject = (poolStreamObject_t *) chPoolAlloc(&streamMemPool);
			if (messagePoolObject) {
				MSP__createMspFrame(messagePoolObject, (uint8_t)ml8511HW.id, 1, (uint32_t*)&streamBuff);
				chMBPostTimeout(&streamMail, (msg_t) messagePoolObject, TIME_IMMEDIATE);
			}

		}else{
			ml8511HW.status = HW_STATUS_ERROR;
		}

		//load new configuration if needed
		thread_t *tp = chMsgWaitTimeout(ml8511ThreadCfg->interval * 1000);
		if(tp){
			ml8511ThreadCfg->interval = (uint16_t)chMsgGet(tp);
			chMsgRelease(tp, MSG_OK);
		}
	}
}

/**
 *
 */
void Ml8511__thread_init(ML8511__threadConfig_t *cfg) {
	ml8511ThreadCfg = cfg;
	if (cfg->driverEnableLine != LINE_NO_PIN){
		palSetLineMode(cfg->driverEnableLine, PAL_STM32_MODE_OUTPUT);
	}

	if (cfg->enablePinLine != LINE_NO_PIN){
		palSetLineMode(cfg->enablePinLine, PAL_STM32_MODE_OUTPUT);
		palSetLine(cfg->enablePinLine);
	}
}

/**
 *
 */
void Ml8511__thread_start(void) {
	chThdCreateStatic(ML8511VA, sizeof(ML8511VA), THREAD_PRIORITY_ML8511, ML8511Thread,NULL);
}