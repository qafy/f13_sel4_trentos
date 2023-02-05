/*
 * Copyright (C) 2023, Jakob Kukla
 */

#include "OS_Dataport.h"
#include "OS_Error.h"

#include <stdlib.h>
#include <string.h>

#include <camkes.h>

#include "crypto.h"

OS_Error_t TPM_Crypto_generateKey(const TPM_Crypto_Handle_t *ctx,
                                  TPM_Crypto_Key_t *key) {
  int rc;

  rc = ctx->generateKey();

  memcpy(key, OS_Dataport_getBuf(ctx->dataport), TPM_CRYPTO_KEY_SIZE);

  return rc;
}

OS_Error_t TPM_Crypto_encrypt(const TPM_Crypto_Handle_t *ctx,
                              TPM_Crypto_Key_t *key, unsigned char *input,
                              size_t input_size, unsigned char *output,
                              size_t *output_size) {
  int rc;

  memcpy(OS_Dataport_getBuf(ctx->dataport), key, TPM_CRYPTO_KEY_SIZE);
  memcpy(OS_Dataport_getBuf(ctx->dataport) + TPM_CRYPTO_KEY_SIZE, input,
         input_size);

  rc = ctx->encrypt(input_size, output_size);
  if (rc != OS_SUCCESS) {
    *output_size = 0;
    return rc;
  }

  memcpy(output, OS_Dataport_getBuf(ctx->dataport), *output_size);

  return rc;
}

OS_Error_t TPM_Crypto_decrypt(const TPM_Crypto_Handle_t *ctx,
                              TPM_Crypto_Key_t *key, unsigned char *input,
                              size_t input_size, unsigned char *output,
                              size_t *output_size) {
  int rc;

  memcpy(OS_Dataport_getBuf(ctx->dataport), key, TPM_CRYPTO_KEY_SIZE);
  memcpy(OS_Dataport_getBuf(ctx->dataport) + TPM_CRYPTO_KEY_SIZE, input,
         input_size);

  rc = ctx->decrypt(input_size, output_size);
  if (rc != OS_SUCCESS) {
    *output_size = 0;
    return rc;
  }

  memcpy(output, OS_Dataport_getBuf(ctx->dataport), *output_size);

  return rc;
}