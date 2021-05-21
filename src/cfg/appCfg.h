#ifndef APP_CFG_H
#define APP_CFG_H

#include "hal.h"

#define HC12_HW_ID_OFFSET 0

#define DEFAULT_TASK_INTERVAL 30 //second

////////////////////////////////////////
///// SENSORS /////////////////////////
////////////////////////////////////////

////////////////////////////////////////
///// BMP280 I2C1 //////////////////////
#define USE_BMP280

#ifdef USE_BMP280
	#define BMP280_HW_ID 1
	#define BMP280_ENABLE_PIN       -1
	#define BMP280_DEFAULT_INTERVAL 10
#endif
////////////////////////////////////////

////////////////////////////////////////
///// BH1750 I2C1 //////////////////////
#define USE_BH1750

#ifdef USE_BH1750
	#define BH1750_HW_ID 2
	#define  BH1750_ENABLE_PIN       -1
	#define BH1750_DEFAULT_INTERVAL 10
#endif
////////////////////////////////////////

////////////////////////////////////////
///// ML8511 ADC1 DEVICE 2 /////////////
///// ADC1 CHSEL4          /////////////
#define USE_ML8511

#ifdef USE_ML8511
#define ML8511_HW_ID 3
#define  ML8511_ENABLE_PIN   LINE_GPIOB_0
#define ML8511_DEFAULT_INTERVAL 10
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
#define WIND_SPEED_FC37_DEFAULT_INTERVAL 10
#endif
////////////////////////////////////////

////////////////////////////////////////
///// POWER               /////////////
#define POWER_HW_ID 4
#define POWER_SOLAR_VOLTAGE_SCALE 4
#define POWER_BATT_VOLTAGE_SCALE 4
////////////////////////////////////////

////////////////////////////////////////
///// LIGHTING SENSOR AS3935 I2C1 //////
//#define USE_AS3935

#ifdef USE_AS3935
#define USE_AS3935_HW_ID            5
#define  AS3935_INTERRUPT_LINE      -1
#define AS3935_DEFAULT_INTERVAL     10
#define AS3935_I2C_ADDR             0x03
#define AS3935_CAPACITANCE          72
#endif
////////////////////////////////////////


////////////////////////////////////////
///// HC12                 /////////////
#define HC12_HW_ID 0
////////////////////////////////////////

////////////////////////////////////////
///// SHELL               /////////////
#define USE_SHELL

#define SHELL_USE_UART_2
////////////////////////////////////////


/////////////////////////////////////
#endif