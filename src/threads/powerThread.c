#include "ch.h"
#include "hal.h"
#include "main.h"
#include "powerThread.h"
#include "hwListThread.h"
#include "pools.h"
#include "msp.h"
#include "hc12Thread.h"
#include "ADS1X15.h"

#define POWER_VOLTAGE_SOLAR 0
#define POWER_VOLTAGE_BATT  1
#define POWER_CHRG_STATUS   2
#define POWER_STDBY_STATUS  3

static ADS1X15_device_t adcDev = {
		.i2cAddress = 0x48,
};

static power__threadConfig_t *powerThreadCfg;
static hw_t powerHW;
static schedule_t   powerSchedule;

static THD_WORKING_AREA(POWERVA, 190);
static THD_FUNCTION(powerThread, arg) {
	(void) arg;
	chRegSetThreadName("Power");

	///////////////////////////////////////////////////////////////
	// REGISTER SCHEDULE
	///////////////////////////////////////////////////////////////
 	powerSchedule.id = powerThreadCfg->hwId;
	powerSchedule.name = POWER_NAME;
	powerSchedule.interval = &powerThreadCfg->interval;
	powerSchedule.tp = chThdGetSelfX();

	(void) chMBPostTimeout(&registerScheduleMail, (msg_t) &powerSchedule, TIME_IMMEDIATE);
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	powerHW.id = powerThreadCfg->hwId;
	powerHW.type = VALUE_TYPE_SENSOR;
	powerHW.name = POWER_NAME;
	powerHW.status = ADS__init(&adcDev) ? HW_STATUS_OK : HW_STATUS_ERROR;

	powerHW.values[POWER_VOLTAGE_SOLAR].formatter = VALUE_FORMATTER_100;
	powerHW.values[POWER_VOLTAGE_SOLAR].name = "Voltage SOLAR";

	powerHW.values[POWER_VOLTAGE_BATT].formatter = VALUE_FORMATTER_100;
	powerHW.values[POWER_VOLTAGE_BATT].name = "Voltage BATT";

	powerHW.values[POWER_CHRG_STATUS].formatter = VALUE_FORMATTER_BOOL;
	powerHW.values[POWER_CHRG_STATUS].name = "Chrg";

	powerHW.values[POWER_STDBY_STATUS].formatter = VALUE_FORMATTER_BOOL;
	powerHW.values[POWER_STDBY_STATUS].name = "Stdby";
	(void) chMBPostTimeout(&registerHwMail, (msg_t) &powerHW, TIME_IMMEDIATE);
	///////////////////////////////////////////////////////////////

	static uint16_t battAdcValue;
	static uint16_t solarAdcValue;

	uint32_t streamBuff[4];
	while (true) {
		if(powerHW.status == HW_STATUS_ERROR){
			ADS__init(&adcDev); // try reconfigure
		}

		if(!HC12__tunrOffRadio()){
			continue;
		}

		HC12__tunrOnRadio();

		if(ADS__readADC_Differential_0_1(&adcDev, &battAdcValue) && ADS__readADC(&adcDev, 2, &solarAdcValue)){
			chSysLock();
			bool chrg = !palReadLine(powerThreadCfg->chrgInfoLine);

			streamBuff[0] = powerHW.values[POWER_VOLTAGE_SOLAR].value   = (uint32_t)((ADS__toVoltage(&adcDev, solarAdcValue) * POWER_SOLAR_VOLTAGE_SCALE) * 100);
			streamBuff[1] = powerHW.values[POWER_VOLTAGE_BATT].value    = (uint32_t)(ADS__toVoltage(&adcDev, battAdcValue) * 100);
			streamBuff[2] = powerHW.values[POWER_CHRG_STATUS].value     = chrg;
			streamBuff[3] = powerHW.values[POWER_STDBY_STATUS].value    = !palReadLine(powerThreadCfg->stdbyInfoLine);
			powerHW.status = HW_STATUS_OK;
			chSysUnlock();

			poolStreamObject_t* messagePoolObject = (poolStreamObject_t *) chPoolAlloc(&streamMemPool);
			if (messagePoolObject) {
				MSP__createMspFrame(messagePoolObject, (uint8_t)powerHW.id, 4, (uint32_t*)&streamBuff);
				chMBPostTimeout(&streamMail, (msg_t) messagePoolObject, TIME_IMMEDIATE);
			}
		}else{
			powerHW.status = HW_STATUS_ERROR;
		}

		//load new configuration if needed
		thread_t *tp = chMsgWaitTimeout(powerThreadCfg->interval * 1000);
		if(tp){
			powerThreadCfg->interval = (uint16_t)chMsgGet(tp);
			chMsgRelease(tp, MSG_OK);
		}
	}
}

/**
 *
 */
void Power__thread_init(power__threadConfig_t *cfg) {
	powerThreadCfg = cfg;
	adcDev.i2cDriver = powerThreadCfg->i2cDriver;
	adcDev.checkI2cFunc = powerThreadCfg->checkI2cFunc;
}

/**
 *
 */
void Power__thread_start(void) {
	chThdCreateStatic(POWERVA, sizeof(POWERVA), THREAD_PRIORITY_ADCPOWER, powerThread,NULL);
}