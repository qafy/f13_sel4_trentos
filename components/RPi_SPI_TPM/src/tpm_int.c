/*
 * Copyright (C) 2023, Jakob Kukla
 *
 * Code adapted from:
 *  - wolfTPM examples/wrap/wrap_test.c (https://github.com/wolfSSL/wolfTPM/blob/master/examples/wrap/wrap_test.c)
 */

#include "OS_Error.h"
#include "lib_debug/Debug.h"

#include "system_config.h"

#include <wolftpm/tpm2_wrap.h>

#include "RPi_SPI_TPM.h"

OS_Error_t tpm_int_get_or_create_srk(WOLFTPM2_KEY *srk) {
  int rc;
  TPM_HANDLE hierarchy = TPM_RH_OWNER;

  /* See if RSA primary storage key already exists */
  rc = wolfTPM2_ReadPublicKey(&dev, srk, TPM_SRK_HANDLE);

  if (rc != 0) {
    /* Create primary storage key (RSA) */
    rc = wolfTPM2_CreateSRK(&dev, srk, TPM_ALG_RSA, (byte *)TPM_SRK_AUTH,
                            sizeof(TPM_SRK_AUTH) - 1);
    if (rc != TPM_RC_SUCCESS) {
      Debug_LOG_ERROR("wolfTPM2_CreateSRK failed: %s\n", TPM2_GetRCString(rc));
      return OS_ERROR_GENERIC;
    }

    /* Move this key into persistent storage */
    rc = wolfTPM2_NVStoreKey(&dev, hierarchy, srk, TPM_SRK_HANDLE);
    if (rc != TPM_RC_SUCCESS) {
      Debug_LOG_ERROR("wolfTPM2_NVStoreKey failed: %s\n", TPM2_GetRCString(rc));
      return OS_ERROR_GENERIC;
    }

    Debug_LOG_INFO("Created new RSA Primary Storage Key at 0x%x\n",
                   TPM_SRK_HANDLE);
  } else {
    /* specify auth password for storage key */
    srk->handle.auth.size = sizeof(TPM_SRK_AUTH) - 1;
    XMEMCPY(srk->handle.auth.buffer, TPM_SRK_AUTH, srk->handle.auth.size);
  }

  return OS_SUCCESS;
}
