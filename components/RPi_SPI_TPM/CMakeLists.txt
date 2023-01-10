#
# RasPi3 (BCM2837) SPI TPM driver
#
# Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
# SPDX-License-Identifier: BSD-3-Clause
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
function(RPi_SPI_TPM_DeclareCAmkESComponent name)

    DeclareCAmkESComponent(
        ${name}
        SOURCES
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/RPi_SPI_TPM.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/bcm2837/bcm2837_gpio.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/bcm2837/bcm2837_spi.c
        INCLUDES
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/3rdParty/bcm2837
        C_FLAGS
            -Wall
            -Werror
        LIBS
            os_core_api
            lib_debug
    )

endfunction()
