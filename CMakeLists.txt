cmake_minimum_required(VERSION 3.22)

# use new behavior for VERSION variable
cmake_policy(SET CMP0048 NEW)

# Default to a Debug build when the user (or CI) hasn't picked a type. The toolchain
# file's CMAKE_*_FLAGS_DEBUG line carries -g3, which is what makes Ozone / GDB happy;
# without it, gcc emits no DWARF and the program looks like a "stripped" binary.
# CI overrides this with -DCMAKE_BUILD_TYPE=Release for the published .elf / .hex.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

project(stm32f4-template
    DESCRIPTION "STM32F407 firmware project template"
    LANGUAGES CXX C ASM)

# build settings
set(EXTRA_WARNINGS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)

set(LINKER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/STM32F373CCTX_FLASH.ld)

set(CPU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

set(COMPILER_FLAGS
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
    -ffunction-sections
    -fdata-sections
    -fsigned-char
    -fmessage-length=0
    -fshort-enums
    -fno-exceptions
)

set(LINKER_FLAGS
    -specs=nosys.specs
    -specs=nano.specs
    # nano.specs strips full newlib for newlib-nano: smaller printf/scanf, smaller
    # impure_data and malloc bookkeeping. Re-link float support for printf so %f / %.2f
    # in our CLI / log output keep working under nano's reduced printf.
    "LINKER:-u,_printf_float"
    "LINKER:--gc-sections"
    "LINKER:--cref"
    "LINKER:--print-memory-usage"
    "LINKER:-Map=stm32f4-template.map"
)

set(COMPILE_DEFINITIONS
    -DSTM32F373xC
    -DUSE_HAL_DRIVER
    -DDEBUG
)

# import helper scripts
include(${CMAKE_SOURCE_DIR}/toolchain/utils.cmake)
include(${CMAKE_SOURCE_DIR}/toolchain/arm-gcc-toolchain.cmake)

string(REPLACE ";" " " _LINKER_CPU_FLAGS "${CPU_FLAGS}")
string(APPEND CMAKE_EXE_LINKER_FLAGS " ${_LINKER_CPU_FLAGS} -Wl,-T ${LINKER_FILE}")
message("CMAKE_EXE_LINKER_FLAGS= ${CMAKE_EXE_LINKER_FLAGS}")


add_compile_options(${CPU_FLAGS} ${COMPILER_FLAGS})
add_link_options(${CPU_FLAGS} ${LINKER_FLAGS})
add_definitions(${COMPILE_DEFINITIONS})

# set output directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# freertos
add_subdirectory(lib/freeRTOS)
target_include_directories(freertos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src )

# embedded-cli
add_subdirectory(lib/embedded-cli)
target_include_directories(embedded-cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src )

add_subdirectory(driver)
target_include_directories(driver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src )

# executable generated here
add_subdirectory(src)

# enable extra warnings
if(EXTRA_WARNINGS STREQUAL "ON")
    enable_extra_warnings(app driver)
endif()


# Doxygen documentation (build with: ninja -C build doc_doxygen)
find_package(Doxygen)
if (DOXYGEN_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc_doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()

# Generate build/include/version.h. BYPRODUCTS + copy_if_different: a version change
# reaches the ELF in a single build, an unchanged version rebuilds nothing.
add_custom_target(generate_version ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/include
    COMMAND bash -c "${CMAKE_SOURCE_DIR}/scripts/get_version.sh | sed 's/.*/\\#define PROJECT_VERSION \"&\"/' > ${CMAKE_BINARY_DIR}/include/version.h.tmp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/include/version.h.tmp ${CMAKE_BINARY_DIR}/include/version.h
    BYPRODUCTS ${CMAKE_BINARY_DIR}/include/version.h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating version header"
    VERBATIM
)

add_dependencies(app generate_version)
