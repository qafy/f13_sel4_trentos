/*
 * Secure Communication component
 *
 * Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
 */

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

seL4_Word
secureCommunication_rpc_get_sender_id(void);

//----------------------------------------------------------------------
// Network
//----------------------------------------------------------------------

static const if_OS_Socket_t networkStackCtx =
    IF_OS_SOCKET_ASSIGN(networkStack);

//----------------------------------------------------------------------
// Crypto
//----------------------------------------------------------------------

static OS_Crypto_Config_t cryptoCfg = {
    .mode = OS_Crypto_MODE_LIBRARY,
    .entropy = IF_OS_ENTROPY_ASSIGN(
        entropy_rpc,
        entropy_port)};

//----------------------------------------------------------------------

OS_Error_t
secureCommunication_rpc_socket_create(
    const int domain,
    const int socket_type,
    int *const pHandle)
{
    Debug_LOG_INFO("Socket create");
    OS_Socket_Handle_t handle;
    OS_Error_t res = OS_Socket_create(&networkStackCtx, &handle, domain, socket_type);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while creating Socket");
        return res;
    }
    *pHandle = handle.handleID;
    return res;
}

OS_Error_t
secureCommunication_rpc_socket_close(
    const int handle)
{
    Debug_LOG_INFO("Socket close");
    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};
    return OS_Socket_close(handle2);
}

OS_Error_t
secureCommunication_rpc_socket_connect(
    const int handle,
    const OS_Socket_Addr_t *const dstAddr)
{
    Debug_LOG_INFO("Socket connect");
    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};
    return OS_Socket_connect(handle2, dstAddr);
}

OS_Error_t
secureCommunication_rpc_socket_bind(
    const int handle,
    const OS_Socket_Addr_t *const localAddr)
{
    Debug_LOG_INFO("Socket bind");
    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};
    return OS_Socket_bind(handle2, localAddr);
}

OS_Error_t
secureCommunication_rpc_socket_listen(
    const int handle,
    const int backlog)
{
    Debug_LOG_INFO("Socket listen");
    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};
    return OS_Socket_listen(handle2, backlog);
}

OS_Error_t
secureCommunication_rpc_socket_accept(
    const int handle,
    int *const pClient_handle,
    OS_Socket_Addr_t *const srcAddr)
{
    Debug_LOG_INFO("Socket accept");
    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};
    OS_Socket_Handle_t pClient_handle2;
    OS_Error_t res = OS_Socket_accept(handle2, &pClient_handle2, srcAddr);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while accepting");
    }
    *pClient_handle = pClient_handle2.handleID;
    return res;
}

OS_Error_t
secureCommunication_rpc_socket_write(
    const int handle,
    size_t *const pLen)
{

    Debug_LOG_INFO("Socket write");
    uint8_t *buf =
        secureCommunication_rpc_buf(secureCommunication_rpc_get_sender_id());
    //size_t size =
    //    secureCommunication_rpc_buf_size(secureCommunication_rpc_get_sender_id());

    //Debug_LOG_INFO("Socket with buf size %d wrote %s", size, buf);

    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};

    size_t length = *pLen;
    return OS_Socket_write(handle2, (void *)buf, length, pLen);
}

OS_Error_t
secureCommunication_rpc_socket_read(
    const int handle,
    size_t *const pLen)
{
    Debug_LOG_INFO("Socket read");

    uint8_t *buf =
        secureCommunication_rpc_buf(secureCommunication_rpc_get_sender_id());
    //size_t size =
    //    secureCommunication_rpc_buf_size(secureCommunication_rpc_get_sender_id());

    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};

    size_t length = *pLen;
    OS_Error_t res = OS_Socket_read(handle2, (void *)buf, length, pLen);

    //Debug_LOG_INFO("Socket with buf size %d read %s", size, buf);

    return res;
}

OS_Error_t
secureCommunication_rpc_socket_sendto(
    const int handle,
    size_t *const pLen,
    const OS_Socket_Addr_t *const dstAddr)
{
    Debug_LOG_INFO("Socket sendto");

    uint8_t *buf =
        secureCommunication_rpc_buf(secureCommunication_rpc_get_sender_id());
    //size_t size =
    //    secureCommunication_rpc_buf_size(secureCommunication_rpc_get_sender_id());

    //Debug_LOG_INFO("Socket with buf size %d wrote %s", size, buf);

    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};

    size_t length = *pLen;
    return OS_Socket_sendto(handle2, (void *)buf, length, pLen, dstAddr);
}

OS_Error_t
secureCommunication_rpc_socket_recvfrom(
    const int handle,
    size_t *const pLen,
    OS_Socket_Addr_t *const srcAddr)
{
    Debug_LOG_INFO("Socket recvfrom");

    uint8_t *buf =
        secureCommunication_rpc_buf(secureCommunication_rpc_get_sender_id());
    //size_t size =
    //    secureCommunication_rpc_buf_size(secureCommunication_rpc_get_sender_id());

    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};

    size_t length = *pLen;
    OS_Error_t res = OS_Socket_recvfrom(handle2, (void *)buf, length, pLen, srcAddr);

    //Debug_LOG_INFO("Socket with buf size %d read %s", size, buf);

    return res;
}

OS_NetworkStack_State_t
secureCommunication_rpc_socket_getStatus(
    void)
{
    // Debug_LOG_INFO("Socket getstatus");
    return OS_Socket_getStatus(&networkStackCtx);
}

OS_Error_t
secureCommunication_rpc_socket_getPendingEvents(
    size_t maxRequestedSize,
    int *pNumberOfEvents)
{
    Debug_LOG_INFO("Socket getPendingEvents");

    uint8_t *buf =
        secureCommunication_rpc_buf(secureCommunication_rpc_get_sender_id());
    size_t size =
        secureCommunication_rpc_buf_size(secureCommunication_rpc_get_sender_id());

    return OS_Socket_getPendingEvents(
        &networkStackCtx, (void *const)buf, size, pNumberOfEvents);
}

int run()
{
    Debug_LOG_INFO("Initializing Secure Communication component");

#define MAX_CLIENTS_NUM 8
    static const event_notify_func_t notifications[MAX_CLIENTS_NUM] =
        {
            secureCommunication_1_event_notify_emit,
            secureCommunication_2_event_notify_emit,
            secureCommunication_3_event_notify_emit,
            secureCommunication_4_event_notify_emit,
            secureCommunication_5_event_notify_emit,
            secureCommunication_6_event_notify_emit,
            secureCommunication_7_event_notify_emit,
            secureCommunication_8_event_notify_emit};

    OS_Crypto_Handle_t hCrypto;
    OS_Error_t res = OS_Crypto_init(&hCrypto, &cryptoCfg);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while initializing Crypto API");
    }

    for (;;)
    {

        OS_Socket_wait(&networkStackCtx);
        for (int i = 0; i < MAX_CLIENTS_NUM; i++)
            notifications[i]();

        seL4_Yield();
    }
    return 0;
}