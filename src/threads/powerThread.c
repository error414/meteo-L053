#include "ch.h"
#include "hal.h"
#include "main.h"
#include "powerThread.h"
#include "hwListThread.h"
#include "scheduleListThread.h"

#define POWER_VOLTAGE_SOLAR 0
#define POWER_VOLTAGE_BATT  1
#define POWER_CHRG_STATUS   2
#define POWER_STDBY_STATUS  3

static const power__threadConfig_t *powerThreadCfg;
static adcsample_t adcPowerSamples[ADC_POWER_GRP_CHARGE_NUM_CHANNELS * ADC_POWER_GRP_CHARGEBUF_DEPTH];
static hw_t powerHW;
static schedule_t   powerSchedule;
static uint16_t     interval;

void Power__thread_setInterval(uint16_t i);

/*static void Power__chrg_it(void *arg);
static void Power__stdby_it(void *arg);*/

static THD_WORKING_AREA(POWERVA, 70);
static THD_FUNCTION(powerThread, arg) {
	(void) arg;
	chRegSetThreadName("Power");
	msg_t msg;

	palSetLine(powerThreadCfg->chargeEnLine); // always enable charging

	///////////////////////////////////////////////////////////////
	// REGISTER SCHEDULE
	///////////////////////////////////////////////////////////////
	powerSchedule.id = powerThreadCfg->hwId;
	powerSchedule.name = POWER_NAME;
	powerSchedule.interval = &interval;
	powerSchedule.setInterval = &Power__thread_setInterval;

	(void) chMBPostTimeout(&registerScheduleMail, (msg_t) &powerSchedule, TIME_IMMEDIATE);
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	powerHW.id = powerThreadCfg->hwId;
	powerHW.type = VALUE_TYPE_SENSOR;
	powerHW.name = POWER_NAME;
	powerHW.status = HW_STATUS_UNKNOWN;

	powerHW.values[POWER_VOLTAGE_SOLAR].formatter = VALUE_FORMATTER_100;
	powerHW.values[POWER_VOLTAGE_SOLAR].name = "Voltage SOLAR";

	powerHW.values[POWER_VOLTAGE_BATT].formatter = VALUE_FORMATTER_100;
	powerHW.values[POWER_VOLTAGE_BATT].name = "Voltage BATT";

	powerHW.values[POWER_CHRG_STATUS].formatter = VALUE_FORMATTER_NONE;
	powerHW.values[POWER_CHRG_STATUS].name = "Chrg";

	powerHW.values[POWER_STDBY_STATUS].formatter = VALUE_FORMATTER_NONE;
	powerHW.values[POWER_STDBY_STATUS].name = "Stdby";
	(void) chMBPostTimeout(&registerHwMail, (msg_t) &powerHW, TIME_IMMEDIATE);
	///////////////////////////////////////////////////////////////

	while (true) {

		adcAcquireBus(powerThreadCfg->adcDriver);
		adcStart(powerThreadCfg->adcDriver, NULL);
		msg = adcConvert(powerThreadCfg->adcDriver, powerThreadCfg->adcGroup, adcPowerSamples, ADC_POWER_GRP_CHARGEBUF_DEPTH);
		adcStop(powerThreadCfg->adcDriver);
		adcReleaseBus(powerThreadCfg->adcDriver);

		if(msg == MSG_OK){
			chSysLock();
			powerHW.values[POWER_VOLTAGE_SOLAR].value   = (uint32_t)((float)(8.152f / (4096.0f / (double)adcPowerSamples[0])) * 100);
			powerHW.values[POWER_VOLTAGE_BATT].value    = (uint32_t)((float)(6.6f / (4096.0f / (double)adcPowerSamples[1])) * 100);
			powerHW.values[POWER_CHRG_STATUS].value     = !palReadLine(powerThreadCfg->chrgInfoLine);
			powerHW.values[POWER_STDBY_STATUS].value    = !palReadLine(powerThreadCfg->stdbyInfoLine);
			powerHW.status = HW_STATUS_OK;
			chSysUnlock();
		}else{
			powerHW.status = HW_STATUS_ERROR;
		}

		chThdSleepMilliseconds(interval);
	}
}

/**
 *
 */
void Power__thread_init(const power__threadConfig_t *cfg) {
	osalDbgCheck(cfg->adcGroup->num_channels == ADC_POWER_GRP_CHARGE_NUM_CHANNELS);
	powerThreadCfg = cfg;
	interval = powerThreadCfg->interval;
}

/**
 *
 */
void Power__thread_start(void) {
	chThdCreateStatic(POWERVA, sizeof(POWERVA), THREAD_PRIORITY_ADCPOWER, powerThread,NULL);
}

/**
 *
 */
void Power__thread_setInterval(uint16_t i) {
	interval = i;
}

/**
 *
 * @param arg
 *//*
static void Power__chrg_it(void *arg) {

}*/

/**
 *
 * @param arg
 */
/*
static void Power__stdby_it(void *arg) {

}*/
