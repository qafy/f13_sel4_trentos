/*
 * Secure Communication component
 *
 * Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "OS_Socket.h"
#include "lib_debug/Debug.h"
#include "TimeServer.h"

#include "mbedtls/pk.h"
#include "mbedtls/pkwrite.h"
#include "mbedtls/rsa.h"

#include <camkes.h>

#ifdef USE_HW_TPM

#include "TPM_Crypto.h"
#include "TPM_Keystore.h"

#else

#include "OS_Crypto.h"
#include "OS_KeystoreRamFV.h"
#include "OS_FileSystem.h"

#endif

#include "SecureCommunication.h"

#define GENERATE_KEYS 0

//----------------------------------------------------------------------
// Network
//----------------------------------------------------------------------

static const if_OS_Socket_t networkStackCtx =
    IF_OS_SOCKET_ASSIGN(networkStack);

//----------------------------------------------------------------------
// Timeserver
//----------------------------------------------------------------------

// timer is not used in SW build without BENCHMARK
__attribute__((unused)) static const if_OS_Timer_t timer =
    IF_OS_TIMER_ASSIGN(
        timer_rpc,
        timer_notify);

#ifdef USE_HW_TPM

//----------------------------------------------------------------------
// TPM Crypto
//----------------------------------------------------------------------

static const TPM_Crypto_Handle_t cryptoCtx =
    IF_TPM_CRYPTO_ASSIGN(crypto_rpc, crypto_port);

//----------------------------------------------------------------------
// TPM Keystore
//----------------------------------------------------------------------

static const TPM_Keystore_Handle_t keystoreCtx =
    IF_TPM_KEYSTORE_ASSIGN(keystore_rpc, keystore_port);

// Client Keys
static TPM_Crypto_Key_t hKeyClnt;
// Server Key
static TPM_Crypto_Key_t hKeySrvPub;

#else

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

// Server Key Data
static OS_CryptoKey_Data_t dataSrvPub;
// Server Key Data
__attribute__((unused)) static OS_CryptoKey_Data_t dataClntPub;
// Client Key Data
static OS_CryptoKey_Data_t dataClntPrvt;

#endif /* USE_HW_TPM */

OS_NetworkStack_State_t initState = UNINITIALIZED;

OS_NetworkStack_State_t getInitState() {
    return initState;
}

OS_Error_t
encryptBuffer(void *input, size_t inputLen, void *output, size_t *outputLen)
{
    OS_Error_t res;

#ifdef USE_HW_TPM

    #ifdef BENCHMARK
    res = TPM_Crypto_encrypt(&cryptoCtx, &hKeyClnt, input, inputLen, output, outputLen);
    #else
    res = TPM_Crypto_encrypt(&cryptoCtx, &hKeySrvPub, input, inputLen, output, outputLen);
    #endif
    if (res != OS_SUCCESS) {
        Debug_LOG_ERROR("Error while proccess code %d", res);
        return res;
    }

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

OS_Error_t
decryptBuffer(void *input, size_t inputLen, void *output, size_t *outputLen)
{
    OS_Error_t res;

#ifdef USE_HW_TPM

    res = TPM_Crypto_decrypt(&cryptoCtx, &hKeyClnt, input, inputLen, output, outputLen);
    if (res != OS_SUCCESS) {
        Debug_LOG_ERROR("Error while proccess 2 code %d", res);
        return res;
    }

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

    res = TPM_Crypto_generateKey(&cryptoCtx, &hKeyClnt);
    if (res != OS_SUCCESS)
    {
        Debug_LOG_ERROR("Failed to generate client key\n");
        return res;
    }

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

#ifdef USE_HW_TPM

static OS_Error_t
importServerKeys()
{
    OS_Error_t res;
    int rc;
    char publicKey[] = SERVER_PUBLIC_KEY;
    unsigned char raw[TPM_CRYPO_PUBLIC_RAW_SIZE] = {0};
    mbedtls_pk_context srvPubPem;
    mbedtls_rsa_context *srvPubRsa;

    // load public client key from PEM
    mbedtls_pk_init(&srvPubPem);

    res = mbedtls_pk_parse_public_key(&srvPubPem,
                               (const unsigned char *)publicKey,
                               strlen(publicKey) + 1);
    if (res)
    {
        Debug_LOG_ERROR("Can not parse server key, err %d", res);
        return res;
    }

    srvPubRsa = mbedtls_pk_rsa(srvPubPem);

    res = mbedtls_rsa_export_raw(srvPubRsa,
                                 raw, 256,
                                 NULL, 0,
                                 NULL, 0,
                                 NULL, 0,
                                 raw + 256, 4);
    if (res)
    {
        Debug_LOG_ERROR("Can not export server key, err %d", res);
        return res;
    }

    rc = TPM_Crypto_importPublic(&cryptoCtx, &hKeySrvPub, raw);
    if (rc) {
        Debug_LOG_ERROR("TPM: Could not import server key %d", rc);
        return res;
    }

    return OS_SUCCESS;
}

static OS_Error_t
importClientKeys()
{
    OS_Error_t res;
    int rc;
    char privateKey[] = CLIENT_PRIVATE_KEY;
    unsigned char raw[TPM_CRYPO_PRIVATE_RAW_SIZE] = {0};
    mbedtls_pk_context clntPrvtPem;
    mbedtls_rsa_context *clntPrvtRsa;

    // load private client key from PEM
    mbedtls_pk_init(&clntPrvtPem);

    res = mbedtls_pk_parse_key(&clntPrvtPem,
                               (const unsigned char *)privateKey,
                               strlen(privateKey) + 1, NULL, 0);
    if (res)
    {
        Debug_LOG_ERROR("Can not parse client key, err %d", res);
        return OS_ERROR_NOT_FOUND;
    }

    clntPrvtRsa = mbedtls_pk_rsa(clntPrvtPem);

    res = mbedtls_rsa_export_raw(clntPrvtRsa,
                                 raw, 256,
                                 raw + 256, 128,
                                 raw + 256 + 128, 128,
                                 raw + 256 + 128 + 128, 256,
                                 raw + 256 + 128 + 128 + 256, 4);
    if (res)
    {
        Debug_LOG_ERROR("Can not export client key, err %d", res);
        return res;
    }

    rc = TPM_Crypto_importPrivate(&cryptoCtx, &hKeyClnt, raw);
    if (rc) {
        Debug_LOG_ERROR("TPM: Could not import client key %d\n", rc);
        return res;
    }

    return OS_SUCCESS;
}

#endif /* USE_HW_TPM */

static OS_Error_t
loadKeys()
{
    OS_Error_t res;

#ifdef USE_HW_TPM

    res = TPM_Keystore_loadKey(&keystoreCtx, TPM_CLIENT_KEY_HANDLE, &hKeyClnt);
    if (res == OS_ERROR_INVALID_HANDLE) {
        Debug_LOG_INFO("Client private key was not found in TPM keystore: Loading key from system_config...");

        res = importClientKeys();
        if (res == OS_ERROR_NOT_FOUND) {
            Debug_LOG_INFO("Client private key was not found in TPM keystore or system_config: Generating new key...");

            res = generateKeys();
            if (res != OS_SUCCESS) {
                Debug_LOG_ERROR("Could not generate client key %d", res);
                return res;
            }
        }
        else if (res != OS_SUCCESS) {
            return res;
        }

        Debug_LOG_INFO("Client private key loaded! Storing in TPM keystore for future use...");

        res = TPM_Keystore_storeKey(&keystoreCtx, TPM_CLIENT_KEY_HANDLE, &hKeyClnt);
        if (res != OS_SUCCESS) {
            Debug_LOG_ERROR("Could not store client key %d\n", res);
            return res;
        }
    }
    else if (res != OS_SUCCESS) {
        Debug_LOG_ERROR("Can not load clnt_prvt, err %d\n", res);
        return res;
    }
    Debug_LOG_INFO("Client private key loaded!");

    res = importServerKeys();
    if (res != OS_SUCCESS) {
        Debug_LOG_ERROR("Can not load srv_pub, err %d\n", res);
        return res;
    }
    Debug_LOG_INFO("Server public key loaded!");

    /* Print client public key */

    unsigned char *pem = malloc(2000);
    size_t* pemSz = malloc(sizeof(pemSz));

    TPM_Crypto_exportPublicPem(&cryptoCtx, &hKeyClnt, pem, pemSz);

    Debug_LOG_INFO("Client public key:\n%s", pem);

    free(pem);
    free(pemSz);

#else

    size_t len;
    char *publicKey, *privateKey;

#ifndef LOAD_KEYS_FROM_FILESYSTEM
    publicKey = SERVER_PUBLIC_KEY;
#ifdef BENCHMARK
    publicKey = CLIENT_PUBLIC_KEY;
#endif
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

#ifdef LOAD_KEYS_FROM_FILESYSTEM
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

#ifdef BENCHMARK
OS_Error_t benchmark() {
    OS_Error_t res;
    uint64_t start, fin;
    char msg[] = "Das ist ein test";
    char* cipher = malloc(256);
    size_t cipher_size;
    char* plain = malloc(256);
    size_t plain_size;

    TimeServer_getTime(&timer, TimeServer_PRECISION_MSEC, &start);

    for (int i = 0; i < 100; i++) {
        encryptBuffer(msg, sizeof(msg), cipher, &cipher_size);
    }

    TimeServer_getTime(&timer, TimeServer_PRECISION_MSEC, &fin);
    Debug_LOG_INFO("Encryption took %lu milliseconds | per iteration : %f", (long unsigned int)(fin - start), (fin - start) / 100. );

    // ----------------------------------------------------------------------------------

    TimeServer_getTime(&timer, TimeServer_PRECISION_MSEC, &start);

    for (int i = 0; i < 10; i++) {
        decryptBuffer(cipher, cipher_size, plain, &plain_size);
    }
    printf("plain: %s\n", plain);

    TimeServer_getTime(&timer, TimeServer_PRECISION_MSEC, &fin);
    Debug_LOG_INFO("Decryption took %lu milliseconds | per iteration : %f", (long unsigned int)(fin - start), (fin - start) / 10. );

    free(plain);
    free(cipher);

    // ----------------------------------------------------------------------------------

#ifdef USE_HW_TPM

    TPM_Crypto_Key_t testKey;

    TimeServer_getTime(&timer, TimeServer_PRECISION_MSEC, &start);

    for (int i = 0; i < 2; i++) {
        TPM_Crypto_generateKey(&cryptoCtx, &testKey);
    }

    TimeServer_getTime(&timer, TimeServer_PRECISION_MSEC, &fin);
    Debug_LOG_INFO("Key generation took %lu milliseconds | per iteration : %f", (long unsigned int)(fin - start), (fin - start) / 2. );

    (void) res;

#else

    TimeServer_getTime(&timer, TimeServer_PRECISION_MSEC, &start);

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

    TimeServer_getTime(&timer, TimeServer_PRECISION_MSEC, &fin);
    Debug_LOG_INFO("Key generation took %lu milliseconds | per iteration : %f", (long unsigned int)(fin - start), (fin - start) / 1. );

#endif

    return OS_SUCCESS;
}
#endif

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

#ifdef BENCHMARK
    return benchmark();
#endif

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