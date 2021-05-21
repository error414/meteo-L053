/**
* @author  Alexander Hoffman
* @email   alxhoff@gmail.com
* @website http://alexhoffman.info
* @license GNU GPL v3
* @brief	STM32 HAL library for BH1750 devices
*
@verbatim
----------------------------------------------------------------------
Copyright (C) Alexander Hoffman, 2017
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		any later version.
This program is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
----------------------------------------------------------------------
@endverbatim
*/

#include <stdlib.h>
#include "hal.h"
#include "bh1750.h"

msg_t BH1750_read_dev(BH1750_HandleTypedef* dev);

/**
 *
 * @param dev
 * @param cmd
 * @return
 */
msg_t BH1750_send_command(BH1750_HandleTypedef* dev, uint8_t cmd){
	i2cAcquireBus(dev->i2c_handle);

	if(dev->i2c_handle->state != I2C_READY){
		i2cReleaseBus(dev->i2c_handle);
		return MSG_TIMEOUT;
	}

	dev->bufferTx = cmd;
	msg_t msg = i2cMasterTransmitTimeout(
			dev->i2c_handle,	//I2C Handle
			dev->address,		//I2C addr of dev
			&dev->bufferTx,		//CMD to be executed
			1,					//8bit addr
			&dev->bufferRx[0],
			0,
			OSAL_MS2I(500)					//Wait time
	);

	i2cReleaseBus(dev->i2c_handle);
	return msg;
}

/**
 *
 * @param dev
 * @return
 */

msg_t BH1750_read_dev(BH1750_HandleTypedef* dev) {
	i2cAcquireBus(dev->i2c_handle);

	if(dev->i2c_handle->state != I2C_READY){
		i2cReleaseBus(dev->i2c_handle);
		return MSG_TIMEOUT;
	}

	msg_t msg = i2cMasterReceiveTimeout(dev->i2c_handle,
	                                    dev->address,
	                                    (uint8_t*)&dev->bufferRx,
	                                    2,
	                                    OSAL_MS2I(800)
	);

	i2cReleaseBus(dev->i2c_handle);

	return msg;
}

/**
 *
 * @param device
 * @param i2c_handle
 * @param addr_grounded
 */
void BH1750_init_default_params(BH1750_HandleTypedef* device, I2CDriver* i2c_handle, bool addr_grounded){

	if(addr_grounded){
		device->address = BH1750_GROUND_ADDR;
	}else{
		device->address = BH1750_NO_GROUND_ADDR;
	}

	device->i2c_handle = i2c_handle;
}

/**
 *
 * @param dev
 * @return
 */
msg_t BH1750_init(BH1750_HandleTypedef* dev) {
	BH1750_send_command(dev, CMD_POWER_ON);
	BH1750_send_command(dev, CMD_RESET);
	BH1750_send_command(dev, CMD_H_RES_MODE);

	chThdSleepMilliseconds(300); // wait for first conversion
	return MSG_OK;
}

/**
 *
 * @param dev
 * @return
 */
msg_t BH1750_convert(BH1750_HandleTypedef* dev){
	dev->value = dev->bufferRx[0];
	dev->value = (dev->value << 8) | dev->bufferRx[1];
	dev->value = (uint16_t)((float)dev->value / 1.2f);
	return MSG_OK;
}

/**
 *
 * @param dev
 * @return
 */
bool BH1750_get_lumen(BH1750_HandleTypedef* dev){
	msg_t msg = BH1750_read_dev(dev);
	if(msg == MSG_OK){
		BH1750_convert(dev);
		return true;
	}

	return false;

}