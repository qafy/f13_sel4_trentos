/*
 * Secure Communication component
 *
 * Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "OS_Socket.h"
#include "interfaces/if_OS_Socket.h"
#include "network/OS_SocketTypes.h"

#include "OS_Dataport.h"
#include "TimeServer.h"
#include "lib_debug/Debug.h"

#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"

#include <camkes.h>

#ifdef USE_HW_TPM
#else

#include "OS_Crypto.h"
#include "OS_KeystoreRamFV.h"
#include "OS_FileSystem.h"

#endif

#define LOAD_KEYS_FROM_FILESYSTEM 1
#define GENERATE_KEYS 0
#define BENCHMARK 0

seL4_Word
secureCommunication_rpc_get_sender_id(void);

//----------------------------------------------------------------------
// Network
//----------------------------------------------------------------------

static const if_OS_Socket_t networkStackCtx =
    IF_OS_SOCKET_ASSIGN(networkStack);

#ifdef USE_HW_TPM

#else
//----------------------------------------------------------------------
// Crypto
//----------------------------------------------------------------------

static const OS_Crypto_Config_t cryptoCfg = {
    .mode = OS_Crypto_MODE_LIBRARY,
    .entropy = IF_OS_ENTROPY_ASSIGN(
        entropy_rpc,
        entropy_port),
};

// key spec for key generation
static const OS_CryptoKey_Spec_t rsa2048prvt = {
    .type = OS_CryptoKey_SPECTYPE_BITS,
    .key = {
        .type = OS_CryptoKey_TYPE_RSA_PRV,
        .params.bits = 2048,
        .attribs = {.flags = OS_CryptoKey_FLAG_NONE,
                    .keepLocal = true},
    },
};

// Crypto handle
static OS_Crypto_Handle_t hCrypto;
// Client Keys
static OS_CryptoKey_Handle_t hKeyClntPrvt, hKeyClntPub;
// Server Key
static OS_CryptoKey_Handle_t hKeySrvPub;

//----------------------------------------------------------------------
// Filesystem
//----------------------------------------------------------------------

static OS_FileSystem_Config_t cfg =
    {
        .type = OS_FileSystem_Type_FATFS,
        .storage = IF_OS_STORAGE_ASSIGN(
            sd_rpc,
            sd_port),
};
//----------------------------------------------------------------------
// Timeserver
//----------------------------------------------------------------------

static const if_OS_Timer_t timer =
    IF_OS_TIMER_ASSIGN(
        timer_rpc,
        timer_notify);

//----------------------------------------------------------------------

// Server Key Data
static OS_CryptoKey_Data_t dataSrvPub;

// Server Key Data
__attribute__((unused)) static OS_CryptoKey_Data_t dataClntPub;

// Client Key Data
static OS_CryptoKey_Data_t dataClntPrvt;

#endif /* USE_HW_TPM */

static OS_NetworkStack_State_t initState = UNINITIALIZED;

// interface methods for OS_Crypto
OS_Error_t
secureCommunication_rpc_socket_create(
    const int domain,
    const int socket_type,
    int *const pHandle);

OS_Error_t
secureCommunication_rpc_socket_close(
    const int handle);

OS_Error_t
secureCommunication_rpc_socket_connect(
    const int handle,
    const OS_Socket_Addr_t *const dstAddr);

OS_Error_t
secureCommunication_rpc_socket_bind(
    const int handle,
    const OS_Socket_Addr_t *const localAddr);

OS_Error_t
secureCommunication_rpc_socket_listen(
    const int handle,
    const int backlog);

OS_Error_t
secureCommunication_rpc_socket_accept(
    const int handle,
    int *const pClient_handle,
    OS_Socket_Addr_t *const srcAddr);

OS_Error_t
secureCommunication_rpc_socket_write(
    const int handle,
    size_t *const pLen);

OS_Error_t
secureCommunication_rpc_socket_read(
    const int handle,
    size_t *const pLen);

OS_Error_t
secureCommunication_rpc_socket_sendto(
    const int handle,
    size_t *const pLen,
    const OS_Socket_Addr_t *const dstAddr);

OS_Error_t
secureCommunication_rpc_socket_recvfrom(
    const int handle,
    size_t *const pLen,
    OS_Socket_Addr_t *const srcAddr);

OS_NetworkStack_State_t
secureCommunication_rpc_socket_getStatus(
    void);

OS_Error_t
secureCommunication_rpc_socket_getPendingEvents(
    size_t maxRequestedSize,
    int *pNumberOfEvents);
