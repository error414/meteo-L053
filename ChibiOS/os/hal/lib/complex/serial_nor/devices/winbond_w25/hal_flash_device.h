/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_flash_device.c
 * @brief   Winbond W25 serial flash driver code.
 *
 * @addtogroup WINBOND_W25
 * @{
 */


#ifndef HAL_FLASH_DEVICE_H
#define HAL_FLASH_DEVICE_H

#include "hal_flash_device.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Device capabilities
 * @{
 */
#define SNOR_DEVICE_SUPPORTS_XIP            FALSE
/** @} */

/**
 * @name    Device identification
 * @{
 */
#define W25Q_SUPPORTED_MANUFACTURE_IDS      {0xEF}
#define W25Q_SUPPORTED_MEMORY_TYPE_IDS      {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x18, 0x19, 0x1a}
/** @} */

/**
 * @name    Command codes
 * @{
 */


#define W25Q_CMD_READ_ID                        0x9f
#define W25Q_CMD_ENABLE_RESET                   0x66
#define W25Q_CMD_RESET                          0x99
#define W25Q_CMD_WRITE_ENABLE                   0x06
#define W25Q_CMD_WRITE_DISABLE                  0x04
#define W25Q_CMD_BULK_ERASE                     0x60
#define W25Q_CMD_READ_FLAG_STATUS_REGISTER_1    0x05
#define W25Q_CMD_READ_FLAG_STATUS_REGISTER_2    0x35
#define W25Q_CMD_READ_FLAG_STATUS_REGISTER_3    0x15
#define W25Q_CMD_RESET_ENABLE                   0x66
#define W25Q_CMD_RESET_MEMORY                   0x99
#define W25Q_CMD_WRITE_V_CONF_REGISTER          0xC0
#define W25Q_CMD_SECTOR_ERASE                   0x20
#define W25Q_CMD_BLOCK_32K_ERASE                0x52
#define W25Q_CMD_BLOCK_64K_ERASE                0xD8

#define W25Q_CMD_READ                           0x03 // 1 / 1
#define W25Q_CMD_FAST_READ_DUAL_IO              0x3B // 2 / 2
#define W25Q_CMD_FAST_READ_QUAD_IO              0xEB // 4 / 4
#define W25Q_CMD_PAGE_PROGRAM                   0x02




/** @} */

/**
 * @name    Flags status register bits
 * @{
 */
#define W25Q_FLAGS_PROGRAM_BUSY                     0x01U
#define W25Q_FLAGS_ERASE_SUSPEND                    0x40U
#define W25Q_FLAGS_ERASE_ERROR                      0x20U
#define W25Q_FLAGS_PROGRAM_ERROR                    0x10U
#define W25Q_FLAGS_VPP_ERROR                        0x08U
#define W25Q_FLAGS_PROGRAM_SUSPEND                  0x04U
#define W25Q_FLAGS_PROTECTION_ERROR                 0x02U
#define W25Q_FLAGS_RESERVED                         0x01U
#define W25Q_FLAGS_ALL_ERRORS                   (W25Q_FLAGS_ERASE_ERROR |   \
                                                 W25Q_FLAGS_PROGRAM_ERROR | \
                                                 W25Q_FLAGS_VPP_ERROR |     \
                                                 W25Q_FLAGS_PROTECTION_ERROR)
/** @} */

/**
 * @name    Bus interface modes.
 * @{
 */
#define W25Q_BUS_MODE_WSPI1L                1U
#define W25Q_BUS_MODE_WSPI2L                2U
#define W25Q_BUS_MODE_WSPI4L                4U
#define W25Q_BUS_MODE_QPI                   5U
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Switch WSPI bus width on initialization.
 * @details A bus width initialization is performed by writing the
 *          Enhanced Volatile Configuration Register. If the flash
 *          device is configured using the Non Volatile Configuration
 *          Register then this option is not required.
 * @note    This option is only valid in WSPI bus mode.
 */
#define W25Q_SWITCH_WIDTH                   TRUE //must be true

/**
 * @brief   Device bus mode to be used.
 * #note    if @p W25Q_SWITCH_WIDTH is @p FALSE then this is the bus mode
 *          that the device is expected to be using.
 * #note    if @p W25Q_SWITCH_WIDTH is @p TRUE then this is the bus mode
 *          that the device will be switched in.
 * @note    This option is only valid in WSPI bus mode.
 */
#if !defined(W25Q_BUS_MODE) || defined(__DOXYGEN__)
#define W25Q_BUS_MODE                       W25Q_BUS_MODE_WSPI2L
#endif

/**
 * @brief   Delays insertions.
 * @details If enabled this options inserts delays into the flash waiting
 *          routines releasing some extra CPU time for threads with lower
 *          priority, this may slow down the driver a bit however.
 */
#if !defined(W25Q_NICE_WAITING) || defined(__DOXYGEN__)
#define W25Q_NICE_WAITING                   TRUE
#endif

/**
 * @brief   Uses 4kB sub-sectors rather than 64kB sectors.
 */
#if !defined(W25Q_USE_SUB_SECTORS) || defined(__DOXYGEN__)
#define W25Q_USE_SUB_SECTORS                TRUE
#endif

/**
 * @brief   Size of the compare buffer.
 * @details This buffer is allocated in the stack frame of the function
 *          @p flashVerifyErase() and its size must be a power of two.
 *          Larger buffers lead to better verify performance but increase
 *          stack usage for that function.
 */
#if !defined(W25Q_COMPARE_BUFFER_SIZE) || defined(__DOXYGEN__)
#define W25Q_COMPARE_BUFFER_SIZE            32
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (W25Q_COMPARE_BUFFER_SIZE & (W25Q_COMPARE_BUFFER_SIZE - 1)) != 0
#error "invalid W25Q_COMPARE_BUFFER_SIZE value"
#endif

#if (W25Q_READ_DUMMY_CYCLES != 0) && (W25Q_READ_DUMMY_CYCLES != 2) && (W25Q_READ_DUMMY_CYCLES != 4) && (W25Q_READ_DUMMY_CYCLES != 6) && (W25Q_READ_DUMMY_CYCLES != 8)
#error "invalid W25Q_READ_DUMMY_CYCLES value (2,4,6,8)"
#endif

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_SPI) && (W25Q_BUS_MODE != W25Q_BUS_MODE_SPI)
#error "only W25Q_BUS_MODE_SPI is allowed when using SPI driver"
#endif

#if (W25Q_BUS_MODE == W25Q_BUS_MODE_WSPI4L) || defined(__DOXYGEN__)

#define W25Q_CMD_READ_WSPI W25Q_CMD_FAST_READ_QUAD_IO

/**
 * @brief   WSPI settings for command only.
 */
#define SNOR_WSPI_CFG_CMD               (WSPI_CFG_CMD_MODE_ONE_LINE     | \
                                         WSPI_CFG_ADDR_MODE_NONE          | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_NONE          | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

/**
 * @brief   WSPI settings for command and address.
 */
#define SNOR_WSPI_CFG_CMD_ADDR          (WSPI_CFG_CMD_MODE_FOUR_LINES     | \
                                         WSPI_CFG_ADDR_MODE_FOUR_LINES    | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_NONE          | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

/**
 * @brief   WSPI settings for command and data.
 */
#define SNOR_WSPI_CFG_CMD_DATA          (WSPI_CFG_CMD_MODE_ONE_LINE     | \
                                         WSPI_CFG_ADDR_MODE_NONE          | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_FOUR_LINES    | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

/**
 * @brief   WSPI settings for command, address and data.
 */
#define SNOR_WSPI_CFG_CMD_ADDR_DATA     (WSPI_CFG_CMD_MODE_FOUR_LINES     | \
                                         WSPI_CFG_ADDR_MODE_FOUR_LINES    | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_FOUR_LINES    | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

#elif W25Q_BUS_MODE == W25Q_BUS_MODE_WSPI2L

#define W25Q_CMD_READ_WSPI W25Q_CMD_FAST_READ_DUAL_IO
#define W25Q_READ_DUMMY_CYCLES 8

#define SNOR_WSPI_CFG_CMD               (WSPI_CFG_CMD_MODE_ONE_LINE      | \
                                         WSPI_CFG_ADDR_MODE_NONE          | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_NONE          | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

#define SNOR_WSPI_CFG_CMD_ADDR          (WSPI_CFG_CMD_MODE_ONE_LINE      | \
                                         WSPI_CFG_ADDR_MODE_ONE_LINE     | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_NONE          | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

#define SNOR_WSPI_CFG_CMD_DATA          (WSPI_CFG_CMD_MODE_ONE_LINE       | \
                                         WSPI_CFG_ADDR_MODE_NONE          | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_ONE_LINE       | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

#define SNOR_WSPI_CFG_CMD_ADDR_DATA     (WSPI_CFG_CMD_MODE_ONE_LINE       | \
                                         WSPI_CFG_ADDR_MODE_ONE_LINE      | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_TWO_LINES      | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

#define SNOR_WSPI_CFG_CMD_ADDR_DATA_SEND     (WSPI_CFG_CMD_MODE_ONE_LINE       | \
	                                         WSPI_CFG_ADDR_MODE_ONE_LINE      | \
	                                         WSPI_CFG_ALT_MODE_NONE           | \
	                                         WSPI_CFG_DATA_MODE_ONE_LINE      | \
	                                         WSPI_CFG_CMD_SIZE_8              | \
	                                         WSPI_CFG_ADDR_SIZE_24)

#elif W25Q_BUS_MODE == W25Q_BUS_MODE_WSPI1L

#define W25Q_CMD_READ_WSPI W25Q_CMD_READ
#define W25Q_READ_DUMMY_CYCLES 0

#define SNOR_WSPI_CFG_CMD               (WSPI_CFG_CMD_MODE_ONE_LINE       | \
                                         WSPI_CFG_ADDR_MODE_NONE          | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_NONE          | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

#define SNOR_WSPI_CFG_CMD_ADDR          (WSPI_CFG_CMD_MODE_ONE_LINE       | \
                                         WSPI_CFG_ADDR_MODE_ONE_LINE      | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_NONE          | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

#define SNOR_WSPI_CFG_CMD_DATA          (WSPI_CFG_CMD_MODE_ONE_LINE       | \
                                         WSPI_CFG_ADDR_MODE_NONE          | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_ONE_LINE      | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

#define SNOR_WSPI_CFG_CMD_ADDR_DATA     (WSPI_CFG_CMD_MODE_ONE_LINE       | \
                                         WSPI_CFG_ADDR_MODE_ONE_LINE      | \
                                         WSPI_CFG_ALT_MODE_NONE           | \
                                         WSPI_CFG_DATA_MODE_ONE_LINE      | \
                                         WSPI_CFG_CMD_SIZE_8              | \
                                         WSPI_CFG_ADDR_SIZE_24)

#else
#error "invalid W25Q_BUS_MODE setting"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern flash_descriptor_t snor_descriptor;

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) && (WSPI_SUPPORTS_MEMMAP == TRUE)
extern const wspi_command_t snor_memmap_read;
#endif

extern flash_descriptor_t snor_info;

#ifdef __cplusplus
extern "C" {
#endif
  void snor_device_init(SNORDriver *devp);
  flash_descriptor_t* snor_device_find(SNORDriver *devp);
  flash_error_t snor_device_read(SNORDriver *devp, flash_offset_t offset, size_t n, uint8_t *rp);
  flash_error_t snor_device_program(SNORDriver *devp, flash_offset_t offset, size_t n, const uint8_t *pp);
  flash_error_t snor_device_erase_all(SNORDriver *devp);
  flash_error_t snor_device_erase_sector(SNORDriver *devp, flash_sector_t sector);
  flash_error_t snor_device_verify_erase(SNORDriver *devp, flash_sector_t sector);
  flash_error_t snor_device_query_erase(SNORDriver *devp, uint32_t *msec);
  flash_error_t snor_device_read_sfdp(SNORDriver *devp, flash_offset_t offset, size_t n, uint8_t *rp);
#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) && (SNOR_DEVICE_SUPPORTS_XIP == TRUE)
  void snor_activate_xip(SNORDriver *devp);
  void snor_reset_xip(SNORDriver *devp);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_FLASH_DEVICE_H */

/** @} */

