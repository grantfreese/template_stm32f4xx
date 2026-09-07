// stm32f3xx_uart_cli.cpp - 'uart' CLI command: report driver-side stats
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
//
// `uart status` prints "overruns: N". A healthy bench keeps N at zero;
// a nonzero count is the regression signal for the DMA RX path. The HIL
// test test.hil.uart asserts this stays at zero after stressing the link.

#include <cstring>

#include "cli/cli_binding.h"
#include "cli/cli_setup.h"

static void PrintUsage() { cli_printf("usage: uart status   show CLI-UART driver stats (overrun count)"); }

static void PrintStatus() { cli_printf("overruns: %lu", static_cast<unsigned long>(GetCliUartOverrunCount())); }

static void OnUart(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)context;

    const char* arg1 = embeddedCliGetToken(args, 1);
    if (arg1 != nullptr && std::strcmp(arg1, "status") == 0)
    {
        PrintStatus();
        return;
    }
    PrintUsage();
}

static constexpr CliCommandBinding kCommandUart = {
    .name = "uart",
    .help = "CLI-UART driver diagnostics ('uart status' for the overrun count)",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = OnUart,
};

void RegisterUartCliCommands(EmbeddedCli* cli) { embeddedCliAddBinding(cli, &kCommandUart); }
