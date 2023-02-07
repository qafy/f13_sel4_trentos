/**
 * \file pk.h
 *
 * \brief Public Key abstraction layer
 */
/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 *
 *  This file is provided under the Apache License 2.0, or the
 *  GNU General Public License v2.0 or later.
 *
 *  **********
 *  Apache License 2.0:
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  **********
 *
 *  **********
 *  GNU General Public License v2.0 or later:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  **********
 */

#ifndef MBEDTLS_PK_WRITE_H
#define MBEDTLS_PK_WRITE_H
#undef MBEDTLS_CONFIG_FILE
#if !defined(MBEDTLS_CONFIG_FILE)
#include "config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "md.h"

#if defined(MBEDTLS_RSA_C)
#include "rsa.h"
#endif

#if defined(MBEDTLS_ECP_C)
#include "ecp.h"
#endif

#if defined(MBEDTLS_ECDSA_C)
#include "ecdsa.h"
#endif
#define MBEDTLS_PEM_WRITE_C
#if defined(MBEDTLS_PEM_WRITE_C)

/**
 * \brief           Write a buffer of PEM information from a DER encoded
 *                  buffer.
 *
 * \param header    The header string to write.
 * \param footer    The footer string to write.
 * \param der_data  The DER data to encode.
 * \param der_len   The length of the DER data \p der_data in Bytes.
 * \param buf       The buffer to write to.
 * \param buf_len   The length of the output buffer \p buf in Bytes.
 * \param olen      The address at which to store the total length written
 *                  or required (if \p buf_len is not enough).
 *
 * \note            You may pass \c NULL for \p buf and \c 0 for \p buf_len
 *                  to request the length of the resulting PEM buffer in
 *                  `*olen`.
 *
 * \note            This function may be called with overlapping \p der_data
 *                  and \p buf buffers.
 *
 * \return          \c 0 on success.
 * \return          #MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL if \p buf isn't large
 *                  enough to hold the PEM buffer. In  this case, `*olen` holds
 *                  the required minimum size of \p buf.
 * \return          Another PEM or BASE64 error code on other kinds of failure.
 */
int mbedtls_pem_write_buffer( const char *header, const char *footer,
                      const unsigned char *der_data, size_t der_len,
                      unsigned char *buf, size_t buf_len, size_t *olen );
                      
/**
 * \brief           Write a public key to a PEM string
 *
 * \param ctx       PK context which must contain a valid public or private key.
 * \param buf       Buffer to write to. The output includes a
 *                  terminating null byte.
 * \param size      Size of the buffer in bytes.
 *
 * \return          0 if successful, or a specific error code
 */
int mbedtls_pk_write_pubkey_pem( mbedtls_pk_context *ctx, unsigned char *buf, size_t size );

/**
 * \brief           Write a private key to a PKCS#1 or SEC1 PEM string
 *
 * \param ctx       PK context which must contain a valid private key.
 * \param buf       Buffer to write to. The output includes a
 *                  terminating null byte.
 * \param size      Size of the buffer in bytes.
 *
 * \return          0 if successful, or a specific error code
 */
int mbedtls_pk_write_key_pem( mbedtls_pk_context *ctx, unsigned char *buf, size_t size );


#endif /* MBEDTLS_PEM_WRITE_C */


#endif /* MBEDTLS_PK_WRITE_H */
