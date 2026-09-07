// cli_setup.cpp - CLI implementation
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "cli_setup.h"

#include "cli_binding.h"
#include "cli_misc.h"
#include "platform/stm32f3xx_uart.h"

// lib includes
#include "platform/stm32f3xx_it.h"
#include "stm32f3xx_hal.h"

// os includes

// standard includes
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Expand cli implementation here (must be in one file only)
#define EMBEDDED_CLI_IMPL

#include "embedded_cli.h"

static EmbeddedCli* cli;

// Backing storage for the entire embeddedcli library: EmbeddedCli +
// EmbeddedCliImpl structs, the rx/cmd/history streaming buffers, and the
// command-binding pointer table + per-binding flags. The library carves this
// block at runtime in embeddedCliNew. CliCommandBinding instances themselves
// live in .rodata (each call site declares its binding as static constexpr
// and passes its address to embeddedCliAddBinding) -- only the pointer table
// lives here, not the binding data. Sized with margin above the library's
// embeddedCliRequiredSize for the current config; init returns NULL (caught
// by cli_is_ready) if too small.
static CLI_UINT cli_buffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];
static bool cli_is_ready = false;  // CLI ready flag; drains and writes bail before this trips true.

// CLI UART instance. The driver owns a 256-byte DMA-backed RX ring; the CLI
// task drains it on every embeddedCliProcess pass via DrainCliUart().
static fw::UartSTM32F3 cli_uart{USART1, 115200};

static void writeCharToCli(EmbeddedCli* embeddedCli, char c)
{
    (void)embeddedCli;
    cli_uart.WriteByte(static_cast<uint8_t>(c));
}

// Write a string directly via writeChar, bypassing embeddedCliPrint so that no
// prompt is emitted. Used during initialization (before the CLI task is
// running) and at shutdown (after the CLI task has been parked via
// RequestCliShutdown), both situations where the embeddedCli's deferred
// drain is unsuitable.
void cli_print_direct(const char* str)
{
    for (size_t i = 0; i < strlen(str); ++i)
    {
        writeCharToCli(nullptr, str[i]);
    }
}

// Function to setup the configuration settings for the CLI, based on the definitions from this header file
void CliSetup(std::string* nameString, std::string* versionString)
{
    cli_uart.Initialize();

    // Initialize the CLI configuration settings
    EmbeddedCliConfig* config = embeddedCliDefaultConfig();
    config->cliBuffer = cli_buffer;
    config->cliBufferSize = CLI_BUFFER_SIZE;
    config->rxBufferSize = CLI_RX_BUFFER_SIZE;
    config->cmdBufferSize = CLI_CMD_BUFFER_SIZE;
    config->historyBufferSize = CLI_HISTORY_SIZE;
    config->maxBindingCount = CLI_MAX_BINDING_COUNT;

    // Create new CLI instance
    cli = embeddedCliNew(config);
    // Assign character write function
    cli->writeChar = writeCharToCli;

    // CLI init failed. Is there not enough memory allocated to the CLI?
    // Please increase the 'CLI_BUFFER_SIZE' in header file.
    // Or decrease max binding / history size.
    // You can get required buffer size by calling
    // uint16_t requiredSize = embeddedCliRequiredSize(config);
    // Then check it's value in debugger
    while (cli == NULL)
    {
        HardFault_Handler();
    }

    // Add all the initial command bindings
    initCliBinding();

    // Store name + version strings for use by onClearCLI when invoked as a command
    SetCliFirmwareStrings(nameString, versionString);

    // Print the welcome screen directly (bypassing embeddedCliPrint) so that no
    // prompt is emitted here. embeddedCliProcess() will print the first prompt on
    // its initial tick.
    cli_print_direct(nameString->c_str());
    cli_print_direct("\r\n");
    cli_print_direct("version: ");
    cli_print_direct(versionString->c_str());
    cli_print_direct("\r\n");

    // CLI has now been initialized, set bool to true to enable interrupts.
    cli_is_ready = true;
}

// Forward incoming UART bytes to the embedded-cli's queue. Drained from the
// CLI task loop (DrainCliUart), not from an ISR, so embeddedCliReceiveChar's
// internal ring is touched from a single context.
static void FeedEmbeddedCli(uint8_t byte, void*)
{
    if (cli_is_ready)
    {
        embeddedCliReceiveChar(cli, static_cast<char>(byte));
    }
}

void DrainCliUart() { cli_uart.DrainRx(FeedEmbeddedCli, nullptr); }

uint32_t GetCliUartOverrunCount() { return cli_uart.GetOverrunCount(); }

// Function to encapsulate the 'embeddedCliPrint()' call with print formatting arguments (act like printf(), but keeps
// cursor at correct location). The 'embeddedCliPrint()' function does already add a linebreak ('\r\n') to the end of
// the print statement, so no need to add it yourself.
void cli_printf(const char* format, ...)
{
    // Create a buffer to store the formatted string
    char buffer[CLI_PRINT_BUFFER_SIZE];

    // Format the string using snprintf
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Check if string fitted in buffer else print error to stderr
    if (length < 0)
    {
        fprintf(stderr, "Error formatting the string\r\n");
        return;
    }

    // Call embeddedCliPrint with the formatted string
    embeddedCliPrint(getCliPointer(), buffer);
}

EmbeddedCli* getCliPointer() { return cli; }
