#ifndef ADC_H
#define ADC_H

#include "stdint.h"
#include "stdbool.h"

#ifdef ADS1015_HAL_I2C_DELAY
#define ADS1015_HAL_I2C_DELAY    2
#endif


#define ADS1X15_OK                  0
#define ADS1X15_INVALID_VOLTAGE     -100
#define ADS1X15_INVALID_GAIN        0xFF
#define ADS1X15_INVALID_MODE        0xFE


typedef struct {
	uint16_t config;
	uint16_t gain;
	uint16_t mode;
	uint16_t datarate;
	uint8_t  compMode;
	uint8_t  compPol;
	uint8_t  compLatch;
	uint8_t  compQueConvert;
	uint16_t bufferRx;
	uint8_t bufferTx[3];
	I2CDriver *i2cDriver;
	uint8_t i2cAddress;
	bool (*checkI2cFunc)(I2CDriver *driver);
} ADS1X15_device_t;


bool ADS__init(ADS1X15_device_t *dev);
void ADS__setGain(ADS1X15_device_t *dev, uint8_t gain);
void ADS__setMode(ADS1X15_device_t *dev, uint8_t mode);
void ADS__setDataRate(ADS1X15_device_t *dev, uint8_t dataRate);

bool ADS__setComparatorThresholdHigh(ADS1X15_device_t *dev, uint16_t hi);
bool ADS__setComparatorThresholdLow(ADS1X15_device_t *dev, uint16_t lo);
bool ADS__setComparatorQueConvert(ADS1X15_device_t *dev, uint8_t mode);
bool ADS__setComparatorLatch(ADS1X15_device_t *dev, uint8_t latch);

bool ADS__readADC(ADS1X15_device_t *dev, uint8_t pin, uint16_t *value);
bool ADS__readADC_Differential_0_1(ADS1X15_device_t *dev, uint16_t *value);

bool ADS__requestADC(ADS1X15_device_t *dev, uint8_t pin);
bool ADS__getValue(ADS1X15_device_t *dev, uint16_t *value);
bool ADS__isBusy(ADS1X15_device_t *dev);

float ADS__toVoltage(ADS1X15_device_t *dev, uint16_t val);
float ADS__getMaxVoltage(ADS1X15_device_t *dev);



#endif

