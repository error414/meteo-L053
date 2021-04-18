#include "ch.h"
#include "main.h"
#include "bmp280Thread.h"
#include "bmp280.h"
#include "pools.h"
#include "msp.h"
#include "hwListThread.h"
#include "scheduleListThread.h"

#define BMP280_TEMP 0
#define BMP280_HUM 1
#define BMP280_PRESSURE 2

static BMP280_HandleTypedef BMP280_dev;
static const Bmp280__threadConfig_t *bmp280ThreadCfg;

static hw_t         bmp280HW;
static schedule_t   bmp280Schedule;
static uint16_t     interval;

void Bmp280__thread_setInterval(uint16_t interval);

static THD_WORKING_AREA(BMP280VA, 260);
static THD_FUNCTION(Bmp280Thread, arg) {
	(void) arg;
	chRegSetThreadName("Bmp280");
	uint32_t streamBuff[5];

	float temperature;
	float pressure;
	float humidity;

	///////////////////////////////////////////////////////////////
	// REGISTER SCHEDULE
	///////////////////////////////////////////////////////////////
	bmp280Schedule.id = bmp280ThreadCfg->hwId;
	bmp280Schedule.name = BM280_NAME;
	bmp280Schedule.interval = &interval;
	bmp280Schedule.setInterval = &Bmp280__thread_setInterval;

	(void) chMBPostTimeout(&registerScheduleMail, (msg_t) &bmp280Schedule, TIME_IMMEDIATE);
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	bmp280HW.id = bmp280ThreadCfg->hwId;
	bmp280HW.type = VALUE_TYPE_SENSOR;
	bmp280HW.name = BM280_NAME;
	bmp280HW.status = (bool)BMP280_init(&BMP280_dev, &BMP280_dev.params); //test HW

	bmp280HW.values[BMP280_TEMP].formatter = VALUE_FORMATTER_100;
	bmp280HW.values[BMP280_TEMP].name = "Tempeature";

	bmp280HW.values[BMP280_HUM].formatter = VALUE_FORMATTER_NONE;
	bmp280HW.values[BMP280_HUM].name = "Humidity";

	bmp280HW.values[BMP280_PRESSURE].formatter = VALUE_FORMATTER_NONE;
	bmp280HW.values[BMP280_PRESSURE].name = "Pressure";

	(void) chMBPostTimeout(&registerHwMail, (msg_t) &bmp280HW, TIME_IMMEDIATE); // after this must be access bmp280HW atomically
	chThdSleepMilliseconds(1000); //wait for stabilise all values
	///////////////////////////////////////////////////////////////

	while (true) {
		checkI2CCondition(bmp280ThreadCfg->driver);

		if(bmp280HW.status == HW_STATUS_ERROR){
			BMP280_init(&BMP280_dev, &BMP280_dev.params); // try reconfigure
		}

		if(BMP280_read_float(&BMP280_dev, &temperature, &pressure, &humidity)){

			chSysLock();
			streamBuff[0] = bmp280HW.values[BMP280_TEMP].value   = (uint32_t)(temperature * 100.0F);
			streamBuff[1] = bmp280HW.values[BMP280_HUM].value       = (uint32_t)humidity;
			streamBuff[2] = bmp280HW.values[BMP280_PRESSURE].value  = (uint32_t)pressure;
							bmp280HW.status = HW_STATUS_OK;
			chSysUnlock();

			poolStreamObject_t* messagePoolObject = (poolStreamObject_t *) chPoolAlloc(&streamMemPool);
			if (messagePoolObject) {
				MSP__createMspFrame(messagePoolObject, (uint8_t)bmp280HW.id, 3, (uint32_t*)&streamBuff);
				chMBPostTimeout(&streamMail, (msg_t) messagePoolObject, TIME_IMMEDIATE);
			}
		}else{
			bmp280HW.status = HW_STATUS_ERROR;
		}

		chThdSleepMilliseconds(interval);
	}
}

/**
 *
 */
void Bmp280__thread_init(const Bmp280__threadConfig_t *cfg) {
	BMP280_dev.i2c = cfg->driver;
	BMP280_dev.addr = BMP280_I2C_ADDRESS_0;

	BMP280_init_default_params(&BMP280_dev.params);

	interval = cfg->interval;

	bmp280ThreadCfg = cfg;
}

/**
 *
 */
void Bmp280__thread_start(void) {
	chThdCreateStatic(BMP280VA, sizeof(BMP280VA), THREAD_PRIORITY_BMP280, Bmp280Thread,NULL);
}

/**
 *
 */
void Bmp280__thread_setInterval(uint16_t i) {
	interval = i;
}