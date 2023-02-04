#
# Network Stack PicoTcp component
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

# Include the mbedtls project, but do not build any targets from it unless they
# are explicitly included.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rdParty/mbedtls EXCLUDE_FROM_ALL)

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
    DeclareCAmkESComponent(${name}
        SOURCES
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/SecureCommunication.c
        C_FLAGS
            -Wall
        INCLUDES
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include/
        LIBS
            picotcp
            system_config
            lib_debug
            lib_macros
            lib_server
            os_core_api
            os_socket_client
            os_crypto
            os_filesystem
            os_keystore_ram_fv
            3rdparty_mbedtls_for_cert
            3rdparty_mbedtls_for_crypto
            3rdparty_mbedtls_for_secure_communication
    )

endfunction()
