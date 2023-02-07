#
# RasPi3 (BCM2837) SPI TPM driver
#
# Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
# Copyright (C) 2023, Jakob Kukla
# SPDX-License-Identifier: BSD-3-Clause
#
# Code adapted from:
#  - TRENTOS RPi_SPI_Flash component
#

cmake_minimum_required(VERSION 3.17)


#-------------------------------------------------------------------------------
# Declare RasPi3 (BCM2837) SPI TPM Driver CAmkES Component
#
# Parameters:
#
#   <name>
#     required, component instance name
#
#   LIBS <lib_1> <lib_2> ...
#     additional libraries. At least a TimeServer client library is required
#
function(RPi_SPI_TPM_DeclareCAmkESComponent name)
    cmake_parse_arguments(
        PARSE_ARGV
        1
        RPi_SPI_TPM
        ""
        ""
        "LIBS"
    )

    set(RPi_SPI_TPM_DEBUG_TRACE "no" CACHE STRING
        "Enables option for debug tracing (default: disabled)")

    # wolfSSL
    set(WITH_WOLFSSL_TREE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/wolfSSL)
    set(WOLFSSL_RSA "yes")
    set(WOLFSSL_SINGLE_THREADED "yes")

    # wolfTPM
    set(WOLFTPM_INTERFACE "BCM2837_SPI")

    # wolfSSL & wolfTPM debug info
    if (WOLF_DEBUG)
        set(WOLFSSL_DEBUG "yes")
        set(WOLFTPM_DEBUG "yes")
    endif()

    add_subdirectory(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/wolfTPM)

    if (RPi_SPI_TPM_DEBUG_TRACE)
        set_source_files_properties(
            #${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/bcm2837/bcm2837_gpio.c
            #${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/bcm2837/bcm2837_spi.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/RPi_SPI_TPM.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/tpm_io.c
            PROPERTIES
                COMPILE_OPTIONS "-finstrument-functions"
        )
    endif()

    DeclareCAmkESComponent(
        ${name}
        SOURCES
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/bcm2837/bcm2837_gpio.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/bcm2837/bcm2837_spi.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/RPi_SPI_TPM.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/crypto_rpc.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/keystore_rpc.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/tpm_int.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/tpm_io.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/util.c
        INCLUDES
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/bcm2837
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include
        C_FLAGS
            -Wall
            -Werror
        LIBS
            os_core_api
            lib_debug
            system_config
            wolftpm
            ${RPi_SPI_TPM_LIBS}
    )

endfunction()

#------------------------------------------------------------------------------
#
#                            Interface libraries
#
#------------------------------------------------------------------------------

project(tpm_crypto C)

set(TPM_CRYPTO_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/interfaces/libs/tpm_crypto/src/TPM_Crypto.c"
)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME}
    INTERFACE
        ${TPM_CRYPTO_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        "${CMAKE_CURRENT_LIST_DIR}/interfaces/libs/tpm_crypto/include"
)

#------------------------------------------------------------------------------

project(tpm_keystore C)

set(TPM_KEYSTORE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/interfaces/libs/tpm_keystore/src/TPM_Keystore.c"
)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME}
    INTERFACE
        ${TPM_KEYSTORE_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        "${CMAKE_CURRENT_LIST_DIR}/interfaces/libs/tpm_keystore/include"
)
