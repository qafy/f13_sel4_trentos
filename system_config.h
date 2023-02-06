/*
 * OS libraries configurations
 *
 * Copyright (C) 2021, HENSOLDT Cyber GmbH
 * Copyright (C) 2023, Jakob Kukla
 */


#pragma once


//-----------------------------------------------------------------------------
// Debug
//-----------------------------------------------------------------------------
#if !defined(NDEBUG)
#   define Debug_Config_STANDARD_ASSERT
#   define Debug_Config_ASSERT_SELF_PTR
#else
#   define Debug_Config_DISABLE_ASSERT
#   define Debug_Config_NO_ASSERT_SELF_PTR
#endif

#define Debug_Config_LOG_LEVEL                  Debug_LOG_LEVEL_INFO
#define Debug_Config_INCLUDE_LEVEL_IN_MSG
#define Debug_Config_LOG_WITH_FILE_LINE


//-----------------------------------------------------------------------------
// Memory
//-----------------------------------------------------------------------------
#define Memory_Config_USE_STDLIB_ALLOC


//-----------------------------------------------------------------------------
// Network Stack
//-----------------------------------------------------------------------------
#define OS_NETWORK_MAXIMUM_SOCKET_NO 1

#define CFG_TEST_HTTP_SERVER      "10.0.0.10"
#define ETH_ADDR                  "10.0.0.11"
#define ETH_GATEWAY_ADDR          "10.0.0.1"
#define ETH_SUBNET_MASK           "255.255.255.0"
#define EXERCISE_CLIENT_PORT      8443


//-----------------------------------------------------------------------------
// NIC driver
//-----------------------------------------------------------------------------
#define NIC_DRIVER_RINGBUFFER_NUMBER_ELEMENTS 16
#define NIC_DRIVER_RINGBUFFER_SIZE                                             \
    (NIC_DRIVER_RINGBUFFER_NUMBER_ELEMENTS * 4096)

//-----------------------------------------------------------------------------
// TPM
//-----------------------------------------------------------------------------
#define TPM_SRK_HANDLE 0x81000200
#define TPM_RSA_BASE_HANDLE 0x81000201

#define TPM_CLIENT_KEY_HANDLE 0

#define TPM_SRK_AUTH "ThisIsMySRKAuth"
// We need to disable rsa authentification, otherwise external keys wouldn't load properly
//#define TPM_RSA_AUTH "ThisIsMyKeyAuth"

//-----------------------------------------------------------------------------
// CRYPTO
//-----------------------------------------------------------------------------
#define SERVER_PUBLIC_KEY \
"-----BEGIN PUBLIC KEY-----\n" \
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAk+eUBh3pCZunT/Xsxkah\n" \
"UEIRm3KM7v7HzF7AfVAAQUW3XV7RA28/saXsaABlbvRL43wkodRGY3pN41MH9nms\n" \
"iJuIeBBShKfn0h9ntFkLDkGDysfXC/ZIIbCzz0QutMb6Nn9SDgwLHABv4+GWEtlo\n" \
"k14NCsJ+5Kk6KLG/xi2vfQ7CFaM/AJ2iZx1om4HddnQVECfvtK6DQGXSs9Qyq+B4\n" \
"bZPEEVYn0SozaOhV+kdSh8Ln9K/CfIqqe1Ciu40/inBwxwuAbs8lfRVYMqXpdC4e\n" \
"LLBRck31UVl94q6piebXYLMIHQHF5izfD9B7r3LR5MSPV0S76ooLg+2t0cyXLcfb\n" \
"YwIDAQAB\n" \
"-----END PUBLIC KEY-----"

#define CLIENT_PUBLIC_KEY \
"-----BEGIN PUBLIC KEY-----\n" \
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAhT+IEvoBItgpNjA5ZoCs\n" \
"1VlPLqLMdTxNc1C5/nkMSuZHWQrxxaDSWo45psWRHwYd+nDH7ZvqHXMo+OyOprnL\n" \
"evdgcpXjgW8tnr7PzFPKCIJoJsgpBqEXWZkuU/6zBSMGRzlP9L3QKgx+KTvAj4RF\n" \
"YswiCsBwKRnoKqBYwK5gy1V4HxtZAD8iY2ir/xwnIcpgY6aDTWS7q8Rjpia3kiu1\n" \
"LvYvIVPaDiNrlrEOfemkEGIyCUriXU2DgTxFDDXn9mmbOCV8AttepHC4Mf7b1nlU\n" \
"sk4Rn8dGo/9eqV0gSN/yDT/GyWKCs79zwQOdsjddrm3/fyCfVAAip4/7YaRcynjb\n" \
"XwIDAQAB\n" \
"-----END PUBLIC KEY-----"

#define CLIENT_PRIVATE_KEY \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEogIBAAKCAQEAhT+IEvoBItgpNjA5ZoCs1VlPLqLMdTxNc1C5/nkMSuZHWQrx\n" \
"xaDSWo45psWRHwYd+nDH7ZvqHXMo+OyOprnLevdgcpXjgW8tnr7PzFPKCIJoJsgp\n" \
"BqEXWZkuU/6zBSMGRzlP9L3QKgx+KTvAj4RFYswiCsBwKRnoKqBYwK5gy1V4HxtZ\n" \
"AD8iY2ir/xwnIcpgY6aDTWS7q8Rjpia3kiu1LvYvIVPaDiNrlrEOfemkEGIyCUri\n" \
"XU2DgTxFDDXn9mmbOCV8AttepHC4Mf7b1nlUsk4Rn8dGo/9eqV0gSN/yDT/GyWKC\n" \
"s79zwQOdsjddrm3/fyCfVAAip4/7YaRcynjbXwIDAQABAoH/aeuWv378aDZsjCbJ\n" \
"ejHPMclMqEXBQXAuIPyK3T5cBy4GiUGp7u9oR5PHQErMkVzLd8kvJDJMaByi9T0W\n" \
"KHKIzbbXdD6yGrHGEeqcRFBWyWzgXfO+qQZlCVQ6/4n3xJ2S4AsvA93fG43Su/RD\n" \
"ndIHVgHvDZzri8CMTVYpNwFm8DpbE7l+z+iniKOd0DSHQY76zI8KIoe0nbd+djjE\n" \
"gwV2BITV6k7OW3NrbYe90iCPFfZtfOoIDGO4rB0TnAsGVlfpfE/mmwuu5LFOMGf4\n" \
"ZyjKtbhmK1Ml/fF+Lfy0EXGcQ82eqNTYBBF8ZOiKVfHX/Gg57N3+QK0Tc3LsaM5B\n" \
"FNQ5AoGBALVYwmmoGUXyR7wAsFCggfzQt/v2Kh3sz8zxpSYuBLnefw4CEZ8XSHwy\n" \
"+Eb9/W+9SavbDhrwAK/rnUwRUFpfPH3TxGlwqu1cDzaflqPFpJVJqYw90UEzc71W\n" \
"aKrSYPWv1ysI+OeDVzD47bqOh+4J/oQy45H/i8eSIghEJ+xHdDw3AoGBALwZ5v8F\n" \
"3tO1Uc9Yl3AHJiBzbwvQiXDnTwZw7rlaWpfiarvv52B/BfzAPyuyaERpFVy6s6Jd\n" \
"C7NtkrVlrMZZs2fUDtFCX+Hy/oEXfMZ1Tob2x5uJTk9DJH6TIybE71nSgorQ0+bc\n" \
"O5mhOJBvLuVZ5hgguG/uWERHt7Mi29ct/9YZAoGBAIO/JB6WXSYPykWvSmiI82a0\n" \
"S7XlNNvgu2bs90oxjIVsO2n13s9xntt8PBt3UrPnFKqhzjGLwzQLPI+S1ImTPuM7\n" \
"AiqIC+W9R+ArOMlqQROkHGUiU+/GbYNUT14q0P4s7Wj6b7niFWoirrMl8WLiJ+Hr\n" \
"BqF+whIO/GJ9AXQKxUspAoGAez6Nd3KlORmIbM6jCqfkd0aq75bHNs6XnKTKBXAK\n" \
"A5I6VMEvXK5dgemEemD+qDQh5wv9PtiwHfQhN/FSbvO+9LygqMNQh37q+jIlcvLR\n" \
"bOSsjGA+ivh3JOfLFE/cc4HWPpXtAUozUsmrghcXJvbsJ8rojY4hDzveROUGHcrp\n" \
"4aECgYEApKe1qhvb7nR8OV6q5LZgK1b484gMC0vukpRZY1jqHmLW5Z05hXe8gJmR\n" \
"vDYZKy6XqMO6x5lG0tUHvV0O6Hnh+KLmeNVpEZTe9Uqy09ltmYUbEjYX1U9/WRDy\n" \
"SQRsMhCXg8BbbylYj1VKIuNC3NyS7CQQko/XuOEKv/tMCRjqOkA=\n" \
"-----END RSA PRIVATE KEY-----"

