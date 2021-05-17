#include "ch.h"
#include "hal.h"
#include "main.h"
#include "powerThread.h"
#include "hwListThread.h"
#include "pools.h"
#include "msp.h"
#include "hc12Thread.h"

#define POWER_VOLTAGE_SOLAR 0
#define POWER_VOLTAGE_BATT  1
#define POWER_CHRG_STATUS   2
#define POWER_STDBY_STATUS  3

static power__threadConfig_t *powerThreadCfg;
static adcsample_t adcPowerSamples[ADC_POWER_GRP_CHARGE_NUM_CHANNELS * ADC_POWER_GRP_CHARGEBUF_DEPTH];
static hw_t powerHW;
static schedule_t   powerSchedule;

static THD_WORKING_AREA(POWERVA, 70);
static THD_FUNCTION(powerThread, arg) {
	(void) arg;
	chRegSetThreadName("Power");
	msg_t msg;

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
	powerHW.status = HW_STATUS_UNKNOWN;

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

	uint32_t streamBuff[4];
	while (true) {
		if(!HC12__tunrOffRadio()){
			continue;
		}
		adcAcquireBus(powerThreadCfg->adcDriver);
		adcStart(powerThreadCfg->adcDriver, NULL);
		msg = adcConvert(powerThreadCfg->adcDriver, powerThreadCfg->adcGroup, adcPowerSamples, ADC_POWER_GRP_CHARGEBUF_DEPTH);
		adcStop(powerThreadCfg->adcDriver);
		adcReleaseBus(powerThreadCfg->adcDriver);

		HC12__tunrOnRadio();

		if(msg == MSG_OK){
			chSysLock();
			bool chrg = !palReadLine(powerThreadCfg->chrgInfoLine);

			streamBuff[0] = powerHW.values[POWER_VOLTAGE_SOLAR].value   = (uint32_t)((float)(10.9f / (4096.0f / (double)adcPowerSamples[0])) * 100);
			streamBuff[1] = powerHW.values[POWER_VOLTAGE_BATT].value    = (uint32_t)((float)(6.6f / (4096.0f / (double)adcPowerSamples[1])) * 100);
			streamBuff[2] = powerHW.values[POWER_CHRG_STATUS].value     = chrg;
			streamBuff[3] = powerHW.values[POWER_STDBY_STATUS].value    = !palReadLine(powerThreadCfg->stdbyInfoLine);
			powerHW.status = HW_STATUS_OK;
			chSysUnlock();

			poolStreamObject_t* messagePoolObject = (poolStreamObject_t *) chPoolAlloc(&streamMemPool);
			if (messagePoolObject) {
				MSP__createMspFrame(messagePoolObject, (uint8_t)powerHW.id, 3, (uint32_t*)&streamBuff);
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
	osalDbgCheck(cfg->adcGroup->num_channels == ADC_POWER_GRP_CHARGE_NUM_CHANNELS);
	powerThreadCfg = cfg;
}

/**
 *
 */
void Power__thread_start(void) {
	chThdCreateStatic(POWERVA, sizeof(POWERVA), THREAD_PRIORITY_ADCPOWER, powerThread,NULL);
}