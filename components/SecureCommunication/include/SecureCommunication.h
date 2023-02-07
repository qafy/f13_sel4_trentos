/*
 * Secure Communication component
 *
 * Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
 */

#pragma once

#include "OS_Error.h"
#include "OS_Socket.h"

#define MAX_CLIENTS_NUM 8
#define KEYSTORE_NUM_ELEMENTS 10

#define GENERATE_KEYS 0

seL4_Word secureCommunication_rpc_get_sender_id(void);

OS_NetworkStack_State_t getInitState();

OS_Error_t encryptBuffer(void *input, size_t inputLen, void *output, size_t *outputLen);

OS_Error_t decryptBuffer(void *input, size_t inputLen, void *output, size_t *outputLen);
