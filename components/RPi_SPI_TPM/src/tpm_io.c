/* tpm_io.c
 *
 * Copyright (C) 2006-2022 wolfSSL Inc.
 *
 * This file is part of wolfTPM.
 *
 * wolfTPM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfTPM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

/* This source code provides example TPM IO HAL Callbacks for various platforms
 *
 * NB: wolfTPM projects requires only #include "tpm_io.h" and
 *     the appropriate defines for the platform in use.
 *
 *     Use cases that do not require an IO callback:
 *      - Native Linux
 *      - Native Windows
 *      - TPM Simulator
 *
 */

#include <wolftpm/tpm2.h>
#include <wolftpm/tpm2_tis.h>

#include "tpm_io.h"

#include "bcm2837_spi.h"

/******************************************************************************/
/* --- BEGIN IO Callback Logic -- */
/******************************************************************************/

/* IO Callback */
int TPM2_IoCb(TPM2_CTX *ctx, const byte *txBuf, byte *rxBuf, word16 xferSz,
              void *userCtx) {
  bcm2837_spi_transfernb((char *)txBuf, (char *)rxBuf, xferSz);

#ifdef WOLFTPM_DEBUG_IO
  printf("TPM2_IoCb: Ret %d, Sz %d\n", ret, xferSz);
  TPM2_PrintBin(txBuf, xferSz);
  TPM2_PrintBin(rxBuf, xferSz);
#endif

  (void)userCtx;
  (void)ctx;

  // FIXME: There is no way of knowing if the transfer was successful
  return TPM_RC_SUCCESS;
}

/******************************************************************************/
/* --- END IO Callback Logic -- */
/******************************************************************************/
