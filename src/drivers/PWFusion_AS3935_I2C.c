/***************************************************************************
* File Name: PWFusion_AS3935_I2C.h
*
* Designed for use with with Playing With Fusion AS3935 Lightning Sensor
* Breakout: SEN-39001-R01.
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
* J. Steinlage	2015Jul20	Original version
* J. Steinlage  2016Jul05   Fixed data write issue - now writing 'NewRegData'
*							  based on feedback from J Shuhy and A Jahnke
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

#include "ch.h"
#include "hal.h"
#include "PWFusion_AS3935_I2C.h"

static uint8_t AS3935_I2C__sing_reg_read(AS3935_I2C_dev_t *dev, uint8_t RegAdd);
static bool AS3935_I2C__sing_reg_write(AS3935_I2C_dev_t *dev, uint8_t RegAdd, uint8_t DataMask, uint8_t RegData);
static bool AS3935_I2C__write16(AS3935_I2C_dev_t *dev, uint16_t data);

/**
 *
 * @param dev
 * @return
 */
bool AS3935_init(AS3935_I2C_dev_t *dev){
	dev->thread = chThdGetSelfX();
	return true;
}

/**
 *
 */
bool AS3935_PowerUp(AS3935_I2C_dev_t *dev){
	// power-up sequence based on datasheet, pg 23/27
	// register 0x00, PWD bit: 0 (clears PWD)
	if(!AS3935_I2C__sing_reg_write(dev, 0x00, 0x01, 0x00)){
		return false;
	}

	if(!AS3935_CalRCO(dev)){
		return false;
	}

	if(!AS3935_I2C__sing_reg_write(dev, 0x08, 0x20, 0x20)){
		return false;
	}
	chThdSleepMilliseconds(2);

	if(!AS3935_I2C__sing_reg_write(dev, 0x08, 0x20, 0x00)){
		return false;
	}

	return true;
}

/**
 *
 * @param dev
 * @return
 */
bool AS3935_AS3935_PowerDown(AS3935_I2C_dev_t *dev)
{
	// register 0x00, PWD bit: 0 (sets PWD)
	return AS3935_I2C__sing_reg_write(dev, 0x00, 0x01, 0x01);
}

/**
 *
 */
bool AS3935_CalRCO(AS3935_I2C_dev_t *dev){
	// run ALIB_RCO Direct Command to cal internal RCO
	bool status = AS3935_I2C__write16(dev, (0x3D << 8) | 0x96);
	chThdSleepMilliseconds(2);
	return status;
}

/**
 *
 * @param capacitance
 * @param location
 * @param disturber
 */
bool AS3935_ManualCal(AS3935_I2C_dev_t *dev, uint8_t capacitance, uint8_t location, uint8_t disturber){
	// start by powering up
	if(!AS3935_PowerUp(dev)){
		return false;
	}

	// indoors/outdoors next...
	bool locationStatus = false;
	if(location == AS3935_LOCATION_OUTDOORS){
		locationStatus = AS3935_SetOutdoors(dev);
	}else{
		locationStatus = AS3935_SetIndoors(dev);
	}

	if(!locationStatus){
		return false;
	}

	// disturber cal
	bool disturberStatus = false;
	if(disturber == AS3935_DISTURBER_DIS){
		disturberStatus = AS3935_DisturberDis(dev);
	}else{
		disturberStatus = AS3935_DisturberEn(dev);
	}

	if(!disturberStatus){
		return false;
	}

	if(!AS3935_SetIRQ_Output_Source(dev, 0)){
		return false;
	}

	chThdSleepMilliseconds(500);
	// capacitance first... directly write value here
	return AS3935_SetTuningCaps(dev, capacitance);
}

/**
 *
 * @param dev
 * @return
 */
bool AS3935_SetIndoors(AS3935_I2C_dev_t *dev){
	// AFE settings addres 0x00, bits 5:1 (10010, based on datasheet, pg 19, table 15)
	// this is the default setting at power-up (AS3935 datasheet, table 9)
	return AS3935_I2C__sing_reg_write(dev, 0x00, 0x3E, 0x24);
}

/**
 *
 * @param dev
 * @return
 */
bool AS3935_SetOutdoors(AS3935_I2C_dev_t *dev){
	// AFE settings addres 0x00, bits 5:1 (01110, based on datasheet, pg 19, table 15)
	return AS3935_I2C__sing_reg_write(dev, 0x00, 0x3E, 0x1C);
}

/**
 *
 * @param dev
 * @return
 */
bool AS3935_DisturberEn(AS3935_I2C_dev_t *dev) {
	// register 0x03, PWD bit: 5 (sets MASK_DIST)
	return AS3935_I2C__sing_reg_write(dev, 0x03, 0x20, 0x00);
}

/**
 *
 * @param dev
 * @return
 */
bool AS3935_DisturberDis(AS3935_I2C_dev_t *dev) {
	// register 0x03, PWD bit: 5 (sets MASK_DIST)
	return AS3935_I2C__sing_reg_write(dev, 0x03, 0x20, 0x20);
}

/**
 *
 * @param dev
 * @param cap_val
 * @return
 */
bool AS3935_SetTuningCaps(AS3935_I2C_dev_t *dev, uint8_t tuneCapacitor) {
	return AS3935_I2C__sing_reg_write(dev, 0x08, 0x0F, tuneCapacitor);	// set capacitance bits to maximum
}

/**
 *
 * @param dev
 * @param irq_select
 * @return
 */
bool AS3935_SetIRQ_Output_Source(AS3935_I2C_dev_t *dev, uint8_t irq_select){
	// set interrupt source - what to displlay on IRQ pin
	// reg 0x08, bits 5 (TRCO), 6 (SRCO), 7 (LCO)
	// only one should be set at once, I think
	// 0 = NONE, 1 = TRCO, 2 = SRCO, 3 = LCO

	if(1 == irq_select){
		return AS3935_I2C__sing_reg_write(dev, 0x08, 0xE0, 0x20);			// set only TRCO bit
	}else if(2 == irq_select){
		return AS3935_I2C__sing_reg_write(dev, 0x08, 0xE0, 0x40);			// set only SRCO bit
	}else if(3 == irq_select){
		return AS3935_I2C__sing_reg_write(dev, 0x08, 0xE0, 0x80);			// set only LCO bit
	}else{
		return AS3935_I2C__sing_reg_write(dev, 0x08, 0xE0, 0x00);			// clear IRQ pin display bits
	}
}

/**
 *
 * @param dev
 * @return
 */
bool PWF_AS3935_I2C__AS3935_Reset(AS3935_I2C_dev_t *dev){
	// run PRESET_DEFAULT Direct Command to set all registers in default state
	bool status = AS3935_I2C__write16(dev, (0x3C << 8) | 0x96);
	chThdSleepMilliseconds(2);
	return status;
}

/**
 *
 * @param dev
 * @return
 */
uint8_t AS3935_GetInterruptSrc(AS3935_I2C_dev_t *dev){
	// definition of interrupt data on table 18 of datasheet
	// for this function:
	// 0 = unknown src, 1 = lightning detected, 2 = disturber, 3 = Noise level too high
	chThdSleepMilliseconds(10);						// wait 3ms before reading (min 2ms per pg 22 of datasheet)
	uint8_t int_src = (AS3935_I2C__sing_reg_read(dev, 0x03) & 0x0F);	// read register, get rid of non-interrupt data
	if(0x08 == int_src){
		return 1;					// lightning caused interrupt
	}else if(0x04 == int_src){
		return 2;					// disturber detected
	}else if(0x01 == int_src){
		return 3;					// Noise level too high
	}else{
		return 0;
	}					// interrupt result not expected
}

/**
 *
 * @param dev
 * @return
 */
uint8_t AS3935_GetLightningDistKm(AS3935_I2C_dev_t *dev){
	return (AS3935_I2C__sing_reg_read(dev, 0x07) & 0x3F);	// read register, get rid of non-distance data
}

/**
 *
 * @param dev
 * @return
 */
uint32_t AS3935_GetStrikeEnergyRaw(AS3935_I2C_dev_t *dev){
	uint32_t nrgy_raw = ((AS3935_I2C__sing_reg_read(dev, 0x06) & 0x1F) << 8);	// MMSB, shift 8  bits left, make room for MSB
	nrgy_raw |= AS3935_I2C__sing_reg_read(dev, 0x05);							// read MSB
	nrgy_raw <<= 8;												// shift 8 bits left, make room for LSB
	nrgy_raw |= AS3935_I2C__sing_reg_read(dev, 0x04);							// read LSB, add to others

	return nrgy_raw;
}

/**
 *
 * @param dev
 * @param min_strk
 * @return
 */
uint8_t AS3935_SetMinStrikes(AS3935_I2C_dev_t *dev, uint8_t min_strk){
	// This function sets min strikes to the closest available number, rounding to the floor,
	// where necessary, then returns the physical value that was set. Options are 1, 5, 9 or 16 strikes.
	// see pg 22 of the datasheet for more info (#strikes in 17 min)
	if(5 > min_strk)
	{
		AS3935_I2C__sing_reg_write(dev, 0x02, 0x30, 0x00);
		return 1;
	}
	else if(9 > min_strk)
	{
		AS3935_I2C__sing_reg_write(dev, 0x02, 0x30, 0x10);
		return 5;
	}
	else if(16 > min_strk)
	{
		AS3935_I2C__sing_reg_write(dev, 0x02, 0x30, 0x20);
		return 9;
	}
	else
	{
		AS3935_I2C__sing_reg_write(dev, 0x02, 0x30, 0x30);
		return 16;
	}
}

/**
 *
 * @param dev
 * @return
 */
bool AS3935_ClearStatistics(AS3935_I2C_dev_t *dev){
	// clear is accomplished by toggling CL_STAT bit 'high-low-high' (then set low to move on)
	return AS3935_I2C__sing_reg_write(dev, 0x02, 0x40, 0x40)
			&& AS3935_I2C__sing_reg_write(dev, 0x02, 0x40, 0x00)
			&& AS3935_I2C__sing_reg_write(dev, 0x02, 0x40, 0x40);


}

uint8_t AS3935_GetNoiseFloorLvl(AS3935_I2C_dev_t *dev){
	// NF settings addres 0x01, bits 6:4
	// default setting of 010 at startup (datasheet, table 9)
	uint8_t reg_raw = AS3935_I2C__sing_reg_read(dev, 0x01);		// read register 0x01
	return ((reg_raw & 0x70)>>4);				// should return value from 0-7, see table 16 for info
}

/**
 *
 * @param dev
 * @param nf_sel
 * @return
 */
bool AS3935_SetNoiseFloorLvl(AS3935_I2C_dev_t *dev, uint8_t nf_sel){
	// NF settings addres 0x01, bits 6:4
	// default setting of 010 at startup (datasheet, table 9)
	if(7 >= nf_sel)								// nf_sel within expected range
	{
		return AS3935_I2C__sing_reg_write(dev, 0x01, 0x70, ((nf_sel & 0x07)<<4));
	}
	else
	{											// out of range, set to default (power-up value 010)
		return AS3935_I2C__sing_reg_write(dev, 0x01, 0x70, 0x20);
	}
}

/**
 *
 * @param dev
 * @return
 */
uint8_t AS3935_GetWatchdogThreshold(AS3935_I2C_dev_t *dev){
	// This function is used to read WDTH. It is used to increase robustness to disturbers,
	// though will make detection less efficient (see page 19, Fig 20 of datasheet)
	// WDTH register: add 0x01, bits 3:0
	// default value of 0001
	// values should only be between 0x00 and 0x0F (0 and 7)
	uint8_t reg_raw = AS3935_I2C__sing_reg_read(dev, 0x01);
	return (reg_raw & 0x0F);
}

/**
 *
 * @param dev
 * @param wdth
 * @return
 */
bool AS3935_SetWatchdogThreshold(AS3935_I2C_dev_t *dev, uint8_t wdth){
	// This function is used to modify WDTH. It is used to increase robustness to disturbers,
	// though will make detection less efficient (see page 19, Fig 20 of datasheet)
	// WDTH register: add 0x01, bits 3:0
	// default value of 0001
	// values should only be between 0x00 and 0x0F (0 and 7)
	return AS3935_I2C__sing_reg_write(dev, 0x01, 0x0F, (wdth & 0x0F));
}

/**
 *
 * @param dev
 * @return
 */
uint8_t AS3935_GetSpikeRejection(AS3935_I2C_dev_t *dev){
	// This function is used to read SREJ (spike rejection). Similar to the Watchdog threshold,
	// it is used to make the system more robust to disturbers, though will make general detection
	// less efficient (see page 20-21, especially Fig 21 of datasheet)
	// SREJ register: add 0x02, bits 3:0
	// default value of 0010
	// values should only be between 0x00 and 0x0F (0 and 7)
	uint8_t reg_raw = AS3935_I2C__sing_reg_read(dev, 0x02);
	return (reg_raw & 0x0F);
}

/**
 *
 * @param srej
 * @return
 */
bool AS3935_SetSpikeRejection(AS3935_I2C_dev_t *dev, uint8_t srej){
	// This function is used to modify SREJ (spike rejection). Similar to the Watchdog threshold,
	// it is used to make the system more robust to disturbers, though will make general detection
	// less efficient (see page 20-21, especially Fig 21 of datasheet)
	// WDTH register: add 0x02, bits 3:0
	// default value of 0010
	// values should only be between 0x00 and 0x0F (0 and 7)
	return AS3935_I2C__sing_reg_write(dev, 0x02, 0x0F, (srej & 0x0F));
}

/**
 *
 * @param fdiv
 * @return
 */
bool AS3935_SetLCO_FDIV(AS3935_I2C_dev_t *dev, uint8_t fdiv){
	// This function sets LCO_FDIV register. This is useful in the tuning of the antenna
	// LCO_FDIV register: add 0x03, bits 7:6
	// default value: 00
	// set 0, 1, 2 or 3 for ratios of 16, 32, 64 and 128, respectively.
	// See pg 23, Table 20 for more info.
	return AS3935_I2C__sing_reg_write(dev, 0x03, 0xC0, ((fdiv & 0x03) << 5));
}

/**
 *
 * @param dev
 * @param tuneCapacitor
 * @return bool
 */
bool AS3935_TuneAntenna(AS3935_I2C_dev_t *dev, uint8_t tuneCapacitor){
	if(!AS3935_I2C__sing_reg_write(dev, 0x03, 0xC0, 0x00)){ // AS3935_LCO_FDIV
		return false;
	}

	if(!AS3935_I2C__sing_reg_write(dev, 0x08, 0x80, 0x80)){ // AS3935_DISP_LCO
		return false;
	}

	if(!AS3935_SetTuningCaps(dev, tuneCapacitor)){
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *
 * @param regAdd
 * @return
 */
static uint8_t AS3935_I2C__sing_reg_read(AS3935_I2C_dev_t *dev, uint8_t regAdd){
	i2cAcquireBus(dev->i2c);

	if(!dev->checkI2cFunc(dev->i2c)){
		i2cReleaseBus(dev->i2c);
		return false;
	}

	static uint8_t dataTx; //static for DMA
	static uint8_t dataRx;

	dataTx = regAdd;

	if (i2cMasterTransmitTimeout(dev->i2c, dev->addr, &dataTx, 1, (uint8_t*)&dataRx, 1, OSAL_MS2I(800)) == MSG_OK) {
		i2cReleaseBus(dev->i2c);
		return dataRx;
	} else{
		i2cReleaseBus(dev->i2c);
		return 0;
	}
}

/**
 *
 * @param regAdd
 * @param dataMask
 * @param regData
 * @return
 */
static bool AS3935_I2C__sing_reg_write(AS3935_I2C_dev_t *dev, uint8_t regAdd, uint8_t dataMask, uint8_t regData){
	// start by reading original register data (only modifying what we need to)
	uint8_t origRegData = AS3935_I2C__sing_reg_read(dev, regAdd);

	// calculate new register data... 'delete' old targeted data, replace with new data
	// note: 'DataMask' must be bits targeted for replacement
	// add'l note: this function does NOT shift values into the proper place... they need to be there already
	uint8_t newRegData = ((origRegData & ~dataMask) | (regData & dataMask));

	i2cAcquireBus(dev->i2c);
	if(!dev->checkI2cFunc(dev->i2c)){
		i2cReleaseBus(dev->i2c);
		return false;
	}

	static uint8_t dataTx[2]; //static for DMA
	static uint8_t dataRx;

	dataTx[0] = regAdd;
	dataTx[1] = newRegData;

	// finally, write the data to the register
	if (i2cMasterTransmitTimeout(dev->i2c, dev->addr, (uint8_t*)&dataTx, 2, (uint8_t*)&dataRx, 0, OSAL_MS2I(800)) == MSG_OK) {
		i2cReleaseBus(dev->i2c);
		return true;
	} else{
		i2cReleaseBus(dev->i2c);
		return false;
	}
}

/**
 *
 * @param dev
 * @param data
 * @return
 */
static bool AS3935_I2C__write16(AS3935_I2C_dev_t *dev, uint16_t data){
	i2cAcquireBus(dev->i2c);

	if(!dev->checkI2cFunc(dev->i2c)){
		i2cReleaseBus(dev->i2c);
		return false;
	}

	static uint16_t dataTx; //static for DMA
	static uint8_t dataRx;

	dataTx = data;

	// finally, write the data to the register
	if (i2cMasterTransmitTimeout(dev->i2c, dev->addr, (uint8_t*)&dataTx, 1, (uint8_t*)&dataRx, 0, OSAL_MS2I(800)) == MSG_OK) {
		i2cReleaseBus(dev->i2c);
		return true;
	} else{
		i2cReleaseBus(dev->i2c);
		return false;
	}
}