// can_cli.cpp - 'can' CLI command
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include <cstring>

#include "can/bus.h"
#include "can/can_broadcast.h"
#include "cli/cli_setup.h"
#include "embedded_cli.h"
#include "main.h"
#include "tasks/task_can.h"

namespace
{

void PrintHelp()
{
    cli_printf("usage: can        show broadcast gate, bus mode, and broadcast values");
    cli_printf("       can on     enable the periodic broadcast");
    cli_printf("       can off    disable the periodic broadcast");
}

void PrintGate()
{
    const char* gate = "OFF";
    if (IsCanBroadcastEnabled())
    {
        gate = "ON";
    }
    cli_printf("broadcast: %s (period %lu ms)", gate, static_cast<unsigned long>(kCanBroadcastPeriodMsec));
}

void PrintStatus()
{
    PrintGate();
    cli_printf("mode:      %s (%lu bit/s)",
               fw::CanModeToString(fw::GetCanMode()),
               static_cast<unsigned long>(fw::kCanBitrateHz));
    cli_printf("FW_VERSION:      %s", kFirmwareVersionString.c_str());
    cli_printf("MCU_TEMPERATURE: %.2f C (raw=%u)",
               static_cast<double>(GetMcuDieTemperatureCelsius()),
               static_cast<unsigned int>(GetMcuDieTemperatureRaw()));
}

void OnCan(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)context;

    const char* arg1 = embeddedCliGetToken(args, 1);
    const char* arg2 = embeddedCliGetToken(args, 2);

    if (arg1 == NULL)
    {
        PrintStatus();
    }
    else if (strcmp(arg1, "on") == 0 && arg2 == NULL)
    {
        SetCanBroadcastEnabled(true);
        PrintGate();
    }
    else if (strcmp(arg1, "off") == 0 && arg2 == NULL)
    {
        SetCanBroadcastEnabled(false);
        PrintGate();
    }
    else
    {
        PrintHelp();
    }
}

}  // namespace

void RegisterCanCliCommands(EmbeddedCli* cli)
{
    static constexpr CliCommandBinding kCommandCan = {
        .name = "can",
        .help = "CAN broadcast control and status",
        .tokenizeArgs = true,
        .context = NULL,
        .binding = OnCan,
    };
    embeddedCliAddBinding(cli, &kCommandCan);
}
