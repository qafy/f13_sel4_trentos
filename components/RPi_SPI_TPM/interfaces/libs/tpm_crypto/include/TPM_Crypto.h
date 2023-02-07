/*
 * Copyright (C) 2023, Jakob Kukla
 */

#ifndef _TPM_CRYPTO_H
#define _TPM_CRYPTO_H

/* this corresponds to sizeof(WOLFTPM2_HANDLE) */
#define TPM_CRYPTO_KEY_SIZE 156

/* these correspond to the values used in `crypto_rpc.c` importPublic and
 * importPrivate functions */
#define TPM_CRYPO_PUBLIC_RAW_SIZE 260
#define TPM_CRYPO_PRIVATE_RAW_SIZE 772

#include "OS_Dataport.h"
#include "OS_Error.h"

typedef unsigned char TPM_Crypto_Key_t[TPM_CRYPTO_KEY_SIZE];

typedef struct {
  OS_Error_t (*generateKey)();
  OS_Error_t (*importPublic)();
  OS_Error_t (*importPrivate)();
  OS_Error_t (*exportPublicPem)(size_t *size);
  OS_Error_t (*encrypt)(size_t input_size, size_t *output_size);
  OS_Error_t (*decrypt)(size_t input_size, size_t *output_size);
  OS_Dataport_t dataport;
} TPM_Crypto_Handle_t;

OS_Error_t TPM_Crypto_generateKey(const TPM_Crypto_Handle_t *ctx,
                                  TPM_Crypto_Key_t *key);

/*
 * TPM_Crypto_importPublic:
 *
 * void *raw is an array of TPM_CRYPO_PUBLIC_RAW_SIZE=260 bytes with the
 * following properties:
 *
 *  0                             128                               256
 * +---------------------------------------------------------------+
 * |                               n                               |
 * +-------+-------------------------------------------------------+
 * |   e   |
 * +-------+
 *  256     260
 *
 * where
 *   - n (256 bytes) is the public modulus n
 *   - e (4 bytes) is the public exponent e
 */
OS_Error_t TPM_Crypto_importPublic(const TPM_Crypto_Handle_t *ctx,
                                   TPM_Crypto_Key_t *key, void *raw);

/*
 * TPM_Crypto_importPrivate:
 *
 * void *raw is an array of TPM_CRYPO_PRIVATE_RAW_SIZE=772 bytes with the
 * following properties:
 *
 *  0                             128                               256
 * +---------------------------------------------------------------+
 * |                               n                               |
 * +-------------------------------+-------------------------------+
 * |                p              |               q               |
 * +-------------------------------+-------------------------------+
 * |                               d                               |
 * +-------+-------------------------------------------------------+
 * |   e   |
 * +-------+
 *  768     772
 *
 * where
 *   - n (256 bytes) is the public modulus n
 *   - p (128 bytes) is the first prime multiple of the modulus p
 *   - q (128 bytes) is the second prime multiple of the modulus q
 *   - d (256 bytes) is the private exponent d
 *   - e (4 bytes) is the public exponent e
 */
OS_Error_t TPM_Crypto_importPrivate(const TPM_Crypto_Handle_t *ctx,
                                    TPM_Crypto_Key_t *key, void *raw);

OS_Error_t TPM_Crypto_exportPublicPem(const TPM_Crypto_Handle_t *ctx,
                                      TPM_Crypto_Key_t *key,
                                      unsigned char *output,
                                      size_t *output_size);

OS_Error_t TPM_Crypto_encrypt(const TPM_Crypto_Handle_t *ctx,
                              TPM_Crypto_Key_t *key, unsigned char *input,
                              size_t input_size, unsigned char *output,
                              size_t *output_size);

OS_Error_t TPM_Crypto_decrypt(const TPM_Crypto_Handle_t *ctx,
                              TPM_Crypto_Key_t *key, unsigned char *input,
                              size_t input_size, unsigned char *output,
                              size_t *output_size);

#define IF_TPM_CRYPTO_ASSIGN(_rpc_, _port_)         \
{                                                   \
  .generateKey        = _rpc_ ## _generateKey,      \
  .importPublic       = _rpc_ ## _importPublic,     \
  .importPrivate      = _rpc_ ## _importPrivate,    \
  .exportPublicPem    = _rpc_ ## _exportPublicPem,  \
  .encrypt            = _rpc_ ## _encrypt,          \
  .decrypt            = _rpc_ ## _decrypt,          \
  .dataport           = OS_DATAPORT_ASSIGN(_port_)  \
}

#endif /* _TPM_CRYPTO_H */