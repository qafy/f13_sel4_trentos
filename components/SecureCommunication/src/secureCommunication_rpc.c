
#include "OS_Socket.h"
#include "lib_debug/Debug.h"

#include <camkes.h>

#include "SecureCommunication.h"

//----------------------------------------------------------------------
// Network
//----------------------------------------------------------------------

static const if_OS_Socket_t networkStackCtx =
    IF_OS_SOCKET_ASSIGN(networkStack);

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
        return res;
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
    OS_Error_t res;
    // for length of encrypted message see this post
    // https://crypto.stackexchange.com/questions/42097/what-is-the-maximum-size-of-the-plaintext-message-for-rsa-oaep/42100#42100
    size_t expectedLen = *pLen > 190 ? 190 : *pLen;

    // get buffers for encryption
    uint8_t *buf =
        secureCommunication_rpc_buf(secureCommunication_rpc_get_sender_id());
    size_t encryptedLen = 256;
    void *encryptedBuf = malloc(encryptedLen);
    if (!encryptedBuf)
    {
        Debug_LOG_ERROR("Unable to allocate memory");
        return OS_ERROR_GENERIC;
    }

    res = encryptBuffer(buf, expectedLen, encryptedBuf, &encryptedLen);
    if (res != OS_SUCCESS)
    {
        free(encryptedBuf);
        return res;
    }

    size_t sentLen = 0;
    // send encrypted data
    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};
    res = OS_Socket_write(handle2, encryptedBuf, encryptedLen, &sentLen);
    if (res != OS_SUCCESS)
    {
        free(encryptedBuf);
        return res;
    }

    if (sentLen != encryptedLen)
    {
        Debug_LOG_ERROR("Sent data is not a full encrypted block");
        free(encryptedBuf);
        return OS_ERROR_GENERIC;
    }

    *pLen = expectedLen;
    free(encryptedBuf);
    return res;
}

OS_Error_t
secureCommunication_rpc_socket_sendto(
    const int handle,
    size_t *const pLen,
    const OS_Socket_Addr_t *const dstAddr)
{
    Debug_LOG_INFO("Socket sendto");
    OS_Error_t res;
    // for length of encrypted message see this post
    // https://crypto.stackexchange.com/questions/42097/what-is-the-maximum-size-of-the-plaintext-message-for-rsa-oaep/42100#42100
    size_t expectedLen = *pLen > 190 ? 190 : *pLen;

    // get buffers for encryption
    uint8_t *buf =
        secureCommunication_rpc_buf(secureCommunication_rpc_get_sender_id());
    size_t encryptedLen = 256;
    void *encryptedBuf = malloc(encryptedLen);
    if (!encryptedBuf)
    {
        Debug_LOG_ERROR("Unable to allocate memory");
        return OS_ERROR_GENERIC;
    }

    res = encryptBuffer(buf, expectedLen, encryptedBuf, &encryptedLen);
    if (res != OS_SUCCESS)
    {
        free(encryptedBuf);
        return res;
    }

    size_t sentLen = 0;
    // send encrypted data
    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};
    res = OS_Socket_sendto(handle2, encryptedBuf, encryptedLen, &sentLen, dstAddr);
    if (res != OS_SUCCESS)
    {
        free(encryptedBuf);
        return res;
    }

    if (sentLen != encryptedLen)
    {
        Debug_LOG_ERROR("Sent data is not a full encrypted block");
        free(encryptedBuf);
        return OS_ERROR_GENERIC;
    }

    *pLen = expectedLen;
    free(encryptedBuf);
    return res;
}

OS_Error_t
secureCommunication_rpc_socket_read(
    const int handle,
    size_t *const pLen)
{

    Debug_LOG_INFO("Socket read");
    OS_Error_t res;

    if (*pLen == 0)
    {
        return OS_SUCCESS;
    }

    // get buffers for decryption
    uint8_t *buf =
        secureCommunication_rpc_buf(secureCommunication_rpc_get_sender_id());
    size_t encryptedLen = 256;
    void *encryptedBuf = malloc(encryptedLen);
    if (!encryptedBuf)
    {
        Debug_LOG_ERROR("Unable to allocate memory");
        return OS_ERROR_GENERIC;
    }

    size_t recvLen = 0;
    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};
    do
    {
        res = OS_Socket_read(handle2, encryptedBuf, 256, &recvLen);
        Debug_LOG_WARNING("OS_Socket_read() reported try again");
        seL4_Yield();
    } while (res == OS_ERROR_TRY_AGAIN);

    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while reading from socket, err %d", res);
        free(encryptedBuf);
        return res;
    }
    if (recvLen != encryptedLen)
    {
        Debug_LOG_ERROR("Sent data is not a full encrypted block");
        free(encryptedBuf);
        return OS_ERROR_GENERIC;
    }
    size_t decryptedLen = secureCommunication_rpc_buf_size(
        secureCommunication_rpc_get_sender_id());
    res = decryptBuffer(encryptedBuf, encryptedLen, buf, &decryptedLen);
    if (res != OS_SUCCESS)
    {
        free(encryptedBuf);
        return res;
    }

    *pLen = decryptedLen;
    free(encryptedBuf);
    return res;
}

OS_Error_t
secureCommunication_rpc_socket_recvfrom(
    const int handle,
    size_t *const pLen,
    OS_Socket_Addr_t *const srcAddr)
{
    Debug_LOG_INFO("Socket recvfrom");
    OS_Error_t res;

    if (*pLen == 0)
    {
        return OS_SUCCESS;
    }

    // get buffers for decryption
    uint8_t *buf =
        secureCommunication_rpc_buf(secureCommunication_rpc_get_sender_id());
    size_t encryptedLen = 256;
    void *encryptedBuf = malloc(encryptedLen);
    if (!encryptedBuf)
    {
        Debug_LOG_ERROR("Unable to allocate memory");
        return OS_ERROR_GENERIC;
    }

    size_t recvLen = 0;
    OS_Socket_Handle_t handle2 = {.ctx = networkStackCtx, .handleID = handle};
    do
    {
        res = OS_Socket_recvfrom(handle2, encryptedBuf, 256, &recvLen, srcAddr);
        Debug_LOG_WARNING("OS_Socket_read() reported try again");
        seL4_Yield();
    } while (res == OS_ERROR_TRY_AGAIN);

    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while reading from socket, err %d", res);
        free(encryptedBuf);
        return res;
    }
    if (recvLen != encryptedLen)
    {
        Debug_LOG_ERROR("Sent data is not a full encrypted block");
        free(encryptedBuf);
        return OS_ERROR_GENERIC;
    }
    size_t decryptedLen = secureCommunication_rpc_buf_size(
        secureCommunication_rpc_get_sender_id());
    res = decryptBuffer(encryptedBuf, encryptedLen, buf, &decryptedLen);
    if (res != OS_SUCCESS)
    {
        free(encryptedBuf);
        return res;
    }

    *pLen = decryptedLen;
    free(encryptedBuf);
    return res;
}

OS_NetworkStack_State_t
secureCommunication_rpc_socket_getStatus(
    void)
{
    OS_NetworkStack_State_t initState = getInitState();

    if (initState == RUNNING)
        return OS_Socket_getStatus(&networkStackCtx);
    else
        return initState;
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
