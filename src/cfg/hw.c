#include "ch.h"
#include "hal.h"
#include "hw.h"

////////////////////////////////////////////////////////////////////////////////////////////
// LUART 1 / data transmit
////////////////////////////////////////////////////////////////////////////////////////////
static const SerialConfig lpsdcfg = {
		9600,
		0,
		USART_CR2_STOP1_BITS | USART_CR2_LINEN,
		0
};

/**
 *
 */
void lpuart_init(void){
	sdStart(&LPSD1, &lpsdcfg);
	palSetLineMode(LINE_GPIOC_4, PAL_MODE_ALTERNATE(2)); //TX
	palSetLineMode(LINE_GPIOC_5, PAL_MODE_ALTERNATE(2)); //RX
}
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
// UART 1 / debug
////////////////////////////////////////////////////////////////////////////////////////////
static const SerialConfig uart1cfg = {
		9600,
		0,
		USART_CR2_STOP1_BITS | USART_CR2_LINEN,
		0
};

/**
 *
 */
void uart1_init(void){
	sdStart(&SD1, &uart1cfg);
	palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(4)); //TX
	palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(4)); //RX
}
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
// I2C2
////////////////////////////////////////////////////////////////////////////////////////////

/*
static const I2CConfig i2cfg2 = {
		STM32_TIMINGR_PRESC(4U) |
		STM32_TIMINGR_SCLDEL(4U) | STM32_TIMINGR_SDADEL(2U) |
		STM32_TIMINGR_SCLH(15U)  | STM32_TIMINGR_SCLL(21U),
		0,
		0
};
*/

static const I2CConfig i2cfg1 = {
		STM32_TIMINGR_PRESC(2U) |
		STM32_TIMINGR_SCLDEL(4U) | STM32_TIMINGR_SDADEL(2U) |
		STM32_TIMINGR_SCLH(15U)  | STM32_TIMINGR_SCLL(21U),
		0,
		0
};



/**
 *
 */
void i2c1_init(void){
	i2cStart(&I2CD1, &i2cfg1);

	palSetLineMode(LINE_GPIOB_8, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); //SCL
	palSetLineMode(LINE_GPIOB_9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); //SDA
}
////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
// ADC / GROUP POWER / SOLAR + BAT
////////////////////////////////////////////////////////////////////////////////////////////
const ADCConversionGroup adcgrpcfgPower = {
		FALSE,
		2,
		FALSE,
		FALSE,
		ADC_CFGR1_RES_12BIT,                            /* CFGRR1 */
		0,                                               /* CFGRR2 */
		ADC_TR(0, 0),                            /* TR */
		ADC_SMPR_SMP_7P5,                                       /* SMPR */
		ADC_CHSELR_CHSEL5 | ADC_CHSELR_CHSEL6           /* CHSELR */
};

/**
 *
 */


void adc_power_init(void){
	palSetLineMode(LINE_GPIOA_5, PAL_MODE_INPUT_ANALOG);
	palSetLineMode(LINE_GPIOA_6, PAL_MODE_INPUT_ANALOG);
}

void power_GPIO_init(void){
	palSetLineMode(LINE_GPIOB_10, PAL_MODE_INPUT_PULLUP); //CHRG
	palSetLineMode(LINE_GPIOB_11, PAL_MODE_INPUT_PULLUP); //STDBY
	palSetLineMode(LINE_GPIOC_1, PAL_STM32_MODE_OUTPUT); //CHRG EN
}
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
// ADC / Device 1
////////////////////////////////////////////////////////////////////////////////////////////
const ADCConversionGroup adcgrpcfgDevice1 = {
		FALSE,
		1,
		FALSE,
		FALSE,
		ADC_CFGR1_RES_12BIT | ADC_CFGR1_CONT,          /* CFGRR1 */
		0,                                               /* CFGRR2 */
		ADC_TR(0, 0),                            /* TR */
		ADC_SMPR_SMP_1P5,                                       /* SMPR */
		ADC_CHSELR_CHSEL3                               /* CHSELR */
};

/**
 *
 */


void adc_device1_init(void){
	palSetLineMode(LINE_GPIOA_3, PAL_MODE_INPUT_ANALOG);
}
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
// ADC / Device 2
////////////////////////////////////////////////////////////////////////////////////////////
const ADCConversionGroup adcgrpcfgDevice2 = {
		FALSE,
		1,
		FALSE,
		FALSE,
		ADC_CFGR1_RES_12BIT | ADC_CFGR1_CONT,          /* CFGRR1 */
		0,                                               /* CFGRR2 */
		ADC_TR(0, 0),                            /* TR */
		ADC_SMPR_SMP_1P5,                                       /* SMPR */
		ADC_CHSELR_CHSEL4                               /* CHSELR */
};

/**
 *
 */


void adc_device2_init(void){
	palSetLineMode(LINE_GPIOA_4, PAL_MODE_INPUT_ANALOG);
}
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
// ADC / Rain
////////////////////////////////////////////////////////////////////////////////////////////
const ADCConversionGroup adcgrpcfgRain = {
		FALSE,
		1,
		FALSE,
		FALSE,
		ADC_CFGR1_RES_12BIT | ADC_CFGR1_CONT,          /* CFGRR1 */
		0,                                               /* CFGRR2 */
		ADC_TR(0, 0),                            /* TR */
		ADC_SMPR_SMP_1P5,                                       /* SMPR */
		ADC_CHSELR_CHSEL7                               /* CHSELR */
};

/**
 *
 */


void adc_rain_init(void){
	palSetLineMode(LINE_GPIOA_7, PAL_MODE_INPUT_ANALOG);
}
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
// WIND
////////////////////////////////////////////////////////////////////////////////////////////
void device_wind_init(void){
	palSetLineMode(LINE_GPIOA_12, PAL_MODE_INPUT_PULLUP);
}
////////////////////////////////////////////////////////////////////////////////////////////

