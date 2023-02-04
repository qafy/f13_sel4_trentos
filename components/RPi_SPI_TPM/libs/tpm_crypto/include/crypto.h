/*
 * Copyright (C) 2023, Jakob Kukla
 */

#ifndef _TPM_CRYPTO_H
#define _TPM_CRYPTO_H

#include "OS_Dataport.h"
#include "OS_Error.h"

typedef struct {
  OS_Error_t (*generateKey)(int *size);
  OS_Error_t (*encrypt)(int input_size, int *output_size);
  OS_Error_t (*decrypt)(int input_size, int *output_size);
  OS_Dataport_t dataport;
} TPM_Crypto_Handle_t;

OS_Error_t TPM_Crypto_generateKey(const TPM_Crypto_Handle_t *ctx, void *key,
                                  int *size);

OS_Error_t TPM_Crypto_encrypt(const TPM_Crypto_Handle_t *ctx, void *key,
                              int key_size, unsigned char *input,
                              int input_size, unsigned char *output,
                              int *output_size);

OS_Error_t TPM_Crypto_decrypt(const TPM_Crypto_Handle_t *ctx, void *key,
                              int key_size, unsigned char *input,
                              int input_size, unsigned char *output,
                              int *output_size);

#define IF_TPM_CRYPTO_ASSIGN(_rpc_, _port_)         \
{                                                   \
  .generateKey        = _rpc_ ## _generateKey,      \
  .encrypt            = _rpc_ ## _encrypt,          \
  .decrypt            = _rpc_ ## _decrypt,          \
  .dataport           = OS_DATAPORT_ASSIGN(_port_)  \
}

#endif /* _TPM_CRYPTO_H */