#
# Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
#


# Sources for only CRYPTO builds
set(SOURCES_SECURE_COMMUNICATION
    "${CMAKE_CURRENT_SOURCE_DIR}/library/pem.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/library/x509write_crt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/library/asn1write.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/library/pkwrite.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/library/pk.c"
    
)

set(MODULE_DEFINITIONS)

list(APPEND MODULE_DEFINITIONS "-DMBEDTLS_PEM_WRITE_C")
list(APPEND MODULE_DEFINITIONS "-DMBEDTLS_PK_WRITE_C")
list(APPEND MODULE_DEFINITIONS "-DMBEDTLS_BIGNUM_C")
list(APPEND MODULE_DEFINITIONS "-DMBEDTLS_ASN1_WRITE_C")

set(COMPILE_OTIONS
    -Wall
)

#------------------------------------------------------------------------------

project(3rdparty_mbedtls_for_secure_communication C)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME}
    INTERFACE
        ${SOURCES_SECURE_COMMUNICATION}
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(${PROJECT_NAME}
    INTERFACE
       ${COMPILE_OPTIONS}#
       
)

add_definitions(${MODULE_DEFINITIONS})
