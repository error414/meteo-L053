#ifndef HW_H
#define HW_H

extern const ADCConversionGroup adcgrpcfgPower;
extern const ADCConversionGroup adcgrpcfgDevice1;
extern const ADCConversionGroup adcgrpcfgDevice2;
extern const ADCConversionGroup adcgrpcfgDeviceWind;

void lpuart_init(void);
void uart1_init(void);
void i2c1_init(void);
void adc_power_init(void);
void adc_device1_init(void);
void adc_device2_init(void);
#endif