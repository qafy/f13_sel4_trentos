/*
 * RasPi SPI TPM driver
 *
 * Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "lib_debug/Debug.h"

#include "bcm2837_spi.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <camkes.h>

//------------------------------------------------------------------------------
void post_init(void) {
  Debug_LOG_INFO("BCM2837_SPI_TPM init");

  // initialize BCM2837 SPI library
  if (!bcm2837_spi_begin(regBase)) {
    Debug_LOG_ERROR("bcm2837_spi_begin() failed");
    return;
  }

  bcm2837_spi_setBitOrder(BCM2837_SPI_BIT_ORDER_MSBFIRST);
  bcm2837_spi_setDataMode(BCM2837_SPI_MODE0);
  // divider 8 gives 50 MHz assuming the RasPi3 is running with the default
  // 400 MHz, but for some reason we force it to run at just 250 MHz with
  // "core_freq=250" in config.txt and thus end up at 31.25 MHz SPI speed.
  bcm2837_spi_setClockDivider(BCM2837_SPI_CLOCK_DIVIDER_8);
  bcm2837_spi_chipSelect(BCM2837_SPI_CS0);
  bcm2837_spi_setChipSelectPolarity(BCM2837_SPI_CS0, 0);

  Debug_LOG_INFO("BCM2837_SPI_TPM done");
}
