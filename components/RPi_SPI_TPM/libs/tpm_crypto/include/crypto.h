/*
 * Copyright (C) 2023, Jakob Kukla
 */

#ifndef _TPM_CRYPTO_H
#define _TPM_CRYPTO_H

// this corresponds to sizeof(WOLFTPM2_HANDLE)
#define TPM_CRYPTO_KEY_SIZE 156

#include "OS_Dataport.h"
#include "OS_Error.h"

typedef unsigned char TPM_Crypto_Key_t[TPM_CRYPTO_KEY_SIZE];

typedef struct {
  OS_Error_t (*generateKey)();
  OS_Error_t (*encrypt)(size_t input_size, size_t *output_size);
  OS_Error_t (*decrypt)(size_t input_size, size_t *output_size);
  OS_Dataport_t dataport;
} TPM_Crypto_Handle_t;

OS_Error_t TPM_Crypto_generateKey(const TPM_Crypto_Handle_t *ctx,
                                  TPM_Crypto_Key_t *key);

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
  .encrypt            = _rpc_ ## _encrypt,          \
  .decrypt            = _rpc_ ## _decrypt,          \
  .dataport           = OS_DATAPORT_ASSIGN(_port_)  \
}

#endif /* _TPM_CRYPTO_H */