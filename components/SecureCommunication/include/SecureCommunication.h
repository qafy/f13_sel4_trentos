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

#include "OS_Crypto.h"
#include "OS_KeystoreFile.h"

#include "lib_debug/Debug.h"

#include <camkes.h>

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
