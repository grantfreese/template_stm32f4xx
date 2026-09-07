set(APP_SOURCE_FILES
    # CLI infrastructure -- always present
    cli/cli_binding.cpp
    cli/cli_misc.cpp
    cli/cli_setup.cpp
    cli/debug_stubs.cpp
    can/bus.cpp
    can/can_broadcast.cpp
    can/can_tx_queue.cpp
    hal/delay.cpp
    hal/system_time.cpp
    main.cpp
    nvm/nvm.cpp
    peripheral.cpp
    stm32_callbacks.cpp
    tasks/task_can.cpp
    tasks/task_cli.cpp
    platform/startup_stm32f373c8tx.s
    platform/stm32f3xx_hal_msp.c
    platform/stm32f3xx_it.c
    platform/stm32f3xx_pin.cpp
    platform/stm32f3xx_platform.cpp
    platform/stm32f3xx_spi.cpp
    platform/stm32f3xx_uart.cpp
    platform/syscalls.c
    platform/sysmem.c
    platform/system_stm32f3xx.c
)

add_executable(app ${APP_SOURCE_FILES})

# Optional CLI modules are configured separately. Edit cmake/cli_features.cmake
# to enable or disable individual CLI modules.
include(${CMAKE_SOURCE_DIR}/cmake/cli_features.cmake)
target_compile_features(app PRIVATE cxx_std_17)
target_link_libraries(app PUBLIC embedded-cli driver freertos)

target_include_directories(app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_BINARY_DIR}/include     # version number (build/include/version.h)
)

# Add custom target to generate HEX file
add_custom_target(hex ALL
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/app.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/app.hex
    DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/app.elf
    COMMENT "Generating HEX file"
)


# print firmware build size
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_SIZE_UTIL} -A
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/app${CMAKE_EXECUTABLE_SUFFIX}"
)
