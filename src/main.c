#include <stdio.h>
#include "ch.h"
#include "hal.h"
#include "main.h"
#include "hw.h"
#include "shell.h"
#include "pools.h"
#include "eeprom.h"
#include "shellCmd.h"

#include "ml8511.h"

#include "scheduleListThread.h"
#include "hwListThread.h"
#include "bmp280Thread.h"
#include "bh1750Thread.h"
#include "ml8511Thread.h"
#include "hc12Thread.h"
#include "powerThread.h"
#include "windThread.h"
#include "rainThread.h"

appConfiguration_t appConfiguration;

/*
 * Application entry point.
 */
int main(void) {
	halInit();
	chSysInit();
	shellInit();

	shared_pools_init();

	///////////////////////////////////////////////////////////////
	//LOAD CONFIG FROM EEPROM
	///////////////////////////////////////////////////////////////
	EEPROM__init(&EEPROMD1);
	EEPROM__read(&EEPROMD1, CONFIG_BASE_ADDR, (uint8_t*)&appConfiguration, sizeof(appConfiguration));

	if(appConfiguration.version != VERSION){
		appConfiguration.version = VERSION;
		for(uint8_t i = 0; i < SCHEDULE_LIST_SIZE; i++){
			appConfiguration.interval[i] = DEFAULT_INTERVAL;
		}
	}

	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	//CONFIGURATION
	///////////////////////////////////////////////////////////////
	const ShellConfig shellCfgUart1 = {
			(BaseSequentialStream *)&SD1,
			shellCommands
	};

	const hc12ThreadCfg_t hc12ThreadCfg = {
			.hwId = 0,
			.lineSet = LINE_GPIOC_11,
			.sc_channel = (BaseChannel*)&LPSD1
	};

	const Bmp280__threadConfig_t bmp280Cfg = {
			.hwId = BMP280_HW_ID,
			.driver = &I2CD1,
			.enablePinLine = LINE_GPIOB_5,
			.interval = appConfiguration.interval[BMP280_HW_ID] > 0 ? appConfiguration.interval[BMP280_HW_ID] * 1000 : DEFAULT_INTERVAL * 1000,
	};

	const BH1750__threadConfig_t bh1750Cfg = {
			.hwId = BH1750_HW_ID,
			.driver = &I2CD1,
			.enablePinLine = LINE_GPIOB_5,
			.interval = appConfiguration.interval[BH1750_HW_ID] > 0 ? appConfiguration.interval[BH1750_HW_ID] * 1000 : DEFAULT_INTERVAL * 1000,
	};

	const ML8511__threadConfig_t ml8511Cfg = {
			.hwId = ML8511_HW_ID,
			.adcGroup = &adcgrpcfgDevice2,
			.adcDriver = &ADCD1,
			.interval = appConfiguration.interval[ML8511_HW_ID] > 0 ? appConfiguration.interval[ML8511_HW_ID] * 1000 : DEFAULT_INTERVAL * 1000,
			.driverEnableLine = ML8511_NO_ENABLE_LINE
	};

	const power__threadConfig_t powerCfg = {
			.hwId = POWER_HW_ID,
			.adcGroup = &adcgrpcfgPower,
			.adcDriver = &ADCD1,
			.chargeEnLine   = LINE_GPIOC_1,
			.chrgInfoLine   = LINE_GPIOB_10,
			.stdbyInfoLine  = LINE_GPIOB_11,
			.interval = appConfiguration.interval[POWER_HW_ID] > 0 ? appConfiguration.interval[POWER_HW_ID] * 1000 : DEFAULT_INTERVAL * 1000,
	};

	static hc12cfg_t hc12cfg = {
			.baud       = HC12_AT_BAUD9600,
			.channel    = HC12_AT_CHANNEL1,
			.modeFU     = HC12_AT_MODE_FU1,
			.power      = HC12_AT_POWER_6_3mw
	};

	const wind__threadConfig_t windCfg = {
			.hwId = WIND_HW_ID,
			.windLine = LINE_GPIOA_7,
			.interval = appConfiguration.interval[WIND_HW_ID] > 0 ? appConfiguration.interval[WIND_HW_ID] * 1000 : DEFAULT_INTERVAL * 1000,
	};

	const rain__threadConfig_t rainCfg = {
			.hwId = POWER_HW_ID,
			.adcGroup = &adcgrpcfgRain,
			.adcDriver = &ADCD1,
			.interval = appConfiguration.interval[RAIN_HW_ID] > 0 ? appConfiguration.interval[RAIN_HW_ID] * 1000 : DEFAULT_INTERVAL * 1000,
	};
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	//INIT HW
	///////////////////////////////////////////////////////////////
	lpuart_init();
	uart1_init();
	i2c1_init();
	adc_power_init();
	adc_device2_init();
	device_wind_init();
	adc_rain_init();
	///////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////
	//INIT THREADS
	///////////////////////////////////////////////////////////////
	ScheduleList__thread_init();
	HwList__thread_init();
	HC12__thread_init(&hc12ThreadCfg, &hc12cfg);
	Bmp280__thread_init(&bmp280Cfg);
	Bh1750__thread_init(&bh1750Cfg);
	Ml8511__thread_init(&ml8511Cfg);
	Power__thread_init(&powerCfg);
	Wind__thread_init(&windCfg);
	Rain__thread_init(&rainCfg);
	///////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////
	//START THREADS
	///////////////////////////////////////////////////////////////
	ScheduleList__thread_start();
	HwList__thread_start();
	Bmp280__thread_start();
	Bh1750__thread_start();
	Ml8511__thread_start();
	Power__thread_start();
	Wind__thread_start();
	HC12__thread_start();
	Rain__thread_start();
	Shell__thread_init(&shellCfgUart1);
	///////////////////////////////////////////////////////////////

	while (true) {
		chThdSleepMilliseconds(10000);
		chSysLock();
		chEvtBroadcastI(&shell_terminated);
		chSysUnlock();
	}
}

/**
 *
 */
void checkI2CCondition(I2CDriver *driver){
	if(driver->state == I2C_LOCKED){
		i2cAcquireBus(driver);

		i2cStop(driver);
		if(driver == &I2CD1){
			i2c1_init();
		}

		i2cReleaseBus(driver);
	}
}