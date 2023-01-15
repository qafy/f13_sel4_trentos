# CMakeList.txt
#
# Copyright (C) 2006-2022 wolfSSL Inc.
#
# This file is part of wolfSSL. (formerly known as CyaSSL)
#
# Usage:
# $ mkdir build
# $ cd build
# $ cmake ..
# $ cmake --build .
#
# To build with debugging use:
# $ cmake .. -DCMAKE_BUILD_TYPE=Debug
#
# See "Building with CMake" in INSTALL for more.

####################################################
# Project
####################################################

cmake_minimum_required(VERSION 3.16)

project(wolftpm VERSION 2.7.0 LANGUAGES C)

set(WOLFTPM_DEFINITIONS)

set(TPM_SOURCES
    src/tpm2.c
    src/tpm2_linux.c
    src/tpm2_packet.c
    src/tpm2_param_enc.c
    src/tpm2_swtpm.c
    src/tpm2_tis.c
    src/tpm2_winapi.c
    src/tpm2_wrap.c
    )

# default to build static library
option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" OFF)
add_library(wolftpm
    STATIC
    ${TPM_SOURCES}
    )
target_compile_definitions(wolftpm PRIVATE
    "BUILDING_WOLFTPM"
    )
target_link_libraries(wolftpm
    muslc
    sel4
    sel4_autoconf
    lib_debug
    lib_mem
)

if (RPi_SPI_TPM_DEBUG_TRACE)
    set_source_files_properties(${TPM_SOURCES}
        PROPERTIES
            COMPILE_OPTIONS "-finstrument-functions"
    )
endif()


# TODO
# * wrapper
# * wolfcrypt
# * I2C
# * Advanced IO
# * Device specific (ST33, ATTPM20, NPCT, SLB, automatic)
# * wait state
# * small stack


# Enable Debugging
set(WOLFTPM_DEBUG "no" CACHE STRING
    "Enables option for debug (default: disabled)")
set_property(CACHE WOLFTPM_DEBUG
    PROPERTY STRINGS "yes;no;verbose")
if(WOLFTPM_DEBUG)
    list(APPEND WOLFTPM_DEFINITIONS
        "-DDEBUG_WOLFTPM"
        "-DDEBUG")
    if("${WOLFTPM_DEBUG}" STREQUAL "verbose")
        list(APPEND WOLFTPM_DEFINITIONS "-DWOLFTPM_DEBUG_VERBOSE")
    endif("${WOLFTPM_DEBUG}" STREQUAL "verbose")
endif()

# Device Interface
set(WOLFTPM_INTERFACE "auto" CACHE STRING
    "Select interface to TPM")
set_property(CACHE WOLFTPM_INTERFACE
    PROPERTY STRINGS "auto;SWTPM;WINAPI;DEVTPM;BCM2837_SPI")

# automatically set
message("INTERFACE ${WOLFTPM_INTERFACE}")
if("${WOLFTPM_INTERFACE}" STREQUAL "auto")
    message("auto")
    if(WIN32 OR MINGW OR MSYS)
        message("Detected windows, using WIN TBS API")
        set_property(CACHE WOLFTPM_INTERFACE PROPERTY VALUE "WINAPI")
    elseif(UNIX)
        message("Detected *nix. using kernel device for interface")
        set_property(CACHE WOLFTPM_INTERFACE PROPERTY VALUE "DEVTPM")
    else()
        set_property(CACHE WOLFTPM_INTERFACE PROPERTY VALUE "SWTPM")
    endif(WIN32 OR MINGW OR MSYS)
endif("${WOLFTPM_INTERFACE}" STREQUAL "auto")


if("${WOLFTPM_INTERFACE}" STREQUAL "SWTPM")
    list(APPEND WOLFTPM_DEFINITIONS "-DWOLFTPM_SWTPM")

elseif("${WOLFTPM_INTERFACE}" STREQUAL "DEVTPM")
    list(APPEND WOLFTPM_DEFINITIONS "-DWOLFTPM_LINUX_DEV")

elseif("${WOLFTPM_INTERFACE}" STREQUAL "WINAPI")
    list(APPEND WOLFTPM_DEFINITIONS "-DWOLFTPM_WINAPI")
    target_link_libraries(wolftpm PRIVATE tbs)
elseif("${WOLFTPM_INTERFACE}" STREQUAL "BCM2837_SPI")
    #list(APPEND WOLFTPM_DEFINITIONS "-DWOLFTPM_SLB9670")
else()
    get_property(INTERFACE_OPTS CACHE WOLFTPM_INTERFACE
        PROPERTY STRINGS)
    message(FATAL_ERROR "\"${WOLFTPM_INTERFACE}\" is not known WOLFTPM_INTERFACE:"
        " ${INTERFACE_OPTS}")
endif("${WOLFTPM_INTERFACE}" STREQUAL "SWTPM")

target_include_directories(wolftpm
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )


if (WITH_WOLFSSL)
    target_link_libraries(wolftpm PUBLIC wolfssl)
    target_include_directories(wolftpm PUBLIC ${WITH_WOLFSSL}/include)
    target_link_directories(wolftpm PUBLIC ${WITH_WOLFSSL}/lib)
    elseif (WITH_WOLFSSL_TREE)
    set(WOLFSSL_TPM "yes" CACHE STRING "")
    set(WOLFSSL_EXAMPLES "no" CACHE STRING "")
    set(WOLFSSL_CRYPT_TESTS "no" CACHE STRING "")
    add_subdirectory(${WITH_WOLFSSL_TREE} wolfssl)
    target_link_libraries(wolftpm wolfssl)
else()
    find_package(PkgConfig)
    pkg_check_modules(WOLFSSL wolfssl)

    if (WOLFSSL_FOUND)
        target_link_libraries(wolftpm PUBLIC ${WOLFSSL_LIBRARIES})
        target_include_directories(wolftpm PUBLIC ${WOLFSSL_INCLUDE_DIRS})
        target_link_directories(wolftpm PUBLIC ${WOLFSSL_LIBRARY_DIRS})
        target_compile_options(wolftpm PUBLIC ${WOLFSSL_CFLAGS_OTHER})
    else()
        # For support with vcpkg
        find_package(wolfssl CONFIG)
        if (wolfssl_FOUND)
            target_link_libraries(wolftpm PUBLIC wolfssl)
        else()
            list(APPEND WOLFTPM_DEFINITIONS "-DWOLFTPM2_NO_WOLFCRYPT")
        endif()
    endif()
endif()

function(add_to_options_file DEFINITIONS OPTION_FILE)
    list(REMOVE_DUPLICATES DEFINITIONS)
    foreach(DEF IN LISTS DEFINITIONS)
        if(DEF MATCHES "^-D")
            if(DEF MATCHES "^-D(N)?DEBUG(=.+)?")
                message("not outputting (N)DEBUG to ${OPTION_FILE}")
            endif()

            string(REGEX REPLACE "^-D" "" DEF_NO_PREFIX ${DEF})
            string(REGEX REPLACE "=.*$" "" DEF_NO_EQUAL_NO_VAL ${DEF_NO_PREFIX})
            string(REPLACE "=" " " DEF_NO_EQUAL ${DEF_NO_PREFIX})

            file(APPEND ${OPTION_FILE} "#undef  ${DEF_NO_EQUAL_NO_VAL}\n")
            file(APPEND ${OPTION_FILE} "#define ${DEF_NO_EQUAL}\n")

            file(APPEND ${OPTION_FILE} "\n")
        else()
            message("option w/o begin -D is ${DEF}, not saving to ${OPTION_FILE}")
        endif()
    endforeach()
endfunction()

add_definitions(${WOLFTPM_DEFINITIONS})

# generate options file
message("Generating user options header...")
if (${CMAKE_DISABLE_SOURCE_CHANGES})
    set(WOLFTPM_BUILD_OUT_OF_TREE_DEFAULT "${CMAKE_DISABLE_SOURCE_CHANGES}")
else()
    set(WOLFTPM_BUILD_OUT_OF_TREE_DEFAULT "no")
endif()

set(WOLFTPM_BUILD_OUT_OF_TREE "${WOLFTPM_BUILD_OUT_OF_TREE_DEFAULT}" CACHE STRING
    "Don't generate files in the source tree (default: ${WOLFTPM_BUILD_OUT_OF_TREE_DEFAULT})")
set_property(CACHE WOLFTPM_BUILD_OUT_OF_TREE
    PROPERTY STRINGS "yes;no")

if (${WOLFTPM_BUILD_OUT_OF_TREE})
   set(WOLFTPM_OUTPUT_BASE ${CMAKE_CURRENT_BINARY_DIR})
else()
   set(WOLFTPM_OUTPUT_BASE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
set(OPTION_FILE "${WOLFTPM_OUTPUT_BASE}/wolftpm/options.h")

file(REMOVE ${OPTION_FILE})

file(APPEND ${OPTION_FILE} "/* wolftpm options.h\n")
file(APPEND ${OPTION_FILE} " * generated from cmake configure options\n")
file(APPEND ${OPTION_FILE} " *\n")
file(APPEND ${OPTION_FILE} " * Copyright (C) 2006-2022 wolfSSL Inc.\n")
file(APPEND ${OPTION_FILE} " *\n")
file(APPEND ${OPTION_FILE} " * This file is part of wolfSSL.\n")
file(APPEND ${OPTION_FILE} " *\n")
file(APPEND ${OPTION_FILE} " */\n\n")
file(APPEND ${OPTION_FILE} "#ifndef WOLFTPM_OPTIONS_H\n")
file(APPEND ${OPTION_FILE} "#define WOLFTPM_OPTIONS_H\n\n\n")
file(APPEND ${OPTION_FILE} "#ifdef __cplusplus\n")
file(APPEND ${OPTION_FILE} "extern \"C\" {\n")
file(APPEND ${OPTION_FILE} "#endif\n\n")

add_to_options_file("${WOLFTPM_DEFINITIONS}" "${OPTION_FILE}")
# CMAKE_C_FLAGS is just a string of space-separated flags to pass to the C
# compiler. We need to replace those spaces with semicolons in order to treat it
# as a CMake list.
string(REPLACE " " ";" CMAKE_C_FLAGS_LIST "${CMAKE_C_FLAGS}")
add_to_options_file("${CMAKE_C_FLAGS_LIST}" "${OPTION_FILE}")

file(APPEND ${OPTION_FILE} "\n#ifdef __cplusplus\n")
file(APPEND ${OPTION_FILE} "}\n")
file(APPEND ${OPTION_FILE} "#endif\n\n\n")
file(APPEND ${OPTION_FILE} "#endif /* WOLFTPM_OPTIONS_H */\n\n")

message("${WOLFTPM_DEFINITIONS}")