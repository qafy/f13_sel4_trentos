/*
 * Copyright (C) 2023, Jakob Kukla
 */

#include "OS_Dataport.h"
#include "OS_Error.h"

#include <stdlib.h>
#include <string.h>

#include <camkes.h>

#include "crypto.h"

OS_Error_t TPM_Crypto_generateKey(const TPM_Crypto_Handle_t *ctx, void *key,
                                  int *size) {
  int rc;

  rc = ctx->generateKey(size);

  memcpy(key, OS_Dataport_getBuf(ctx->dataport), *size);

  return rc;
}

OS_Error_t TPM_Crypto_encrypt(const TPM_Crypto_Handle_t *ctx, void *key,
                              int key_size, unsigned char *input,
                              int input_size, unsigned char *output,
                              int *output_size) {
  int rc;

  memcpy(OS_Dataport_getBuf(ctx->dataport), key, key_size);
  memcpy(OS_Dataport_getBuf(ctx->dataport) + key_size, input, input_size);

  rc = ctx->encrypt(input_size, output_size);
  if (rc != OS_SUCCESS) {
    *output_size = 0;
    return rc;
  }

  memcpy(output, OS_Dataport_getBuf(ctx->dataport), *output_size);

  return rc;
}

OS_Error_t TPM_Crypto_decrypt(const TPM_Crypto_Handle_t *ctx, void *key,
                              int key_size, unsigned char *input,
                              int input_size, unsigned char *output,
                              int *output_size) {
  int rc;

  memcpy(OS_Dataport_getBuf(ctx->dataport), key, key_size);
  memcpy(OS_Dataport_getBuf(ctx->dataport) + key_size, input, input_size);

  rc = ctx->decrypt(input_size, output_size);
  if (rc != OS_SUCCESS) {
    *output_size = 0;
    return rc;
  }

  memcpy(output, OS_Dataport_getBuf(ctx->dataport), *output_size);

  return rc;
}