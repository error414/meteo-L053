#include <stdio.h>
#include "ch.h"
#include "hal.h"
#include "appCfg.h"
#include "main.h"
#include "hw.h"
#include "pools.h"
#include "eeprom.h"


#include "scheduleListThread.h"
#include "hwListThread.h"
#include "hc12Thread.h"
#include "powerThread.h"


#ifdef USE_BMP280
#include "bmp280Thread.h"
#endif

#ifdef USE_BH1750
#include "bh1750Thread.h"
#endif

#ifdef USE_ML8511
#include "ml8511Thread.h"
#endif

#ifdef USE_RAIN_FC37
#include "rainThread.h"
#endif

#ifdef USE_WIND_SPEED
#include "windThread.h"
#endif

#ifdef USE_SHELL
#include "shell.h"
#include "shellCmd.h"
#endif

appConfiguration_t appConfiguration;

/*
 * Application entry point.
 */
int main(void) {
	halInit();
	chSysInit();

#ifdef USE_SHELL
	shellInit();
#endif

	shared_pools_init();

	///////////////////////////////////////////////////////////////
	//LOAD CONFIG FROM EEPROM
	///////////////////////////////////////////////////////////////
	EEPROM__init(&EEPROMD1);
	EEPROM__read(&EEPROMD1, CONFIG_BASE_ADDR, (uint8_t*)&appConfiguration, sizeof(appConfiguration));

	if(appConfiguration.version != VERSION){
		appConfiguration.version = VERSION;
		for(uint8_t i = 0; i < SCHEDULE_LIST_SIZE; i++){
			appConfiguration.interval[i] = DEFAULT_TASK_INTERVAL;
		}
	}

	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	//CONFIGURATION
	///////////////////////////////////////////////////////////////
#ifdef USE_BMP280
	Bmp280__threadConfig_t bmp280Cfg = {
			.hwId = BMP280_HW_ID,
			.driver = &I2CD1,
			.enablePinLine = BMP280_ENABLE_PIN,
			.interval = appConfiguration.interval[BMP280_HW_ID] > 0 ? appConfiguration.interval[BMP280_HW_ID] : BMP280_DEFAULT_INTERVAL,
	};
#endif

#ifdef USE_BH1750
	BH1750__threadConfig_t bh1750Cfg = {
			.hwId = BH1750_HW_ID,
			.driver = &I2CD1,
			.enablePinLine = BH1750_ENABLE_PIN,
			.interval = appConfiguration.interval[BH1750_HW_ID] > 0 ? appConfiguration.interval[BH1750_HW_ID] : BH1750_DEFAULT_INTERVAL,
	};
#endif

#ifdef USE_ML8511
	ML8511__threadConfig_t ml8511Cfg = {
			.hwId = ML8511_HW_ID,
			.adcGroup = &adcgrpcfgDevice1,
			.adcDriver = &ADCD1,
			.interval = appConfiguration.interval[ML8511_HW_ID] > 0 ? appConfiguration.interval[ML8511_HW_ID] : ML8511_DEFAULT_INTERVAL,
			.driverEnableLine = ML8511_ENABLE_PIN
	};
#endif

#ifdef USE_RAIN_FC37
	const rain__threadConfig_t rainCfg = {
			.hwId = RAIN_FC37_HW_ID,
			.adcGroup = &adcgrpcfgRain,
			.adcDriver = &ADCD1,
			.interval = appConfiguration.interval[RAIN_FC37_HW_ID] > 0 ? appConfiguration.interval[RAIN_FC37_HW_ID] * 1000 : RAIN_FC37_DEFAULT_INTERVAL * 1000,
	};
#endif

#ifdef USE_WIND_SPEED
	const wind__threadConfig_t windCfg = {
			.hwId = WIND_SPEED_FC37_HW_ID,
			.windLine = WIND_SPEED_INPUT_PIN,
			.interval = appConfiguration.interval[WIND_SPEED_FC37_HW_ID] > 0 ? appConfiguration.interval[WIND_SPEED_FC37_HW_ID] * 1000 : WIND_SPEED_FC37_DEFAULT_INTERVAL * 1000,
	};
#endif

#ifdef USE_SHELL
	const ShellConfig shellCfgUart = {
			(BaseSequentialStream *)&SD1,
			shellCommands
	};
#endif

	const hc12ThreadCfg_t hc12ThreadCfg = {
			.hwId           = HC12_HW_ID,
			.lineSet        = LINE_GPIOC_11,
			.enableLine     = LINE_GPIOC_10,
			.sc_channel     = (BaseChannel*)&LPSD1
	};

	power__threadConfig_t powerCfg = {
			.hwId               = POWER_HW_ID,
			.adcGroup           = &adcgrpcfgPower,
			.adcDriver          = &ADCD1,
			.chrgInfoLine       = LINE_GPIOB_10,
			.stdbyInfoLine      = LINE_GPIOB_11,
			.interval           = appConfiguration.interval[POWER_HW_ID] > 0 ? appConfiguration.interval[POWER_HW_ID] : DEFAULT_TASK_INTERVAL,
	};

	static hc12cfg_t hc12cfg = {
			.baud       = HC12_AT_BAUD9600,
			.channel    = HC12_AT_CHANNEL1,
			.modeFU     = HC12_AT_MODE_FU1,
			.power      = HC12_AT_POWER_6_3mw
	};

	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	//INIT HW
	///////////////////////////////////////////////////////////////
	lpuart_init();
	uart1_init();
	adc_power_init();
	power_GPIO_init();
#ifdef USE_I2C1
	i2c1_init();
#endif
#ifdef USE_ML8511
	adc_device2_init();
#endif
#ifdef USE_RAIN_FC37
	adc_rain_init();
#endif
#ifdef USE_WIND_SPEED
	device_wind_init();
#endif
	///////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////
	//INIT THREADS
	///////////////////////////////////////////////////////////////
	ScheduleList__thread_init();
	HwList__thread_init();
	HC12__thread_init(&hc12ThreadCfg, &hc12cfg);
	Power__thread_init(&powerCfg);
#ifdef USE_BMP280
	Bmp280__thread_init(&bmp280Cfg);
#endif
#ifdef USE_BH1750
	Bh1750__thread_init(&bh1750Cfg);
#endif
#ifdef USE_ML8511
	Ml8511__thread_init(&ml8511Cfg);
#endif
#ifdef USE_RAIN_FC37
	Rain__thread_init(&rainCfg);
#endif
#ifdef USE_WIND_SPEED
	Wind__thread_init(&windCfg);
#endif
	///////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////
	//START THREADS
	///////////////////////////////////////////////////////////////
	ScheduleList__thread_start();   // must be first
	HwList__thread_start();         // must be second
	Power__thread_start();
	HC12__thread_start();
#ifdef USE_BMP280
	Bmp280__thread_start();
#endif
#ifdef USE_BH1750
	Bh1750__thread_start();
#endif
#ifdef USE_ML8511
	Ml8511__thread_start();
#endif
#ifdef USE_RAIN_FC37
	Rain__thread_start();
#endif
#ifdef USE_WIND_SPEED
	Wind__thread_start();
#endif
#ifdef USE_SHELL
	Shell__thread_init(&shellCfgUart);
	///////////////////////////////////////////////////////////////
	while (true) {
		chThdSleepMilliseconds(10000);
		chSysLock();
		chEvtBroadcastI(&shell_terminated);
		chSysUnlock();
	}
#else
	while (true) {
		hThdSleepMilliseconds(10000);
	}
#endif
}

/**
 *
 */
#ifdef USE_I2C1
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
#endif