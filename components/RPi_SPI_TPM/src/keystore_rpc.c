/*
 * Copyright (C) 2023, Jakob Kukla
 */

#include "OS_Dataport.h"
#include "OS_Error.h"

#include <string.h>

#include <camkes.h>

#include "system_config.h"

#include <wolftpm/tpm2_wrap.h>

#include "RPi_SPI_TPM.h"

static OS_Dataport_t k_port = OS_DATAPORT_ASSIGN(keystore_port);

OS_Error_t keystore_rpc_loadKey(int handle, size_t *size) {
  int rc;
  WOLFTPM2_KEY key;

  rc = wolfTPM2_ReadPublicKey(&dev, &key, handle);
  if (rc != TPM_RC_SUCCESS) {
    printf("wolfTPM2_ReadPublicKey failed: %s\n", TPM2_GetRCString(rc));
    return OS_ERROR_INVALID_HANDLE;
  }

  key.handle.auth.size = sizeof(TPM_RSA_AUTH) - 1;
  memcpy(key.handle.auth.buffer, TPM_RSA_AUTH, key.handle.auth.size);

  *size = sizeof(WOLFTPM2_HANDLE);
  // FIXME: Can we remove this memcpy safely?
  memcpy(OS_Dataport_getBuf(k_port), &key, *size);

  return OS_SUCCESS;
}

OS_Error_t keystore_rpc_storeKey(int handle) {
  int rc;
  WOLFTPM2_KEY key;

  // FIXME: Can we remove this memcpy safely?
  memcpy(&key, OS_Dataport_getBuf(k_port), sizeof(WOLFTPM2_HANDLE));

  rc = wolfTPM2_NVStoreKey(&dev, TPM_RH_OWNER, &key, handle);
  if (rc != TPM_RC_SUCCESS) {
    printf("wolfTPM2_NVStoreKey failed: %s\n", TPM2_GetRCString(rc));
    return OS_ERROR_GENERIC;
  }

  memcpy(OS_Dataport_getBuf(k_port), &key, sizeof(WOLFTPM2_HANDLE));

  return OS_SUCCESS;
}
