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
  OS_Dataport_t dataport;
} TPM_Keystore_Handle_t;

OS_Error_t TPM_Keystore_loadKey(const TPM_Keystore_Handle_t *ctx, size_t handle,
                                TPM_Crypto_Key_t *key);

OS_Error_t TPM_Keystore_storeKey(const TPM_Keystore_Handle_t *ctx,
                                 size_t handle, TPM_Crypto_Key_t *key);

#define IF_TPM_KEYSTORE_ASSIGN(_rpc_, _port_)         \
{                                                   \
  .loadKey            = _rpc_ ## _loadKey,          \
  .storeKey           = _rpc_ ## _storeKey,         \
  .dataport           = OS_DATAPORT_ASSIGN(_port_)  \
}

#endif /* _TPM_KEYSTORE_H */