/*
 * Copyright (C) 2023, Jakob Kukla
 *
 * Code adapted from:
 *  - wolfTPM examples/wrap/wrap_test.c (https://github.com/wolfSSL/wolfTPM/blob/master/examples/wrap/wrap_test.c)
 */

#include "OS_Dataport.h"
#include "OS_Error.h"
#include "lib_debug/Debug.h"

#include <string.h>

#include <camkes.h>

#include "system_config.h"

#include <wolftpm/tpm2_wrap.h>

#include "RPi_SPI_TPM.h"

static OS_Dataport_t c_port = OS_DATAPORT_ASSIGN(crypto_port);

OS_Error_t crypto_rpc_generateKey() {
  int rc, tpm_rc;
  WOLFTPM2_KEY *key = OS_Dataport_getBuf(c_port);
  WOLFTPM2_KEY srk;
  TPMT_PUBLIC template;

  rc = tpm_int_get_or_create_srk(&srk);
  if (rc != OS_SUCCESS) {
    Debug_LOG_ERROR("keystore_int_get_or_create_srk failed: %d\n", rc);
    return rc;
  }

  tpm_rc = wolfTPM2_GetKeyTemplate_RSA(
      &template, TPMA_OBJECT_sensitiveDataOrigin | TPMA_OBJECT_userWithAuth |
                     TPMA_OBJECT_decrypt | TPMA_OBJECT_noDA);
  if (tpm_rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("wolfTPM2_GetKeyTemplate_RSA failed 0x%x: %s\n", tpm_rc,
                    TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  // We need to disable authentification, otherwise external keys wouldn't load
  // properly
  /*
  tpm_rc =
      wolfTPM2_CreateAndLoadKey(&dev, key, &srk.handle, &template,
                                (byte *)TPM_RSA_AUTH, sizeof(TPM_RSA_AUTH) - 1);
  */

  tpm_rc =
      wolfTPM2_CreateAndLoadKey(&dev, key, &srk.handle, &template, NULL, 0);
  if (tpm_rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("wolfTPM2_CreateAndLoadKey failed 0x%x: %s\n", tpm_rc,
                    TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  return OS_SUCCESS;
}

OS_Error_t crypto_rpc_importPublic() {
  int tpm_rc;
  unsigned char raw[260];
  WOLFTPM2_KEY key = {0};
  unsigned char *n = raw;
  unsigned char *e_raw = n + 256;
  size_t nSz = 256;

  memcpy(raw, OS_Dataport_getBuf(c_port), sizeof(raw));

  uint32_t e = e_raw[0] << 24 | e_raw[1] << 16 | e_raw[2] << 8 | e_raw[3];

  tpm_rc = wolfTPM2_LoadRsaPublicKey_ex(&dev, &key, n, nSz, e, TPM_ALG_NULL,
                                        WOLFTPM2_WRAP_DIGEST);
  if (tpm_rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("wolfTPM2_LoadRsaPublicKey failed 0x%x: %s\n", tpm_rc,
                    TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  memcpy(OS_Dataport_getBuf(c_port), &key, sizeof(WOLFTPM2_HANDLE));

  return OS_SUCCESS;
}

OS_Error_t crypto_rpc_importPrivate() {
  int rc, tpm_rc;
  unsigned char raw[772];
  WOLFTPM2_KEYBLOB keyblob;
  WOLFTPM2_KEY srk;

  unsigned char *n = raw;
  unsigned char *p = n + 256;
  unsigned char *q = p + 128;
  unsigned char *d = q + 128;
  unsigned char *e_raw = d + 256;
  size_t nSz = 256;
  size_t qSz = 128;

  memcpy(raw, OS_Dataport_getBuf(c_port), sizeof(raw));

  uint32_t e = e_raw[0] << 24 | e_raw[1] << 16 | e_raw[2] << 8 | e_raw[3];

  rc = tpm_int_get_or_create_srk(&srk);
  if (rc != OS_SUCCESS) {
    Debug_LOG_ERROR("keystore_int_get_or_create_srk failed: %d\n", rc);
    return rc;
  }

  tpm_rc = wolfTPM2_ImportRsaPrivateKey(&dev, &srk, &keyblob, n, nSz, e, q, qSz,
                                        TPM_ALG_NULL, WOLFTPM2_WRAP_DIGEST);
  if (tpm_rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("wolfTPM2_ImportRsaPrivateKey failed 0x%x: %s\n", tpm_rc,
                    TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  tpm_rc = wolfTPM2_LoadKey(&dev, &keyblob, &srk.handle);
  if (tpm_rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("wolfTPM2_LoadKey failed 0x%x: %s\n", tpm_rc,
                    TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  memcpy(OS_Dataport_getBuf(c_port), &keyblob, sizeof(WOLFTPM2_HANDLE));

  return OS_SUCCESS;
}

OS_Error_t crypto_rpc_exportPublicPem(size_t *size) {
  int tpm_rc;
  WOLFTPM2_HANDLE *handle = OS_Dataport_getBuf(c_port);
  WOLFTPM2_KEY key = {0};
  /* output needs to be in heap
   * output_size needs to be a local variable
   */
  unsigned char *output = malloc(OS_Dataport_getSize(c_port));
  size_t output_size;

  tpm_rc = wolfTPM2_ReadPublicKey(&dev, &key, handle->hndl);
  if (tpm_rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("wolfTPM2_ReadPublicKey failed %s\n",
                    TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  tpm_rc = wolfTPM2_RsaKey_TpmToPemPub(&dev, &key, output, &output_size);
  if (tpm_rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("wolfTPM2_TpmToPemPub failed: %s\n",
                    TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  *size = output_size;
  memcpy(OS_Dataport_getBuf(c_port), output, *size);
  free(output);

  return OS_SUCCESS;
}

OS_Error_t crypto_rpc_encrypt(size_t input_size, size_t *output_size) {
  int tpm_rc;
  unsigned char output[OS_Dataport_getSize(c_port)];
  WOLFTPM2_KEY *key = OS_Dataport_getBuf(c_port);

  tpm_rc =
      wolfTPM2_RsaEncrypt(&dev, key, TPM_ALG_OAEP,
                          OS_Dataport_getBuf(c_port) + sizeof(WOLFTPM2_HANDLE),
                          (int)input_size, output, (int *)output_size);
  if (tpm_rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("wolfTPM2_RsaEncrypt failed 0x%x: %s\n", tpm_rc,
                    TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  if (OS_Dataport_getSize(c_port) < *output_size) {
    *output_size = 0;
    Debug_LOG_ERROR("crypto_rpc_encrypt failed: Dataport smaller than output");
    return OS_ERROR_BUFFER_TOO_SMALL;
  }

  memcpy(OS_Dataport_getBuf(c_port), output, *output_size);

  return OS_SUCCESS;
}

OS_Error_t crypto_rpc_decrypt(size_t input_size, size_t *output_size) {
  int tpm_rc;
  unsigned char output[OS_Dataport_getSize(c_port)];
  WOLFTPM2_KEY *key = OS_Dataport_getBuf(c_port);

  tpm_rc =
      wolfTPM2_RsaDecrypt(&dev, key, TPM_ALG_OAEP,
                          OS_Dataport_getBuf(c_port) + sizeof(WOLFTPM2_HANDLE),
                          (int)input_size, output, (int *)output_size);
  if (tpm_rc != TPM_RC_SUCCESS) {
    Debug_LOG_ERROR("wolfTPM2_RsaDecrypt failed 0x%x: %s\n", tpm_rc,
                    TPM2_GetRCString(tpm_rc));
    return OS_ERROR_GENERIC;
  }

  if (OS_Dataport_getSize(c_port) < *output_size) {
    *output_size = 0;
    return OS_ERROR_BUFFER_TOO_SMALL;
  }

  memcpy(OS_Dataport_getBuf(c_port), output, *output_size);

  return OS_SUCCESS;
}
