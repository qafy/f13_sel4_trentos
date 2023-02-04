/*
 * Copyright (C) 2023, Jakob Kukla
 *
 * Code adapted from:
 *  - wolfTPM examples/wrap/wrap_test.c (https://github.com/wolfSSL/wolfTPM/blob/master/examples/wrap/wrap_test.c)
 */

#include "OS_Dataport.h"
#include "OS_Error.h"

#include <string.h>

#include <camkes.h>

#include "system_config.h"

#include <wolftpm/tpm2_wrap.h>

#include "RPi_SPI_TPM.h"

static OS_Dataport_t c_port = OS_DATAPORT_ASSIGN(crypto_port);

OS_Error_t crypto_rpc_generateKey(int *size) {
  int rc, tpm_rc;
  WOLFTPM2_KEY key;
  WOLFTPM2_KEY srk;
  TPMT_PUBLIC template;

  rc = tpm_int_get_or_create_srk(&srk);
  if (rc != OS_SUCCESS) {
    printf("keystore_int_get_or_create_srk failed: %d\n", rc);
    return rc;
  }

  tpm_rc = wolfTPM2_GetKeyTemplate_RSA(
      &template, TPMA_OBJECT_sensitiveDataOrigin | TPMA_OBJECT_userWithAuth |
                     TPMA_OBJECT_decrypt | TPMA_OBJECT_noDA);
  if (tpm_rc != TPM_RC_SUCCESS) {
    printf("wolfTPM2_GetKeyTemplate_RSA failed 0x%x: %s\n", tpm_rc,
           TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  tpm_rc =
      wolfTPM2_CreateAndLoadKey(&dev, &key, &srk.handle, &template,
                                (byte *)TPM_RSA_AUTH, sizeof(TPM_RSA_AUTH) - 1);
  if (tpm_rc != TPM_RC_SUCCESS) {
    printf("wolfTPM2_CreateAndLoadKey failed 0x%x: %s\n", tpm_rc,
           TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  *size = sizeof(WOLFTPM2_HANDLE);
  // FIXME: Can we remove this memcpy safely?
  memcpy(OS_Dataport_getBuf(c_port), (void *)&key, *size);

  return OS_SUCCESS;
}

OS_Error_t crypto_rpc_encrypt(int input_size, int *output_size) {
  int tpm_rc;
  // FIXME: what is the correct output length? for OAEP?
  void *output = malloc(OS_Dataport_getSize(c_port));
  WOLFTPM2_KEY key;

  // FIXME: Can we remove this memcpy safely?
  memcpy(&key, OS_Dataport_getBuf(c_port), sizeof(WOLFTPM2_HANDLE));

  tpm_rc =
      wolfTPM2_RsaEncrypt(&dev, &key, TPM_ALG_OAEP,
                          OS_Dataport_getBuf(c_port) + sizeof(WOLFTPM2_HANDLE),
                          input_size, output, output_size);
  if (tpm_rc != TPM_RC_SUCCESS) {
    printf("wolfTPM2_RsaEncrypt failed 0x%x: %s\n", tpm_rc,
           TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  memcpy(OS_Dataport_getBuf(c_port), output, *output_size);
  free(output);

  return OS_SUCCESS;
}

OS_Error_t crypto_rpc_decrypt(int input_size, int *output_size) {
  int tpm_rc;
  // FIXME: what is the correct output length? for OAEP?
  void *output = malloc(OS_Dataport_getSize(c_port));
  WOLFTPM2_KEY key;

  // FIXME: Can we remove this memcpy safely?
  memcpy(&key, OS_Dataport_getBuf(c_port), sizeof(WOLFTPM2_HANDLE));

  tpm_rc =
      wolfTPM2_RsaDecrypt(&dev, &key, TPM_ALG_OAEP,
                          OS_Dataport_getBuf(c_port) + sizeof(WOLFTPM2_HANDLE),
                          input_size, output, output_size);
  if (tpm_rc != TPM_RC_SUCCESS) {
    printf("wolfTPM2_RsaDecrypt failed 0x%x: %s\n", tpm_rc,
           TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  memcpy(OS_Dataport_getBuf(c_port), output, *output_size);
  free(output);

  return OS_SUCCESS;
}