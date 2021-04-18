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
 * @brief   WINBOND W25Q serial flash driver code.
 *
 * @addtogroup WINBOND W25Q
 * @{
 */

#include <string.h>

#include "hal.h"
#include "hal_serial_nor.h"

#define PAGE_SIZE                           256U
#define PAGE_MASK                           (PAGE_SIZE - 1U)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if W25Q_USE_SUB_SECTORS == TRUE
#define SECTOR_SIZE                         0x00001000U
#define CMD_SECTOR_ERASE                    W25Q_CMD_SUBSECTOR_ERASE
#else
#define SECTOR_SIZE                         0x00010000U
#define CMD_SECTOR_ERASE                    W25Q_CMD_SECTOR_ERASE
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   W25QXX descriptor.
 */
flash_descriptor_t snor_descriptor = {
		.attributes       = FLASH_ATTR_ERASED_IS_ONE | FLASH_ATTR_REWRITABLE | FLASH_ATTR_SUSPEND_ERASE_CAPABLE,
		.page_size        = 0u,
		.sectors_count    = 0U,           /* It is overwritten.*/
		.sectors          = NULL,
		.sectors_size     = SECTOR_SIZE,
		.address          = 0U,           /* It is overwritten.*/
		.size             = 0U            /* It is overwritten.*/
};

flash_descriptor_t snor_info;

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) || defined(__DOXYGEN__)
#if (WSPI_SUPPORTS_MEMMAP == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Fast read command for memory mapped mode.
 */
/*const wspi_command_t snor_memmap_read = {
		(void)devp;
		(void)rp;
		(void)offset;
		(void)n;

		return FLASH_NO_ERROR;
};*/
#endif
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
/* Initial W25Q_CMD_READ_ID command.*/
static const wspi_command_t w25q_cmd_read_id = {
  .cmd              = W25Q_CMD_READ_ID,
  .cfg              = 0U |
#if W25Q_SWITCH_WIDTH == TRUE
                      WSPI_CFG_CMD_MODE_ONE_LINE |
                      WSPI_CFG_DATA_MODE_ONE_LINE,
#else
#if W25Q_BUS_MODE == W25Q_BUS_MODE_WSPI1L
                      WSPI_CFG_CMD_MODE_ONE_LINE |
                      WSPI_CFG_DATA_MODE_ONE_LINE,
#elif W25Q_BUS_MODE == W25Q_BUS_MODE_WSPI2L
					  WSPI_CFG_DATA_MODE_TWO_LINES |
                      WSPI_CFG_DATA_MODE_TWO_LINES,
#elif W25QBUS_MODE == W5Q_BUS_MODE_WSPI4L
                      WSPI_CFG_CMD_MODE_FOUR_LINES |
                      WSPI_CFG_DATA_MODE_FOUR_LINES,
#endif
#endif
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};
#endif

/* Initial W25Q_CMD_WRITE_ENABLE command.*/
static const wspi_command_t n25q_cmd_write_enable = {
  .cmd              = W25Q_CMD_WRITE_ENABLE,
  .cfg              = WSPI_CFG_CMD_MODE_ONE_LINE,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 *
 * @param set
 * @param size
 * @param element
 * @return
 */
static bool w25q_find_id(const uint8_t *set, size_t size, uint8_t element) {
  size_t i;

  for (i = 0; i < size; i++) {
    if (set[i] == element) {
      return true;
    }
  }
  return false;
}

/**
 *
 * @param devp
 * @return
 */
static flash_error_t w25q_poll_status(SNORDriver *devp) {
  uint8_t sts;

  do {
#if W25Q_NICE_WAITING == TRUE
    osalThreadSleepMilliseconds(1);
#endif
    /* Read status command.*/
    bus_cmd_receive(devp->config->busp, W25Q_CMD_READ_FLAG_STATUS_REGISTER_1, 1, &sts);
  } while ((sts & W25Q_FLAGS_PROGRAM_BUSY) == 0x1U);

  return FLASH_NO_ERROR;
}

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) || defined(__DOXYGEN__)
/**
 *
 * @param devp
 */
static void w25q_reset_memory(SNORDriver *devp) {

  /* 1x W25Q_CMD_RESET_ENABLE command.*/
  static const wspi_command_t cmd_reset_enable_1 = {
    .cmd              = W25Q_CMD_RESET_ENABLE,
    .cfg              = WSPI_CFG_CMD_MODE_ONE_LINE,
    .addr             = 0,
    .alt              = 0,
    .dummy            = 0
  };

  /* 1x W25Q_CMD_RESET_MEMORY command.*/
  static const wspi_command_t cmd_reset_memory_1 = {
    .cmd              = W25Q_CMD_RESET_MEMORY,
    .cfg              = WSPI_CFG_CMD_MODE_ONE_LINE,
    .addr             = 0,
    .alt              = 0,
    .dummy            = 0
  };

  wspiCommand(devp->config->busp, &cmd_reset_enable_1);
  wspiCommand(devp->config->busp, &cmd_reset_memory_1);
}
#endif /* SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI */

static const uint8_t w25q_manufacturer_ids[] = W25Q_SUPPORTED_MANUFACTURE_IDS;
static const uint8_t w25q_memory_type_ids[] = W25Q_SUPPORTED_MEMORY_TYPE_IDS;

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 *
 * @param devp
 */
void snor_device_init(SNORDriver *devp) {

  w25q_poll_status(devp);

  /* Reading device ID.*/
  bus_cmd_receive(devp->config->busp, W25Q_CMD_READ_ID,  sizeof devp->device_id, devp->device_id);

  /* Checking if the device is white listed.*/
  osalDbgAssert(w25q_find_id(w25q_manufacturer_ids, sizeof w25q_manufacturer_ids, devp->device_id[0]), "invalid manufacturer id");
  osalDbgAssert(devp->device_id[1] = 0x40, "invalid memory type id");
  osalDbgAssert(w25q_find_id(w25q_memory_type_ids, sizeof w25q_memory_type_ids, devp->device_id[2]), "invalid memory type id");

  /* Setting up the device size.*/
  snor_descriptor.sectors_count = (1U << (size_t)devp->device_id[2]) / SECTOR_SIZE;
  snor_descriptor.size = (size_t)snor_descriptor.sectors_count * SECTOR_SIZE;

#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI && W25Q_BUS_MODE == W25Q_BUS_MODE_QPI
  {
    static const uint8_t flash_conf[1] = {
      (W25Q_READ_DUMMY_CYCLES << 4U) | 0x0FU
    };

    /* Setting up the dummy cycles to be used for fast read operations.*/
    bus_cmd(devp->config->busp, W25Q_CMD_WRITE_ENABLE);
    bus_cmd_send(devp->config->busp, W25Q_CMD_WRITE_V_CONF_REGISTER, 1, flash_conf);
  }
#endif
  w25q_poll_status(devp);
}

/**
 *
 * @param devp
 */
flash_descriptor_t* snor_device_find(SNORDriver *devp) {

	//enable reset
	bus_cmd(devp->config->busp, W25Q_CMD_ENABLE_RESET);
	bus_cmd(devp->config->busp, W25Q_CMD_RESET);

	//wait for ready
	w25q_poll_status(devp);

	/* Reading device ID.*/
	bus_cmd_receive(devp->config->busp, W25Q_CMD_READ_ID,  sizeof devp->device_id, devp->device_id);

	snor_info.sectors_count = 0;
	snor_info.size = 0;

	if(w25q_find_id(w25q_manufacturer_ids, sizeof w25q_manufacturer_ids, devp->device_id[0]) && devp->device_id[1] == 0x40 && w25q_find_id(w25q_memory_type_ids, sizeof w25q_memory_type_ids, devp->device_id[2])){
		snor_info.sectors_count = (1U << (size_t)devp->device_id[2]) / SECTOR_SIZE;
		snor_info.size = (size_t)snor_info.sectors_count * SECTOR_SIZE;
	}

	return &snor_info;
}

/**
 *
 * @param devp
 * @param offset
 * @param n
 * @param rp
 * @return
 */
flash_error_t snor_device_read(SNORDriver *devp, flash_offset_t offset, size_t n, uint8_t *rp) {
  w25q_poll_status(devp);
#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
  /* Fast read command in WSPI mode.*/
  bus_cmd_addr_dummy_receive(devp->config->busp, W25Q_CMD_READ_WSPI, offset, W25Q_READ_DUMMY_CYCLES, n, rp);
#else
  /* Normal read command in SPI mode.*/
  bus_cmd_addr_receive(devp->config->busp, W25Q_CMD_READ, offset, n, rp);
#endif
  w25q_poll_status(devp);
  return FLASH_NO_ERROR;
}

/**
 *
 * @param devp
 * @param offset
 * @param n
 * @param pp
 * @return
 */
flash_error_t snor_device_program(SNORDriver *devp, flash_offset_t offset, size_t n, const uint8_t *pp) {
  w25q_poll_status(devp);
  /* Data is programmed page by page.*/
  while (n > 0U) {
    /* Data size that can be written in a single program page operation.*/
    size_t chunk = (size_t)(((offset | PAGE_MASK) + 1U) - offset);
    if (chunk > n) {
      chunk = n;
    }

    /* Enabling write operation.*/
    bus_cmd(devp->config->busp, W25Q_CMD_WRITE_ENABLE);

    /* Page program command.*/
    bus_cmd_addr_send(devp->config->busp, W25Q_CMD_PAGE_PROGRAM, offset, chunk, pp);

    /* Wait for status and check errors.*/
    w25q_poll_status(devp);

    /* Next page.*/
    offset += chunk;
    pp     += chunk;
    n      -= chunk;
  }
  return FLASH_NO_ERROR;
}

/**
 *
 * @param devp
 * @return
 */
flash_error_t snor_device_erase_all(SNORDriver *devp) {
  w25q_poll_status(devp);
  /* Enabling write operation.*/
  bus_cmd(devp->config->busp, W25Q_CMD_WRITE_ENABLE);

  /* Bulk erase command.*/
  bus_cmd(devp->config->busp, W25Q_CMD_BULK_ERASE);
  w25q_poll_status(devp);
  return FLASH_NO_ERROR;
}

/**
 *
 * @param devp
 * @param sector
 * @return
 */
flash_error_t snor_device_erase_sector(SNORDriver *devp, flash_sector_t sector) {
  flash_offset_t offset = (flash_offset_t)(sector * SECTOR_SIZE);

  w25q_poll_status(devp);
#if W25Q_BUS_MODE == W25Q_BUS_MODE_SPI
	/* Enabling write operation.*/
	bus_cmd(devp->config->busp, W25Q_CMD_WRITE_ENABLE);

#if MX25_USE_SUB_SECTORS == FALSE
	/* Block erase command.*/
	bus_cmd_addr(devp->config->busp, W25Q_CMD_BLOCK_32K_ERASE, offset);
#else
	/* Sector erase command.*/
  bus_cmd_addr(devp->config->busp, W25Q_CMD_SECTOR_ERASE, offset);
#endif
#else
	/* Enabling write operation.*/
  bus_cmd(devp->config->busp, W25Q_CMD_WRITE_ENABLE);

#if W25Q_USE_SUB_SECTORS == FALSE
  /* Block erase command.*/
  bus_cmd_addr(devp->config->busp, W25Q_CMD_BLOCK_32K_ERASE, offset);
#else
  /* Sector erase command.*/
  bus_cmd_addr(devp->config->busp, W25Q_CMD_SECTOR_ERASE, offset);
#endif
#endif

  w25q_poll_status(devp);
  chThdSleepMilliseconds(20 );
  return FLASH_NO_ERROR;
}

/**
 *
 * @param devp
 * @param sector
 * @return
 */
flash_error_t snor_device_verify_erase(SNORDriver *devp, flash_sector_t sector) {
  uint8_t cmpbuf[W25Q_COMPARE_BUFFER_SIZE];
  flash_offset_t offset;
  size_t n;

  /* Read command.*/
  offset = (flash_offset_t)(sector * SECTOR_SIZE);
  n = SECTOR_SIZE;
  while (n > 0U) {
    uint8_t *p;

#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
   bus_cmd_addr_dummy_receive(devp->config->busp, W25Q_CMD_READ_WSPI,  offset, W25Q_READ_DUMMY_CYCLES, sizeof cmpbuf, cmpbuf);
#else
   /* Normal read command in SPI mode.*/
   bus_cmd_addr_receive(devp->config->busp, W25Q_CMD_READ, offset, sizeof cmpbuf, cmpbuf);
#endif

    /* Checking for erased state of current buffer.*/
    for (p = cmpbuf; p < &cmpbuf[W25Q_COMPARE_BUFFER_SIZE]; p++) {
      if (*p != 0xFFU) {
        /* Ready state again.*/
        devp->state = FLASH_READY;

        return FLASH_ERROR_VERIFY;
      }
    }

    offset += sizeof cmpbuf;
    n -= sizeof cmpbuf;
  }

  return FLASH_NO_ERROR;
}

/**
 *
 * @param devp
 * @param msec
 * @return
 */
flash_error_t snor_device_query_erase(SNORDriver *devp, uint32_t *msec) {
  uint8_t sts;

	(void)devp;
	(void)msec;

	return FLASH_NO_ERROR;
}

/**
 *
 * @param devp
 * @param offset
 * @param n
 * @param rp
 * @return
 */
flash_error_t snor_device_read_sfdp(SNORDriver *devp, flash_offset_t offset,
                                    size_t n, uint8_t *rp) {

  (void)devp;
  (void)rp;
  (void)offset;
  (void)n;

  return FLASH_NO_ERROR;
}

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) || defined(__DOXYGEN__)
void snor_activate_xip(SNORDriver *devp) {
	(void)devp;
}

#endif /* SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI */

/** @} */


