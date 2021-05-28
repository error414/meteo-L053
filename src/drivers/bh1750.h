/**
 * @author  Alexander Hoffman
 * @email   alxhoff@gmail.com
 * @website http://alexhoffman.info
 * @license GNU GPL v3
 * @brief   STM32 HAL library for BH1750 devices
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

#ifndef BH1750_H_
#define BH1750_H_

//Device Address
#define BH1750_NO_GROUND_ADDR     (0x5C + 0)
#define BH1750_GROUND_ADDR        (0x23 + 0)

//instructions
//datasheet ref http://cpre.kmutnb.ac.th/esl/learning/bh1750-light-sensor/bh1750fvi-e_datasheet.pdf
#define CMD_POWER_DOWN          0x00
#define CMD_POWER_ON            0x01
#define CMD_RESET               0x07
#define CMD_H_RES_MODE          0x10
#define CMD_H_RES_MODE2         0x11
#define CMD_L_RES_MODE          0x13
#define CMD_ONE_H_RES_MODE      0x20
#define CMD_ONE_H_RES_MODE2     0x21
#define CMD_ONE_L_RES_MODE      0x23
#define CMD_CNG_TIME_HIGH       0x30    // 3 LSB set time
#define CMD_CNG_TIME_LOW        0x60    // 5 LSB set time

typedef struct {
	I2CDriver* i2c_handle;
	uint8_t address;
	bool (*checkI2cFunc)(I2CDriver *driver);

	uint16_t value;
	uint8_t bufferTx;
	uint8_t bufferRx[2];
	uint8_t mode;
} BH1750_HandleTypedef;

void BH1750_init_default_params(BH1750_HandleTypedef* device, I2CDriver* i2c_handle, bool addr_grounded);
msg_t BH1750_init(BH1750_HandleTypedef* dev);
bool BH1750_get_lumen(BH1750_HandleTypedef* dev);

#endif /* BH1750_H_ */