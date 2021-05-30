/***************************************************************************
* File Name: PWFusion_AS3935_I2C.h
*
* Designed for use with with Playing With Fusion AS3935 Lightning Sensor
* Breakout: SEN-39001(R01)
*
*   SEN-39001 (universal applications)
*   ---> http://www.playingwithfusion.com/productview.php?pdid=22
*
* Copyright © 2015 Playing With Fusion, Inc.
* SOFTWARE LICENSE AGREEMENT: This code is released under the MIT License.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
* **************************************************************************
* REVISION HISTORY:
* Author		Date		Comments
* J. Steinlage	2015	Original version
*
* Playing With Fusion, Inc. invests time and resources developing open-source
* code. Please support Playing With Fusion and continued open-source
* development by buying products from Playing With Fusion!
*
* **************************************************************************
* ADDITIONAL NOTES:
* This file contains functions to interface with the AS3935 Franklin
* Lightning Sensor manufactured by AMS. Originally designed for application
* on the Arduino Uno platform.
**************************************************************************/
#ifndef PWF_AS3935_I2C_h
#define PWF_AS3935_I2C_h

#include "hal.h"

#define AS3935_LOCATION_OUTDOORS    1
#define AS3935_LOCATION_INDOORS     2

#define AS3935_DISTURBER_DIS   0
#define AS3935_DISTURBER_EN    1

#define AS3935_INTERRUPT_NAME_NOISE_LEVEL_HIGH      b0001
#define AS3935_INTERRUPT_NAME_DISTURBER_DETECTED    b0100
#define AS3935_INTERRUPT_NAME_LIGHTNING             b1000

typedef struct{
	uint16_t addr;
	I2CDriver* i2c;
	bool (*checkI2cFunc)(I2CDriver *driver);
	uint32_t isrLine;
	thread_t *thread;
} AS3935_I2C_dev_t;


bool AS3935_init(AS3935_I2C_dev_t *dev);
bool AS3935_PowerUp(AS3935_I2C_dev_t *dev);
bool AS3935_PowerDown(AS3935_I2C_dev_t *dev);
bool AS3935_ManualCal(AS3935_I2C_dev_t *dev, uint8_t capacitance, uint8_t location, uint8_t disturber);
bool AS3935_SetIndoors(AS3935_I2C_dev_t *dev);
bool AS3935_SetOutdoors(AS3935_I2C_dev_t *dev);
bool AS3935_DisturberEn(AS3935_I2C_dev_t *dev);
bool AS3935_DisturberDis(AS3935_I2C_dev_t *dev);
bool AS3935_CalRCO(AS3935_I2C_dev_t *dev);
bool AS3935_SetTuningCaps(AS3935_I2C_dev_t *dev, uint8_t cap_val);
bool AS3935_SetIRQ_Output_Source(AS3935_I2C_dev_t *dev, uint8_t irq_select);
bool PWF_AS3935_I2C__AS3935_Reset(AS3935_I2C_dev_t *dev);
uint8_t AS3935_GetInterruptSrc(AS3935_I2C_dev_t *dev);
uint8_t AS3935_GetLightningDistKm(AS3935_I2C_dev_t *dev);
uint32_t AS3935_GetStrikeEnergyRaw(AS3935_I2C_dev_t *dev);
uint8_t AS3935_SetMinStrikes(AS3935_I2C_dev_t *dev, uint8_t min_strk);
bool AS3935_ClearStatistics(AS3935_I2C_dev_t *dev);
uint8_t AS3935_GetNoiseFloorLvl(AS3935_I2C_dev_t *dev);
bool AS3935_SetNoiseFloorLvl(AS3935_I2C_dev_t *dev, uint8_t nf_sel);
uint8_t AS3935_GetWatchdogThreshold(AS3935_I2C_dev_t *dev);
bool AS3935_SetWatchdogThreshold(AS3935_I2C_dev_t *dev, uint8_t wdth);
uint8_t AS3935_GetSpikeRejection(AS3935_I2C_dev_t *dev);
bool AS3935_SetSpikeRejection(AS3935_I2C_dev_t *dev, uint8_t srej);
bool AS3935_TuneAntenna(AS3935_I2C_dev_t *dev, uint8_t tuneCapacitor);

bool AS3935_GetValues(AS3935_I2C_dev_t *dev, uint8_t *interruptSrc, uint8_t *distance, uint32_t *energy);

#endif