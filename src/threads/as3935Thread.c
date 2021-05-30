
#include "ch.h"
#include "main.h"
#include "as3935Thread.h"
#include "PWFusion_AS3935_I2C.h"
#include "pools.h"
#include "msp.h"
#include "hwListThread.h"
#include "chprintf.h"

#ifdef USE_AS3935

#define AS3935_INTERRUPT_SRC    0
#define AS3935_DISTANCE         1
#define AS3935_ENERGY           2

void As3935__thread_IRQ(void *arg);
void As3935__thread_enableInterrupt(As3935__threadConfig_t *cfg);
void As3935__thread_disableInterrupt(As3935__threadConfig_t *cfg);

static AS3935_I2C_dev_t AS3935_dev;
static As3935__threadConfig_t *as3935ThreadCfg;

binary_semaphore_t      as3935_bsem;
static hw_t             as3935HW;
static schedule_t       as3935Schedule;
volatile uint32_t        interruptCount;


static THD_WORKING_AREA(AS3935VA, 220);
static THD_FUNCTION(As3935Thread, arg) {
	(void) arg;
	chRegSetThreadName("AS3935");
	uint32_t streamBuff[3];

	static uint8_t interruptSrc;
	static uint8_t distance;
	static uint32_t energy;

	///////////////////////////////////////////////////////////////
	// REGISTER SCHEDULE
	///////////////////////////////////////////////////////////////
	as3935Schedule.id       = as3935ThreadCfg->hwId;
	as3935Schedule.name     = AS3935_NAME;
	as3935Schedule.interval = &as3935ThreadCfg->interval;
	as3935Schedule.tp       = chThdGetSelfX();

	(void) chMBPostTimeout(&registerScheduleMail, (msg_t) &as3935Schedule, TIME_IMMEDIATE);
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	chBSemWait(&as3935_bsem);
	AS3935_init(&AS3935_dev);

	as3935HW.id = as3935ThreadCfg->hwId;
	as3935HW.type = VALUE_TYPE_SENSOR;
	as3935HW.name = AS3935_NAME;
	as3935HW.status = AS3935_ManualCal(&AS3935_dev, 0, AS3935_LOCATION_OUTDOORS, AS3935_DISTURBER_EN) ? HW_STATUS_OK : HW_STATUS_ERROR;

	as3935HW.values[0].formatter = VALUE_FORMATTER_NONE;
	as3935HW.values[0].name = "Lighting";
	chBSemSignal(&as3935_bsem);
	(void) chMBPostTimeout(&registerHwMail, (msg_t) &as3935HW, TIME_IMMEDIATE); // after this must be access bmp280HW atomically*/
	chThdSleepMilliseconds(1000); //wait for stabilise all values*/
	///////////////////////////////////////////////////////////////

	if(as3935HW.status == HW_STATUS_OK){
		As3935__thread_enableInterrupt(as3935ThreadCfg);
	}

	while (true) {

		chBSemWait(&as3935_bsem);

		if(as3935HW.status == HW_STATUS_ERROR){
			AS3935_init(&AS3935_dev); // try reconfigure
			as3935HW.status = AS3935_ManualCal(&AS3935_dev, 72, AS3935_LOCATION_OUTDOORS, AS3935_DISTURBER_EN) ? HW_STATUS_OK : HW_STATUS_ERROR;

			if(as3935HW.status == HW_STATUS_OK){
				As3935__thread_enableInterrupt(as3935ThreadCfg);
			}
		}

		if(interruptCount > 0){
			As3935__thread_disableInterrupt(as3935ThreadCfg);

			interruptSrc    = AS3935_GetInterruptSrc(&AS3935_dev);
			distance        = AS3935_GetLightningDistKm(&AS3935_dev);
			energy          = AS3935_GetStrikeEnergyRaw(&AS3935_dev);

			if(interruptSrc != 0xff && distance != 0xff && energy != 0xff){
				streamBuff[0] = as3935HW.values[AS3935_INTERRUPT_SRC].value  = (uint32_t)interruptSrc;
				streamBuff[1] = as3935HW.values[AS3935_DISTANCE].value       = (uint32_t)distance;
				streamBuff[2] = as3935HW.values[AS3935_ENERGY].value         = (uint32_t)energy;
				as3935HW.status = HW_STATUS_OK;

				interruptCount = 0;

				poolStreamObject_t* messagePoolObject = (poolStreamObject_t *) chPoolAlloc(&streamMemPool);
				if (messagePoolObject) {
					MSP__createMspFrame(messagePoolObject, (uint8_t)as3935HW.id, 3, (uint32_t*)&streamBuff);
					chMBPostTimeout(&streamMail, (msg_t) messagePoolObject, TIME_IMMEDIATE);
				}

			}else{
				as3935HW.status = HW_STATUS_ERROR;
			}
			As3935__thread_enableInterrupt(as3935ThreadCfg);
		}

		chBSemSignal(&as3935_bsem);

		//load new configuration if needed wait for interval
		thread_t *tp = chMsgWaitTimeout(as3935ThreadCfg->interval * 1000);
		if(tp){
			as3935ThreadCfg->interval = (uint16_t)chMsgGet(tp);
			chMsgRelease(tp, MSG_OK);
		}
	}
}

/**
 *
 */
void As3935__thread_init(As3935__threadConfig_t *cfg) {
	AS3935_dev.i2c = cfg->driver;
	AS3935_dev.addr = cfg->i2cAddr;
	AS3935_dev.checkI2cFunc = cfg->checkI2cFunc;
	as3935ThreadCfg = cfg;
	chBSemObjectInit(&as3935_bsem, false);
}

/**
 *
 */
void As3935__thread_start(void) {
	chThdCreateStatic(AS3935VA, sizeof(AS3935VA), THREAD_PRIORITY_BMP280, As3935Thread,NULL);
}

/**
 *
 */
void As3935__thread_IRQ(void *arg) {
	interruptCount++;
}

/**
 *
 */
void As3935__thread_enableInterrupt(As3935__threadConfig_t *cfg) {
	palEnableLineEvent(cfg->interruptLine, PAL_EVENT_MODE_RISING_EDGE);
	palSetLineMode(cfg->interruptLine, PAL_MODE_INPUT_PULLDOWN);
	palSetLineCallback(cfg->interruptLine, As3935__thread_IRQ, NULL);
}

/**
 *
 */
void As3935__thread_disableInterrupt(As3935__threadConfig_t *cfg) {
	palDisableLineEvent(cfg->interruptLine);
}

/**
 *
 */
bool As3935__thread_autoTune(BaseSequentialStream *outStream) {
	if(chBSemWaitTimeout(&as3935_bsem, 1000) != MSG_OK){
		return false;
	}

	As3935__thread_disableInterrupt(as3935ThreadCfg);

	systime_t startTime;

	for(uint8_t i = 0; i < 15; i++){
		interruptCount = 0;
		AS3935_TuneAntenna(&AS3935_dev, i);

		chThdSleepMilliseconds(10);
		startTime = chVTGetSystemTimeX();
		As3935__thread_enableInterrupt(as3935ThreadCfg);
		chThdSleepSeconds(2);
		As3935__thread_disableInterrupt(as3935ThreadCfg);

		chprintf(outStream, "%6u   %6u  %.3f khz  %6u", i, interruptCount, (float)(interruptCount * 64) / 1000, chTimeDiffX(chVTGetSystemTimeX(), startTime));
		chprintf(outStream, "" NEWLINE_STR);
		chThdSleepSeconds(2);
	}

	AS3935_ManualCal(&AS3935_dev, 0, AS3935_LOCATION_OUTDOORS, AS3935_DISTURBER_EN);

	chBSemSignal(&as3935_bsem);

	return true;
}

#endif