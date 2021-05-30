#ifndef APP_CFG_H
#define APP_CFG_H

#include "hal.h"

#define HC12_HW_ID_OFFSET 0
#define LINE_NO_PIN 0xffffff

#define DEFAULT_TASK_INTERVAL 60 //second

////////////////////////////////////////
///// SENSORS /////////////////////////
////////////////////////////////////////

////////////////////////////////////////
///// BMP280 I2C1 //////////////////////
#define USE_BMP280

#ifdef USE_BMP280
	#define BMP280_HW_ID 1
	#define BMP280_ENABLE_PIN       LINE_GPIOB_7
	#define BMP280_DEFAULT_INTERVAL DEFAULT_TASK_INTERVAL
#endif
////////////////////////////////////////

////////////////////////////////////////
///// BH1750 I2C1 //////////////////////
#define USE_BH1750

#ifdef USE_BH1750
	#define BH1750_HW_ID 2
	#define BH1750_ENABLE_PIN  LINE_NO_PIN
	#define BH1750_DEFAULT_INTERVAL DEFAULT_TASK_INTERVAL
#endif
////////////////////////////////////////

////////////////////////////////////////
///// ML8511 ADC1 DEVICE 2 /////////////
///// ADC1 CHSEL4          /////////////
#define USE_ML8511

#ifdef USE_ML8511
#define ML8511_HW_ID 3
#define ML8511_ENABLE_PIN          LINE_GPIOB_0
#define ML8511_DRIVER_ENABLE_PIN   LINE_NO_PIN
#define ML8511_DEFAULT_INTERVAL DEFAULT_TASK_INTERVAL
#endif
////////////////////////////////////////

////////////////////////////////////////
///// RAIN SENSOR BOOL     /////////////
///// ADC1 CHSEL7          /////////////
//#define USE_RAIN_FC37

#ifdef USE_RAIN_FC37
#define RAIN_FC37_HW_ID 6
#define RAIN_FC37_DEFAULT_INTERVAL 10
#endif
////////////////////////////////////////

////////////////////////////////////////
///// WIND SPEED           /////////////
//#define USE_WIND_SPEED

#ifdef USE_WIND_SPEED
#define WIND_SPEED_FC37_HW_ID 5
#define WIND_SPEED_INPUT_PIN LINE_GPIOA_7
#define WIND_SPEED_FC37_DEFAULT_INTERVAL DEFAULT_TASK_INTERVAL
#endif
////////////////////////////////////////

////////////////////////////////////////
///// POWER               /////////////
#define POWER_HW_ID 4
#define POWER_SOLAR_VOLTAGE_SCALE 1.534
////////////////////////////////////////

////////////////////////////////////////
///// LIGHTING SENSOR AS3935 I2C1 //////
//#define USE_AS3935

#ifdef USE_AS3935
#define USE_AS3935_HW_ID            5
#define  AS3935_INTERRUPT_LINE      LINE_GPIOC_10
#define AS3935_DEFAULT_INTERVAL     DEFAULT_TASK_INTERVAL
#define AS3935_I2C_ADDR             0x00
#define AS3935_CAPACITANCE          72
#endif
////////////////////////////////////////


////////////////////////////////////////
///// HC12                 /////////////
#define HC12_HW_ID 0
////////////////////////////////////////

////////////////////////////////////////
///// SHELL               /////////////
//#define USE_SHELL
#define SHELL_USE_UART_2
////////////////////////////////////////

#if defined(USE_RAIN_FC37) || defined(USE_ML8511)
#define HW_USE_ADC
#endif


/////////////////////////////////////
#endif