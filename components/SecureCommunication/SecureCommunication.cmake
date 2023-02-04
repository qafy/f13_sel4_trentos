#
# Secure Communication component
#
# Copyright (C) 2021, HENSOLDT Cyber GmbH
#

cmake_minimum_required(VERSION 3.18)

project(secureCommunication_api C)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

#-------------------------------------------------------------------------------
#
# Declare SecureCommunication CAmkES Component
#
# Parameters:
#
#   <name>
#     component instance name. The server will be called "<name>" and the
#     corresponding client library will be called "<name>_client"
#
function(SecureCommunication_DeclareCAmkESComponent
    name
)

    # Let caller append to any of the build options with his own variables
    cmake_parse_arguments(
        PARSE_ARGV
        1
        NETWORKSTACK_EXTRA
        ""
        ""
        "SOURCES;C_FLAGS;INCLUDES;LIBS"
    )

    #---------------------------------------------------------------------------
    set(SECURE_COMMUNICATION_LIBS
            picotcp
            system_config
            lib_debug
            lib_macros
            lib_server
            os_core_api
            os_socket_client
            # os_crypto is needed for mbedtls
            os_crypto
            3rdparty_mbedtls_for_cert
            3rdparty_mbedtls_for_crypto
    )

    if(HW_TPM)
        list(APPEND SECURE_COMMUNICATION_LIBS
            tpm_crypto
            tpm_keystore
        )
    else()
        list(APPEND SECURE_COMMUNICATION_LIBS
            os_filesystem
            os_keystore_ram_fv
        )
    endif()

    DeclareCAmkESComponent(${name}
        SOURCES
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/SecureCommunication.c
        C_FLAGS
            -Wall
            -Werror
        INCLUDES
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include/
        LIBS
            ${SECURE_COMMUNICATION_LIBS}
    )

endfunction()
