/*
 * RasPi SPI TPM driver
 *
 * Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
 * Copyright (C) 2023, Jakob Kukla
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Code adapted from:
 *  - TRENTOS RPi_SPI_Flash component
 *  - TRENTOS EntropySource component
 *  - wolfTPM examples/wrap/wrap_test.c (https://github.com/wolfSSL/wolfTPM/blob/master/examples/wrap/wrap_test.c)
 */

#include "OS_Dataport.h"
#include "lib_debug/Debug.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <camkes.h>

#include "bcm2837_gpio.h"
#include "bcm2837_spi.h"

#include "tpm_io.h"
#include <wolftpm/tpm2.h>
#include <wolftpm/tpm2_wrap.h>

#include "RPi_SPI_TPM.h"
#include "util.h"

void tpm_init(void) {
  int rc;
  void *userCtx = NULL;
  WOLFTPM2_CAPS caps;

  // TODO set wait time between wolftpm transfers (XTPM_WAIT macro)
  rc = wolfTPM2_Init(&dev, TPM2_IoCb, userCtx);
  if (rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("TPM2_Init failed 0x%x: %s\n", rc, TPM2_GetRCString(rc));
    return;
  }

  rc = wolfTPM2_GetCapabilities(&dev, &caps);
  Debug_LOG_INFO("Mfg %s (%d), Vendor %s, Fw %u.%u (0x%x), "
                 "FIPS 140-2 %d, CC-EAL4 %d\n",
                 caps.mfgStr, caps.mfg, caps.vendorStr, caps.fwVerMajor,
                 caps.fwVerMinor, caps.fwVerVendor, caps.fips140_2,
                 caps.cc_eal4);
}

void bcm2837_gpio_set_pud(uint8_t pin, uint8_t pud) {
  bcm2837_gpio_pud(pud);
  util_sleep(10);
  bcm2837_gpio_pudclk(pin, 1);
  util_sleep(10);
  bcm2837_gpio_pud(BCM2837_GPIO_PUD_OFF);
  bcm2837_gpio_pudclk(pin, 0);
}

//------------------------------------------------------------------------------
void post_init(void) {
  Debug_LOG_INFO("BCM2837_SPI_TPM init");

  // initialize BCM2837 SPI library
  if (!bcm2837_spi_begin(regBase)) {
    Debug_LOG_ERROR("bcm2837_spi_begin() failed");
    return;
  }

  // Pull PIN 18 (GPIO24) (RESET) to high (is active low):
  // FIXME: This is not necessary for some reason (PIN is pulled high from the
  // start, after setting enable_jtag_gpio=0
  bcm2837_gpio_set_pud(RPI_BPLUS_GPIO_J8_18, BCM2837_GPIO_PUD_UP);

  bcm2837_spi_setBitOrder(BCM2837_SPI_BIT_ORDER_MSBFIRST);
  bcm2837_spi_setDataMode(BCM2837_SPI_MODE0);
  // divider 8 gives 50 MHz assuming the RasPi3 is running with the default
  // 400 MHz, but for some reason we force it to run at just 250 MHz with
  // "core_freq=250" in config.txt and thus end up at 31.25 MHz SPI speed.
  bcm2837_spi_setClockDivider(BCM2837_SPI_CLOCK_DIVIDER_8);
  bcm2837_spi_chipSelect(BCM2837_SPI_CS1);
  bcm2837_spi_setChipSelectPolarity(BCM2837_SPI_CS1, 0);

  tpm_init();

  Debug_LOG_INFO("BCM2837_SPI_TPM done");
}

//------------------------------------------------------------------------------
// Interface EntropySource
//------------------------------------------------------------------------------

static OS_Dataport_t e_port = OS_DATAPORT_ASSIGN(entropy_port);

//------------------------------------------------------------------------------
size_t entropy_rpc_read(const size_t len) {
  size_t sz;
  int rc;
  WOLFTPM2_BUFFER random;

  // Make sure we don't exceed the dataport len
  sz = OS_Dataport_getSize(e_port);
  sz = len > sz ? sz : len;

  random.size = sz;
  memset(random.buffer, 0, sizeof(random.buffer));

  rc = wolfTPM2_GetRandom(&dev, random.buffer, sizeof(random.buffer));
  if (rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("TPM GetRandom failed");
    // TODO: How to report an error with this interface?
    return -1;
  }

  memcpy(OS_Dataport_getBuf(e_port), random.buffer, sz);

  return sz;
}
