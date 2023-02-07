/* tpm_io.h
 *
 * Copyright (C) 2006-2022 wolfSSL Inc.
 * Copyright (C) 2023, Jakob Kukla
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
 *
 * Code adapted from:
 *  - wolfTPM examples/tpm_io.h (https://github.com/wolfSSL/wolfTPM/blob/a0bd9fef9842ffbdf933afbd15ed4fa8bc8daf26/examples/tpm_io.h)
 */

#ifndef _TPM_IO_H_
#define _TPM_IO_H_

#include <wolftpm/tpm2.h>

/* TPM2 IO Examples */

/** @defgroup TPM2_IO wolfTPM2 IO HAL Callbacks
 *
 * This module describes the available example TPM 2.0 IO HAL Callbacks in
 * wolfTPM
 *
 * wolfTPM uses a single IO callback function.
 * This allows the TPM 2.0 stack to be highly portable.
 * These IO Callbacks are working examples for various embedded platforms and
 * operating systems.
 *
 * Here is a non exhaustive list of the existing TPM 2.0 IO Callbacks
 * * ST Micro STM32, through STM32 CubeMX HAL
 * * Native Linux (/dev/tpm0)
 * * Linux through spidev without kernel driver thanks to wolfTPM own TIS layer
 * * Linux through i2c without kernel driver thanks to wolfTPM own TIS layer
 * * Native Windows
 * * Atmel MCUs
 * * Xilinx Zynq
 * * Barebox
 * * QNX
 *
 * Using custom IO Callback is always possible.
 *
 */

int TPM2_IoCb(TPM2_CTX *ctx, const byte *txBuf, byte *rxBuf, word16 xferSz,
              void *userCtx);

#endif /* _TPM_IO_H_ */
