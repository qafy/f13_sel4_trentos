/*
 * Copyright (C) 2023, Jakob Kukla
 */

#ifndef _TPM_KEYSTORE_H
#define _TPM_KEYSTORE_H

#include "OS_Dataport.h"
#include "OS_Error.h"

#include "TPM_Crypto.h"

typedef struct {
  OS_Error_t (*loadKey)(int handle);
  OS_Error_t (*storeKey)(int handle);
  OS_Error_t (*clearTPM)();
  OS_Dataport_t dataport;
} TPM_Keystore_Handle_t;

OS_Error_t TPM_Keystore_loadKey(const TPM_Keystore_Handle_t *ctx, size_t handle,
                                TPM_Crypto_Key_t *key);

OS_Error_t TPM_Keystore_storeKey(const TPM_Keystore_Handle_t *ctx,
                                 size_t handle, TPM_Crypto_Key_t *key);

/* This will clear the entire TPM, not just the Keystore. Use with care! */
OS_Error_t TPM_Keystore_clearTPM(const TPM_Keystore_Handle_t *ctx);

#define IF_TPM_KEYSTORE_ASSIGN(_rpc_, _port_)       \
{                                                   \
  .loadKey            = _rpc_ ## _loadKey,          \
  .storeKey           = _rpc_ ## _storeKey,         \
  .clearTPM           = _rpc_ ## _clearTPM,         \
  .dataport           = OS_DATAPORT_ASSIGN(_port_)  \
}

#endif /* _TPM_KEYSTORE_H */