/*
 * Copyright (C) 2023, Jakob Kukla
 */

#include "OS_Dataport.h"
#include "OS_Error.h"

#include <stdlib.h>
#include <string.h>

#include <camkes.h>

#include "system_config.h"

#include "keystore.h"

OS_Error_t TPM_Keystore_loadKey(const TPM_Keystore_Handle_t *ctx, size_t handle,
                                void *key, size_t *size) {
  int rc;

  rc = ctx->loadKey(TPM_RSA_BASE_HANDLE + handle, size);

  memcpy(key, OS_Dataport_getBuf(ctx->dataport), *size);

  return rc;
}

OS_Error_t TPM_Keystore_storeKey(const TPM_Keystore_Handle_t *ctx,
                                 size_t handle, void *key, size_t size) {
  int rc;

  memcpy(OS_Dataport_getBuf(ctx->dataport), key, size);

  rc = ctx->storeKey(TPM_RSA_BASE_HANDLE + handle);

  memcpy(key, OS_Dataport_getBuf(ctx->dataport), size);

  return rc;
}
