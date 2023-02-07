#
# Demo TUM Course
#
# Copyright (C) 2021, Hensoldt Cyber GmbH
# Copyright (C) 2023, Jakob Kukla
#

cmake_minimum_required(VERSION 3.7.2)

add_compile_definitions(MBEDTLS_RSA_NO_CRT)


#-------------------------------------------------------------------------------
os_set_config_file(
    "system_config.h"
    CONFIG_PROJECT_NAME "system_config"
)

#-------------------------------------------------------------------------------
project(demo_tum_course C)

option(HW_TPM "Use RPi_SPI_TPM component for hardware TPM support" OFF)

include("plat/${PLATFORM}/plat_nic.cmake")
include("components/SecureCommunication/CMakeLists.txt")

if(LOAD_KEYS_FROM_FILESYSTEM)
    add_definitions(-DLOAD_KEYS_FROM_FILESYSTEM)
endif()

if(BENCHMARK)
    add_definitions(-DBENCHMARK)
endif()

if(HW_TPM)
    add_definitions(-DUSE_HW_TPM)
    # Small hack to add definitions to the camkes preprocessor
    set_property(GLOBAL APPEND PROPERTY CAMKES_ROOT_CPP_FLAGS "-DUSE_HW_TPM")

    include("components/RPi_SPI_TPM/CMakeLists.txt")

    RPi_SPI_TPM_DeclareCAmkESComponent(
        RPi_SPI_TPM
        LIBS
            TimeServer_client
    )

else()
    SdHostController_DeclareCAmkESComponent(
        SdHostController
    )

    EntropySource_DeclareCAmkESComponent(
        EntropySource
    )
endif()

SecureCommunication_DeclareCAmkESComponent(
    SecureCommunication
)

TimeServer_DeclareCAmkESComponent(
    TimeServer
)

NetworkStack_PicoTcp_DeclareCAmkESComponent(
    NetworkStack_PicoTcp
    C_FLAGS
        -DNetworkStack_PicoTcp_USE_HARDCODED_IPADDR
        -DDEV_ADDR=ETH_ADDR
        -DGATEWAY_ADDR=ETH_GATEWAY_ADDR
        -DSUBNET_MASK=ETH_SUBNET_MASK
)

DeclareCAmkESComponent(
    TestApp
    SOURCES
        components/TestApp/src/TestApp.c
    C_FLAGS
        -Wall
    LIBS
        system_config
        lib_debug
        os_core_api
        os_socket_client
)

DeclareAndCreateCamkESSystem(demo_tum_course.camkes)
GenerateSimulateScript()