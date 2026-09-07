# cli_features.cmake - CLI module and debug streamer build configuration
# copyright Grant Freese 2026
#
# Each group controls one CLI module and its paired debug streamer.
# Comment out a group (or individual lines within it) to remove from the binary.
# Commented-out debug sources produce no staging output and no print output:
# debug_stubs.cpp provides weak no-ops for staging, and the print function
# resolves to null (via debug_registry.h) so DebugPrintPending() skips it.
#
# To add a new CLI module:
#   1. Add a target_sources() line for the CLI source here.
#   2. Add a weak declaration to src/cli/cli_registry.h.
#   3. Implement RegisterXxxCliCommands(EmbeddedCli*) in the new source file.
#
# To add a new debug streamer:
#   1. Add a target_sources() line for the debug source here.
#   2. Add a weak declaration to src/cli/debug_registry.h.
#   3. Add a weak no-op stub to src/cli/debug_stubs.cpp.
#   4. Implement DebugPrintXxx(int budget) and DebugStageXxx() in the new source file.

# debug framework CLI -- flag state, rate, print loop (always-on infrastructure,
# but listed here so it can be stripped in minimal builds)
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/src/cli/cli_debug.cpp)

# can -- 'can' command: broadcast TX gate, bus run state, current broadcast values
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/src/can/can_cli.cpp)

# i2c -- I2C bus diagnostics
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/driver/i2c2/cli_i2c.cpp)

# nvm -- 'nvm show/get/set' for the persistent settings record
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/src/cli/cli_nvm.cpp)

# simple -- toggle simple CLI mode (no embedded-cli autocomplete, no ANSI escape
# sequences in the output stream)
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/src/cli/cli_simple.cpp)

# spi -- 'spi info' command: PCLK1, PCLK2, per-peripheral BR field and computed
# SCK. Verifies SPI clock rate without a scope.
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/src/cli/cli_spi.cpp)

# status -- 'status [reset]' command: per-task loop statistics (count,
# min/avg/max work time, deadline misses) and stack usage.
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/src/cli/cli_status.cpp)

# uart -- 'uart status' command: CLI-UART driver overrun count (DMA RX ring
# should keep this at zero).
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/src/platform/stm32f3xx_uart_cli.cpp)

# Standalone debug streamers (no corresponding full CLI module).
# Comment out to silence that streamer; the staging calls become no-ops via
# debug_stubs.cpp and the print function resolves to null.
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/driver/adc1/debug_adc1.cpp)   # debug adc1
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/driver/i2c2/debug_i2c2.cpp)   # debug i2c2 / i2c2err
target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/src/can/debug_can.cpp)        # debug can
