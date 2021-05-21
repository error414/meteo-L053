#include "ch.h"
#include "main.h"
#include "as3935Thread.h"
#include "PWFusion_AS3935_I2C.h"
#include "pools.h"
#include "msp.h"
#include "hwListThread.h"


static AS3935_I2C_dev_t AS3935_dev;
static As3935__threadConfig_t *as3935ThreadCfg;

static hw_t         as3935HW;
static schedule_t   as3935Schedule;

static THD_WORKING_AREA(AS3935VA, 260);
static THD_FUNCTION(As3935Thread, arg) {
	(void) arg;
	chRegSetThreadName("AS3935");
	//uint32_t streamBuff[3];

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
	as3935HW.id = as3935ThreadCfg->hwId;
	as3935HW.type = VALUE_TYPE_SENSOR;
	as3935HW.name = AS3935_NAME;
	as3935HW.status = (bool)AS3935_init(&AS3935_dev) && AS3935_ManualCal(&AS3935_dev, 72, AS3935_LOCATION_OUTDOORS, AS3935_DISTURBER_EN); //test HW

	as3935HW.values[0].formatter = VALUE_FORMATTER_NONE;
	as3935HW.values[0].name = "Lighting";

	(void) chMBPostTimeout(&registerHwMail, (msg_t) &as3935HW, TIME_IMMEDIATE); // after this must be access bmp280HW atomically
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	while (true) {
		checkI2CCondition(as3935ThreadCfg->driver);

		if(as3935HW.status == HW_STATUS_ERROR){
			AS3935_init(&AS3935_dev); // try reconfigure
		}

		//load new configuration if needed wait for interval
		thread_t *tp = chMsgWaitTimeout(TIME_IMMEDIATE);
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
	as3935ThreadCfg = cfg;
}

/**
 *
 */
void As3935__thread_start(void) {
	chThdCreateStatic(AS3935VA, sizeof(AS3935VA), THREAD_PRIORITY_BMP280, As3935Thread,NULL);
}