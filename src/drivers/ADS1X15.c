#include "hal.h"
#include "ADS1X15.h"

////////////////////////////////////////////////////////////////////////////////////////////
// PROTOCOL
////////////////////////////////////////////////////////////////////////////////////////////

#define ADS1015_HAL_I2C_DELAY    200

#define ADS1015_CONVERSION_DELAY    1
#define ADS1115_CONVERSION_DELAY    8


// REGISTERS
#define ADS1X15_REG_CONVERT         0x00
#define ADS1X15_REG_CONFIG          0x01
#define ADS1X15_REG_LOW_THRESHOLD   0x02
#define ADS1X15_REG_HIGH_THRESHOLD  0x03


// CONFIG REGISTER

// BIT 15       Operational Status         // 1 << 15
#define ADS1X15_OS_BUSY             0x0000
#define ADS1X15_OS_NOT_BUSY         0x8000
#define ADS1X15_OS_START_SINGLE     0x8000

// BIT 12-14    read differential
#define ADS1X15_MUX_DIFF_0_1        0x0000
#define ADS1X15_MUX_DIFF_0_3        0x1000
#define ADS1X15_MUX_DIFF_1_3        0x2000
#define ADS1X15_MUX_DIFF_2_3        0x3000
//              read single
#define ADS1X15_READ_0              0x4000  // pin << 12
#define ADS1X15_READ_1              0x5000  // pin = 0..3
#define ADS1X15_READ_2              0x6000
#define ADS1X15_READ_3              0x7000


// BIT 9-11     gain                        // (0..5) << 9
#define ADS1X15_PGA_6_144V          0x0000  // voltage
#define ADS1X15_PGA_4_096V          0x0200  //
#define ADS1X15_PGA_2_048V          0x0400  // default
#define ADS1X15_PGA_1_024V          0x0600
#define ADS1X15_PGA_0_512V          0x0800
#define ADS1X15_PGA_0_256V          0x0A00

// BIT 8        mode                        // 1 << 8
#define ADS1X15_MODE_CONTINUE       0x0000
#define ADS1X15_MODE_SINGLE         0x0100

// BIT 5-7      datarate sample per second  // (0..7) << 5
/*
| datarate | ADS101x | ADS 111x |
|:----:|----:|----:|
| 0 | 128  | 8   |
| 1 | 250  | 16  |
| 2 | 490  | 32  |
| 3 | 920  | 64  |
| 4 | 1600 | 128 |
| 5 | 2400 | 250 |
| 6 | 3300 | 475 |
| 7 | 3300 | 860 |
*/

// BIT 4 comparator modi                    // 1 << 4
#define ADS1X15_COMP_MODE_TRADITIONAL   0x0000
#define ADS1X15_COMP_MODE_WINDOW        0x0010

// BIT 3 ALERT active value                 // 1 << 3
#define ADS1X15_COMP_POL_ACTIV_LOW      0x0000
#define ADS1X15_COMP_POL_ACTIV_HIGH     0x0008

// BIT 2 ALERT latching                     // 1 << 2
#define ADS1X15_COMP_NON_LATCH          0x0000
#define ADS1X15_COMP_LATCH              0x0004

// BIT 0-1 ALERT mode                       // (0..3)
#define ADS1X15_COMP_QUE_1_CONV         0x0000  // trigger alert after 1 convert
#define ADS1X15_COMP_QUE_2_CONV         0x0001  // trigger alert after 2 converts
#define ADS1X15_COMP_QUE_4_CONV         0x0002  // trigger alert after 4 converts
#define ADS1X15_COMP_QUE_NONE           0x0003  // dosable comparator

// _CONFIG masks
//
// | bit  | description |
// |:----:|:----|
// |  0   | # channels |
// |  1   | -  |
// |  2   | resolution |
// |  3   | - |
// |  4   | GAIN supported |
// |  5   | COMPARATOR supported |
// |  6   | - |
// |  7   | - |
#define ADS_CONF_CHAN_1  0x00
#define ADS_CONF_CHAN_4  0x01
#define ADS_CONF_RES_12  0x00
#define ADS_CONF_RES_16  0x04
#define ADS_CONF_NOGAIN  0x00
#define ADS_CONF_GAIN    0x10
#define ADS_CONF_NOCOMP  0x00
#define ADS_CONF_COMP    0x20

////////////////////////////////////////////////////////////////////////////////////////////

static ADS1X15_device_t ADS1X15_device = {
		.compMode       = 0,
		.compPol        = 1,
		.compLatch      = 0,
		.compQueConvert = 3,
};


bool ADS__readADCPriv(ADS1X15_device_t *dev, uint16_t readmode, uint16_t *value);
bool ADS__requestADCPriv(ADS1X15_device_t *dev, uint16_t readmode);

static bool ADS__writeRegister(ADS1X15_device_t *dev, uint8_t reg, uint16_t value);
static bool ADS__readRegister(ADS1X15_device_t *dev, uint8_t reg, uint16_t *data);
////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC
////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 * @param settings
 * @return
 */
bool ADS__init(ADS1X15_device_t *dev){

	dev->compQueConvert = 3;
	dev->compPol = 1;

	ADS__setGain(dev, ADS1X15_PGA_6_144V);
	ADS__setMode(dev, 1);
	ADS__setDataRate(dev, 4);
	return ADS__readADC(dev, 0, (uint16_t*)&dev->bufferRx); // init reading
}

/**
 *
 * @param gain
 */
void ADS__setGain(ADS1X15_device_t *dev, uint8_t gain){
	switch (gain){
		default:  // catch invalid values and go for the safest gain.
		case 0: dev->gain = ADS1X15_PGA_6_144V;  break;
		case 1: dev->gain = ADS1X15_PGA_4_096V;  break;
		case 2: dev->gain = ADS1X15_PGA_2_048V;  break;
		case 4: dev->gain = ADS1X15_PGA_1_024V;  break;
		case 8: dev->gain = ADS1X15_PGA_0_512V;  break;
		case 16: dev->gain = ADS1X15_PGA_0_256V;  break;
	}
}

/**
 *
 * @param mode
 */
void ADS__setMode(ADS1X15_device_t *dev, uint8_t mode){
	switch (mode){
		case 0: dev->mode = ADS1X15_MODE_CONTINUE; break;
		default:
		case 1: dev->mode = ADS1X15_MODE_SINGLE;   break;
	}
}

/**
 *
 * @param dataRate
 */
void ADS__setDataRate(ADS1X15_device_t *dev, uint8_t dataRate){
	dev->datarate = dataRate;
	if (dev->datarate > 7){
		dev->datarate = 4;  // default
	}
	dev->datarate <<= 5;      // convert 0..7 to mask needed.
}

/**
 *
 * @param pin
 * @return
 */
bool ADS__readADC(ADS1X15_device_t *dev, uint8_t pin, uint16_t *value){
	uint16_t mode = ((4 + pin) << 12); // pin to mask
	return ADS__readADCPriv(dev, mode, value);
}

/**
 *
 * @param pin
 */
bool ADS__requestADC(ADS1X15_device_t *dev, uint8_t pin){
	uint16_t mode = ((4 + pin) << 12);   // pin to mask
	return ADS__requestADCPriv(dev, mode);
}

/**
 *
 * @return
 */
bool ADS__getValue(ADS1X15_device_t *dev, uint16_t *value){
	return ADS__readRegister(dev, ADS1X15_REG_CONVERT, value);
}


/**
 *
 * @param val
 * @return
 */
float ADS__toVoltage(ADS1X15_device_t *dev, uint16_t val){
  if (val == 0){
	  return 0;
  }

  float volts = ADS__getMaxVoltage(dev);
  if (volts < 0){
	  return volts;
  }

  return (volts * (float )val) / 32767;
}

/**
 *
 * @return
 */
float ADS__getMaxVoltage(ADS1X15_device_t *dev){
  switch (dev->gain){
    case ADS1X15_PGA_6_144V: return 6.144f;
    case ADS1X15_PGA_4_096V: return 4.096f;
    case ADS1X15_PGA_2_048V: return 2.048f;
    case ADS1X15_PGA_1_024V: return 1.024f;
    case ADS1X15_PGA_0_512V: return 0.512f;
    case ADS1X15_PGA_0_256V: return 0.256f;
  }

  return ADS1X15_INVALID_VOLTAGE;
}

/**
 *
 * @param latch
 */
bool ADS__setComparatorLatch(ADS1X15_device_t *dev, uint8_t latch) {
	dev->compLatch = latch ? 0 : 1;
	return true;
};

/**
 *
 * @param mode
 */
bool ADS__setComparatorQueConvert(ADS1X15_device_t *dev, uint8_t mode) {
	dev->compQueConvert = (mode < 3) ? mode : 3;
	return true;
};

/**
 *
 * @param lo
 */
bool ADS__setComparatorThresholdLow(ADS1X15_device_t *dev, uint16_t lo){
	return ADS__writeRegister(dev, ADS1X15_REG_LOW_THRESHOLD, lo);
};

/**
 *
 * @param hi
 */
bool ADS__setComparatorThresholdHigh(ADS1X15_device_t *dev, uint16_t hi){
	return ADS__writeRegister(dev, ADS1X15_REG_HIGH_THRESHOLD, hi);
};



////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE
////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @param readmode
 * @return
 */
bool ADS__readADCPriv(ADS1X15_device_t *dev, uint16_t readmode, uint16_t *value){
	if(!ADS__requestADCPriv(dev, readmode)){
		return false;
	}

	systime_t start, end;
	start = osalOsGetSystemTimeX();
	end = osalTimeAddX(start, OSAL_MS2I(1000));

	if (dev->mode == ADS1X15_MODE_SINGLE && dev->compQueConvert == 3){
		if(!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end)){
			return false;
		}

		while ( ADS__isBusy(dev) ){
			chThdSleepMilliseconds(10);
		}
	}
	return ADS__getValue(dev, value);
}

/**
 *
 * @param readmode
 */
bool ADS__requestADCPriv(ADS1X15_device_t *dev, uint16_t readmode){
	// write to register is needed in continuous mode as other flags can be changed
	dev->config = ADS1X15_OS_START_SINGLE;  // bit 15     force wake up if needed
	dev->config |= readmode;                         // bit 12-14
	dev->config |= dev->gain;                            // bit 9-11
	dev->config |= dev->mode;                            // bit 8
	dev->config |= dev->datarate;                        // bit 5-7
	if (dev->compMode) dev->config |= ADS1X15_COMP_MODE_WINDOW;         // bit 4      comparator modi
	else            dev->config |= ADS1X15_COMP_MODE_TRADITIONAL;
	if (dev->compPol) dev->config |= ADS1X15_COMP_POL_ACTIV_HIGH;      // bit 3      ALERT active value
	else            dev->config |= ADS1X15_COMP_POL_ACTIV_LOW;
	if (dev->compLatch) dev->config |= ADS1X15_COMP_LATCH;
	else            dev->config |= ADS1X15_COMP_NON_LATCH;           // bit 2      ALERT latching
	dev->config |= dev->compQueConvert;                                  // bit 0..1   ALERT mode
	return ADS__writeRegister(dev, ADS1X15_REG_CONFIG, dev->config);
}
////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 * @return
 */
bool ADS__readADC_Differential_0_1(ADS1X15_device_t *dev, uint16_t *value){
	return ADS__readADCPriv(dev, ADS1X15_MUX_DIFF_0_1, value);
}

////////////////////////////////////////////////////////////////////////////////////////////
// I2C HW ACCESS
////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 * @param address
 * @param reg
 * @param value
 * @return
 */
static bool ADS__writeRegister(ADS1X15_device_t *dev, uint8_t reg, uint16_t value){
	i2cAcquireBus(dev->i2cDriver);

	if(dev->i2cDriver->state != I2C_READY){
		i2cReleaseBus(dev->i2cDriver);
		return false;
	}

	dev->bufferTx[0] = (uint8_t)reg;
	dev->bufferTx[1] = (uint8_t)(value >> 8);
	dev->bufferTx[2] = (uint8_t)(value & 0xFF);

	bool state = i2cMasterTransmitTimeout(dev->i2cDriver, dev->i2cAddress, dev->bufferTx, 3, (uint8_t*)&ADS1X15_device.bufferRx, 0, OSAL_MS2I(500)) == MSG_OK;
	i2cReleaseBus(dev->i2cDriver);

	return  state;
}

/**
 *
 * @param address
 * @param reg
 * @return
 */
static bool ADS__readRegister(ADS1X15_device_t *dev, uint8_t reg, uint16_t *dataRx){
	i2cAcquireBus(dev->i2cDriver);

	if(dev->i2cDriver->state != I2C_READY){
		i2cReleaseBus(dev->i2cDriver);
		return false;
	}

	dev->bufferTx[0] = (uint8_t)reg;
	static uint8_t bufferRx[2];

	bool state = i2cMasterTransmitTimeout(dev->i2cDriver, dev->i2cAddress, (uint8_t*)dev->bufferTx, 1, (uint8_t*)&bufferRx, 2, OSAL_MS2I(ADS1015_HAL_I2C_DELAY)) == MSG_OK;
	i2cReleaseBus(dev->i2cDriver);

	*dataRx = (uint16_t)((bufferRx[0] << 8) | bufferRx[1]);
	return state;
}

/**
 *
 * @return
 */
bool ADS__isBusy(ADS1X15_device_t *dev){
	ADS__readRegister(dev, ADS1X15_REG_CONFIG, &dev->bufferRx);
	if ((dev->bufferRx & ADS1X15_OS_NOT_BUSY) != 0){
		return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////





