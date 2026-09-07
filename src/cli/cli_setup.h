// cli_setup.h - CLI configuration
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "embedded_cli.h"

// lib includes
#include "stm32f3xx_hal.h"

// os includes

// standard includes
#include <string>

// Definitions for CLI sizes
#define CLI_BUFFER_SIZE 1024
// embedded_cli's input fifo between DrainCliUart and embeddedCliProcess. Sized
// to match kRxRingSize so a full DMA ring can be drained in one pass without
// dropping bytes at fifoBufPush. With DMA already providing the ISR-vs-task
// decoupling, this fifo is conceptually redundant and could be eliminated in a
// future cleanup that pushes drained bytes straight into the parser state
// machine -- doing so would free another ~256 B.
#define CLI_RX_BUFFER_SIZE 256
#define CLI_CMD_BUFFER_SIZE 64
#define CLI_HISTORY_SIZE 32
#define CLI_MAX_BINDING_COUNT 32

/**
 * Definition of the cli_printf() buffer size.
 * Can make smaller to decrease RAM usage,
 * make larger to be able to print longer strings.
 */
#define CLI_PRINT_BUFFER_SIZE 512

/// @brief Initialize CLI
/// @param nameString firmware name string
/// @param versionString firmware version string
void CliSetup(std::string* nameString, std::string* versionString);

/**
 * Function to encapsulate the 'embeddedCliPrint()' call with
 * print formatting arguments (act like printf(), but keeps cursor at correct location).
 * The 'embeddedCliPrint()' function does already add a linebreak ('\\r\\n')
 * to the end of the print statement, so no need to add it yourself.
 * @param format format string
 * @param ... format arguments
 */
void cli_printf(const char* format, ...);

//! @brief Write a string to UART synchronously, bypassing the embeddedCli
//! output buffer.
//!
//! Each byte is sent via blocking `HAL_UART_Transmit` (~87 us at 115200 baud)
//! and the function returns only after every byte has shifted out of the
//! UART peripheral. Use when the caller cannot tolerate the embeddedCli
//! buffer's deferred drainage -- e.g. printing a marker just before a reset,
//! where the 50 ms CLI task cadence is too slow.
//!
//! Does NOT emit a prompt or any embeddedCli framing; the caller is
//! responsible for supplying `\r\n` if a newline is wanted.
//!
//! Concurrency: not thread-safe with the CLI task's own UART writes -- the
//! caller is expected to have first stopped the CLI task (see
//! `RequestCliShutdown` / `IsCliShutdownComplete` in `task_cli.h`).
void cli_print_direct(const char* str);

/**
 * Getter function, to keep only one instance of the EmbeddedCli pointer in this file.
 * @return
 */
EmbeddedCli* getCliPointer();

//! @brief Drain any bytes the CLI UART's DMA ring has captured into the
//! embedded-cli command queue. Called from the CLI task before
//! embeddedCliProcess to make freshly-arrived bytes visible. Not safe to
//! call from an ISR.
void DrainCliUart();

//! @brief Latched count of CLI UART overrun-error events. Should stay at
//! zero across a full HIL suite; any nonzero value flags a regression in
//! the DMA RX path.
uint32_t GetCliUartOverrunCount();
