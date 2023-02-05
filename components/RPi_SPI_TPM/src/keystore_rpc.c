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

OS_Error_t keystore_rpc_loadKey(int handle) {
  int rc;
  WOLFTPM2_KEY *key = OS_Dataport_getBuf(k_port);

  rc = wolfTPM2_ReadPublicKey(&dev, key, handle);
  if (rc != TPM_RC_SUCCESS) {
    printf("wolfTPM2_ReadPublicKey couldn't find key at handle 0x%x: %s\n",
           handle, TPM2_GetRCString(rc));
    return OS_ERROR_INVALID_HANDLE;
  }

  // We need to disable authentification, otherwise external keys wouldn't load
  // properly
  /*
  key->handle.auth.size = sizeof(TPM_RSA_AUTH) - 1;
  memcpy(key->handle.auth.buffer, TPM_RSA_AUTH, key->handle.auth.size);
  */

  return OS_SUCCESS;
}

OS_Error_t keystore_rpc_storeKey(int handle) {
  int rc;
  WOLFTPM2_KEY *key = OS_Dataport_getBuf(k_port);

  rc = wolfTPM2_NVStoreKey(&dev, TPM_RH_OWNER, key, handle);
  if (rc != TPM_RC_SUCCESS) {
    printf("wolfTPM2_NVStoreKey failed: %s\n", TPM2_GetRCString(rc));
    return OS_ERROR_GENERIC;
  }

  return OS_SUCCESS;
}
