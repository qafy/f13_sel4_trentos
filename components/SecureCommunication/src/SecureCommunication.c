/*
 * Secure Communication component
 *
 * Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
 */
#include "SecureCommunication.h"

static OS_Error_t
encryptBuffer(void *input, size_t inputLen, void *output, size_t *outputLen)
{
    OS_Error_t res;
    
#ifdef USE_HW_TPM

    (void)res;

#else

    OS_CryptoCipher_Handle_t hCipher;
    res = OS_CryptoCipher_init(&hCipher, hCrypto, hKeySrvPub, OS_CryptoCipher_ALG_RSA_OAEP_ENC, NULL, 0);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while init code %d", res);
        return res;
    }

    res = OS_CryptoCipher_process(hCipher, input, inputLen, output, outputLen);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while proccess code %d", res);
        return res;
    }
    OS_CryptoCipher_free(hCipher);

#endif /* USE_HW_TPM */

    return OS_SUCCESS;
}

static OS_Error_t
decryptBuffer(void *input, size_t inputLen, void *output, size_t *outputLen)
{
    OS_Error_t res;

#ifdef USE_HW_TPM

    (void)res;

#else

    OS_CryptoCipher_Handle_t hCipher;
    res = OS_CryptoCipher_init(&hCipher, hCrypto, hKeyClntPrvt, OS_CryptoCipher_ALG_RSA_OAEP_DEC, NULL, 0);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while init 2 code %d", res);
        return res;
    }

    res = OS_CryptoCipher_process(hCipher, input, inputLen, output, outputLen);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while proccess 2 code %d", res);
        return res;
    }
    OS_CryptoCipher_free(hCipher);

#endif /* USE_HW_TPM */

    return OS_SUCCESS;
}

__attribute__((unused)) static OS_Error_t
generateKeys()
{
    OS_Error_t res;

#ifdef USE_HW_TPM

    (void)res;

#else

    OS_CryptoKey_Data_t dataClntPub;
    // Filesystem init
    OS_FileSystem_Handle_t hFs;
    OS_FileSystemFile_Handle_t hKeyFile;
    OS_FileSystem_init(&hFs, &cfg);
    OS_FileSystem_mount(hFs);

    size_t outputLen = 4096;
    void *outputBuf = malloc(outputLen);
    if (!outputBuf)
    {
        Debug_LOG_ERROR("Can not allocate memory");
        return OS_ERROR_GENERIC;
    }

    Debug_LOG_INFO("Generating new keypair");

    // generate keypair
    res = OS_CryptoKey_generate(&hKeyClntPrvt, hCrypto, &rsa2048prvt);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Failed to generate clnt_prvt key, err %d", res);
        return res;
    }

    OS_CryptoKey_Attrib_t attr = {
        .flags = OS_CryptoKey_FLAG_NONE,
        .keepLocal = true,
    };
    res = OS_CryptoKey_makePublic(&hKeyClntPub, hCrypto, hKeyClntPrvt, &attr);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Failed to generate clnt_pub key, err %d", res);
        return res;
    }

    // Export Keys
    res = OS_CryptoKey_export(hKeyClntPrvt, &dataClntPrvt);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Failed to export clnt_prvt key, err %d", res);
        return res;
    }
    OS_CryptoKey_free(hKeyClntPrvt);

    res = OS_CryptoKey_export(hKeyClntPub, &dataClntPub);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Failed to export clnt_pub key, err %d", res);
        return res;
    }
    OS_CryptoKey_free(hKeyClntPub);

    // convert clnt_prvt to PEM
    mbedtls_pk_context pkcontext;
    mbedtls_pk_init(&pkcontext);
    mbedtls_pk_setup(&pkcontext, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));

    mbedtls_rsa_context *rsacontext = mbedtls_pk_rsa(pkcontext);
    mbedtls_rsa_init(rsacontext, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
    res = mbedtls_rsa_import_raw(rsacontext,
                                 dataClntPrvt.data.rsa.prv.nBytes,
                                 dataClntPrvt.data.rsa.prv.nLen,
                                 dataClntPrvt.data.rsa.prv.pBytes,
                                 dataClntPrvt.data.rsa.prv.pLen,
                                 dataClntPrvt.data.rsa.prv.qBytes,
                                 dataClntPrvt.data.rsa.prv.qLen,
                                 dataClntPrvt.data.rsa.prv.dBytes,
                                 dataClntPrvt.data.rsa.prv.dLen,
                                 dataClntPrvt.data.rsa.pub.eBytes,
                                 dataClntPrvt.data.rsa.pub.eLen);
    if (res)
    {
        Debug_LOG_ERROR("Failed to import into mbedtls, err %d", res);
        return OS_ERROR_GENERIC;
    }

    res = mbedtls_pk_write_key_pem(&pkcontext, outputBuf, outputLen);
    if (res)
    {
        Debug_LOG_ERROR("Failed to convert to PEM, err %d", res);
        return OS_ERROR_GENERIC;
    }
    Debug_LOG_INFO("clnt_prvt length %d: %s", strlen(outputBuf), (char *)outputBuf);

    // write PEM to Filesystem
    res = OS_FileSystemFile_open(hFs, &hKeyFile, "clnt_prvt.pem",
                                 OS_FileSystem_OpenMode_RDWR, OS_FileSystem_OpenFlags_CREATE);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not open clnt_prvt.pem, err %d", res);
        return res;
    }
    res = OS_FileSystemFile_write(hFs, hKeyFile, 0, strlen(outputBuf) + 1, outputBuf);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not write clnt_prvt.pem, err %d", res);
        return res;
    }
    OS_FileSystemFile_close(hFs, hKeyFile);
    mbedtls_pk_free(&pkcontext);

    // convert public key to PEM
    mbedtls_pk_init(&pkcontext);
    mbedtls_pk_setup(&pkcontext, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));

    rsacontext = mbedtls_pk_rsa(pkcontext);
    mbedtls_rsa_import_raw(rsacontext,
                           dataClntPub.data.rsa.pub.nBytes,
                           dataClntPub.data.rsa.pub.nLen,
                           NULL, 0, NULL, 0, NULL, 0,
                           dataClntPub.data.rsa.pub.eBytes,
                           dataClntPub.data.rsa.pub.eLen);
    if (res)
    {
        Debug_LOG_ERROR("Failed to import into mbedtls, err %d", res);
        return OS_ERROR_GENERIC;
    }

    res = mbedtls_pk_write_pubkey_pem(&pkcontext, outputBuf, outputLen);
    if (res)
    {
        Debug_LOG_ERROR("Failed to convert to PEM, err %d", res);
        return OS_ERROR_GENERIC;
    }

    Debug_LOG_INFO("clnt_pub: %s", (char *)outputBuf);

    // write PEM to Filesystem
    OS_FileSystemFile_open(hFs, &hKeyFile, "clnt_pub.pem",
                           OS_FileSystem_OpenMode_RDWR, OS_FileSystem_OpenFlags_CREATE);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not open clnt_pub.pem, err %d", res);
        return res;
    }
    OS_FileSystemFile_write(hFs, hKeyFile, 0, strlen(outputBuf) + 1, outputBuf);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not write clnt_pub.pem, err %d", res);
        return res;
    }
    OS_FileSystemFile_close(hFs, hKeyFile);
    mbedtls_pk_free(&pkcontext);

    free(outputBuf);
    OS_FileSystem_unmount(hFs);
    OS_FileSystem_free(hFs);

#endif /* USE_HW_TPM */

    return OS_SUCCESS;
}

#ifndef USE_HW_TPM

__attribute__((unused)) static OS_Error_t
loadKeysFromFilesystem(char **pub, char **prv)
{
    Debug_LOG_INFO("Reading key from filesystem");

    // Filesystem init
    OS_Error_t res;
    OS_FileSystem_Handle_t hFs;
    OS_FileSystem_init(&hFs, &cfg);
    OS_FileSystem_mount(hFs);

    OS_FileSystemFile_Handle_t hKeyFile;
    off_t fileSize = 4096;
    res = OS_FileSystemFile_getSize(hFs, "srv_pub.pem", &fileSize);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not get size from file, err %d", res);
        return res;
    }

    void *buf = malloc(fileSize);
    if (!buf)
    {
        Debug_LOG_ERROR("Can not allocate Memory");
        return OS_ERROR_GENERIC;
    }

    res = OS_FileSystemFile_open(hFs, &hKeyFile, "srv_pub.pem",
                                 OS_FileSystem_OpenMode_RDONLY, OS_FileSystem_OpenFlags_NONE);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while opning clnt_prvt, err %d", res);
        return res;
    }

    res = OS_FileSystemFile_read(hFs, hKeyFile, 0, fileSize, buf);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while reading Private Key from File system, err %d", res);
        return res;
    }
    OS_FileSystemFile_close(hFs, hKeyFile);

    *pub = buf;

    res = OS_FileSystemFile_getSize(hFs, "clnt_prvt.pem", &fileSize);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not get size from file, err %d", res);
        return res;
    }
    buf = malloc(fileSize);
    if (!buf)
    {
        Debug_LOG_ERROR("Can not allocate Memory");
        return OS_ERROR_GENERIC;
    }
    res = OS_FileSystemFile_open(hFs, &hKeyFile, "clnt_prvt.pem",
                                 OS_FileSystem_OpenMode_RDONLY, OS_FileSystem_OpenFlags_NONE);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while opning clnt_prvt, err %d", res);
        return res;
    }
    res = OS_FileSystemFile_read(hFs, hKeyFile, 0, fileSize, buf);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while reading Private Key from File system, err %d", res);
        return res;
    }
    OS_FileSystemFile_close(hFs, hKeyFile);

    *prv = buf;

    OS_FileSystem_unmount(hFs);
    OS_FileSystem_free(hFs);

    return OS_SUCCESS;
}

#endif /* !USE_HW_TPM */

static OS_Error_t
loadKeys()
{
#ifdef USE_HW_TPM
#else
    size_t len;
    OS_Error_t res;
    char *publicKey, *privateKey;

#if !LOAD_KEYS_FROM_FILESYSTEM
    publicKey = SERVER_PUBLIC_KEY;
    privateKey = CLIENT_PRIVATE_KEY;
#else
    loadKeysFromFilesystem(&publicKey, &privateKey);
#endif

    Debug_LOG_INFO("Private key: %s", privateKey);
    Debug_LOG_INFO("Public key: %s", publicKey);

    // load public server key from PEM
    mbedtls_pk_context srvPubPem;
    mbedtls_pk_init(&srvPubPem);
    res = mbedtls_pk_parse_public_key(&srvPubPem,
                                      (const unsigned char *)publicKey,
                                      strlen(publicKey) + 1);
    if (res)
    {
        Debug_LOG_ERROR("Can not parse key, err %d", res);
        return res;
    }

    mbedtls_rsa_context *srvPubRsa = mbedtls_pk_rsa(srvPubPem);
    res = mbedtls_rsa_check_pubkey(srvPubRsa);
    if (res)
    {
        Debug_LOG_ERROR("RSA public key has wrong format, err %d", res);
        return OS_ERROR_GENERIC;
    }

    dataSrvPub = (OS_CryptoKey_Data_t){
        .type = OS_CryptoKey_TYPE_RSA_PUB,
        .attribs = {
            .flags = OS_CryptoKey_FLAG_NONE,
            .keepLocal = true,
        },
    };

    res = mbedtls_rsa_export_raw(srvPubRsa,
                                 dataSrvPub.data.rsa.pub.nBytes, 256,
                                 NULL, 0, NULL, 0, NULL, 0,
                                 dataSrvPub.data.rsa.pub.eBytes, 3);

    if (res)
    {
        Debug_LOG_ERROR("Can not export key, err %d", res);
        return OS_ERROR_GENERIC;
    }

    dataSrvPub.data.rsa.pub.eLen = 3;
    dataSrvPub.data.rsa.pub.nLen = 256;

    // load private client key from PEM
    mbedtls_pk_context clntPrvtPem;
    mbedtls_pk_init(&clntPrvtPem);

    res = mbedtls_pk_parse_key(&clntPrvtPem,
                               (const unsigned char *)privateKey,
                               strlen(privateKey) + 1, NULL, 0);
    if (res)
    {
        Debug_LOG_ERROR("Can not parse private key, err %d", res);
        return OS_ERROR_GENERIC;
    }

    mbedtls_rsa_context *clntPrvtRsa = mbedtls_pk_rsa(clntPrvtPem);
    res = mbedtls_rsa_check_pubkey(clntPrvtRsa);
    if (res)
    {
        Debug_LOG_ERROR("RSA private key has wrong format, err %d", res);
        return OS_ERROR_GENERIC;
    }

    dataClntPrvt = (OS_CryptoKey_Data_t){
        .type = OS_CryptoKey_TYPE_RSA_PRV,
        .attribs = {
            .flags = OS_CryptoKey_FLAG_NONE,
            .keepLocal = true,
        },
    };

    res = mbedtls_rsa_export_raw(clntPrvtRsa,
                                 dataClntPrvt.data.rsa.prv.nBytes, 256,
                                 dataClntPrvt.data.rsa.prv.pBytes, 128,
                                 dataClntPrvt.data.rsa.prv.qBytes, 128,
                                 dataClntPrvt.data.rsa.prv.dBytes, 255,
                                 dataClntPrvt.data.rsa.prv.eBytes, 3);
    if (res)
    {
        Debug_LOG_ERROR("Can not export private key, err %d", res);
        return OS_ERROR_GENERIC;
    }

    dataClntPrvt.data.rsa.prv.nLen = 256;
    dataClntPrvt.data.rsa.prv.pLen = 128;
    dataClntPrvt.data.rsa.prv.qLen = 128;
    dataClntPrvt.data.rsa.prv.dLen = 255;
    dataClntPrvt.data.rsa.prv.eLen = 3;

#if LOAD_KEYS_FROM_FILESYSTEM
    free(publicKey);
    free(privateKey);
#endif

    // Keystore init
#define KEYSTORE_NUM_ELEMENTS 10
#define KEYSTORE_RAM_BUF_SIZE OS_KeystoreRamFV_SIZE_OF_BUFFER(KEYSTORE_NUM_ELEMENTS)

    void *keystoreBuffer = malloc(KEYSTORE_RAM_BUF_SIZE);
    if (!keystoreBuffer)
    {
        Debug_LOG_ERROR("Can not allocate enough memory for keystoreBuffer");
        return OS_ERROR_GENERIC;
    }
    OS_Keystore_Handle_t hKeys;
    res = OS_KeystoreRamFV_init(&hKeys, keystoreBuffer, KEYSTORE_RAM_BUF_SIZE);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not initialize keystoreRamFv, err %d", res);
        return res;
    }
    // store server key in keystore
    res = OS_Keystore_storeKey(hKeys, "srv_pub", &dataSrvPub, sizeof(dataSrvPub));
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Failed to store srv_pub key, err %d", res);
        return res;
    }

    // store client key in keystore
    res = OS_Keystore_storeKey(hKeys, "clnt_prvt", &dataClntPrvt, sizeof(dataClntPrvt));
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Failed to store clnt_prvt key, err %d", res);
        return res;
    }

    len = sizeof(OS_CryptoKey_Data_t);
    // load server key from keystore
    res = OS_Keystore_loadKey(hKeys, "srv_pub", &dataSrvPub, &len);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not load srv_pub, err %d", res);
        return res;
    }

    len = sizeof(OS_CryptoKey_Data_t);
    // load client key from keystore
    res = OS_Keystore_loadKey(hKeys, "clnt_prvt", &dataClntPrvt, &len);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not load clnt_prvt, err %d", res);
        return res;
    }
    OS_Keystore_free(hKeys);

    // import keys from key data
    res = OS_CryptoKey_import(&hKeySrvPub, hCrypto, &dataSrvPub);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Can not import srv_pub, err %d", res);
        return res;
    }

    res = OS_CryptoKey_import(&hKeyClntPrvt, hCrypto, &dataClntPrvt);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Error while importing Private Key, err %d", res);
        return res;
    }

#endif /* USE_HW_TPM */

    return OS_SUCCESS;
}

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

int run()
{
    OS_Error_t res;
    Debug_LOG_INFO("Initializing Secure Communication component");

#ifndef USE_HW_TPM
    // Crypto init
    res = OS_Crypto_init(&hCrypto, &cryptoCfg);
    if (res != OS_SUCCESS)
    {
        initState = FATAL_ERROR;
        Debug_LOG_ERROR("Error while initializing Crypto API");
        return res;
    }
#endif

#if GENERATE_KEYS
    res = generateKeys();
    if (res != OS_SUCCESS)
    {
        initState = FATAL_ERROR;
        return res;
    }

#endif
    res = loadKeys();
    if (res != OS_SUCCESS)
    {
        initState = FATAL_ERROR;
        return res;
    }
    initState = RUNNING;

    // Network init
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
            secureCommunication_8_event_notify_emit,
        };

    for (;;)
    {

        OS_Socket_wait(&networkStackCtx);
        for (int i = 0; i < MAX_CLIENTS_NUM; i++)
            notifications[i]();

        seL4_Yield();
    }
    return 0;
}