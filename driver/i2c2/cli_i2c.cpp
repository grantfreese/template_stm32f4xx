// cli_i2c.cpp - I2C bus diagnostic CLI command implementation
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
//
// Buses are registered at init via RegisterI2cCliBus(); the scan/probe verbs
// operate on whatever buses the application registered. With no buses
// registered the command reports that and does nothing.

#include "cli_i2c.h"

#include <cstdlib>
#include <cstring>

#include "cli/cli_setup.h"
#include "hal/ii2c.h"

using namespace fw;

static constexpr int kMaxBuses = 4;

static struct
{
    const char* label;
    II2C* bus;
} buses[kMaxBuses];
static int bus_count{0};

void RegisterI2cCliBus(const char* label, II2C& bus)
{
    if (bus_count < kMaxBuses)
    {
        buses[bus_count].label = label;
        buses[bus_count].bus = &bus;
        bus_count++;
    }
    // Silent overflow. Not expected in practice; registration happens once at init.
}

static II2C* GetBus(int index)
{
    if (index >= 0 && index < bus_count)
    {
        return buses[index].bus;
    }
    return nullptr;
}

// Probe a 7-bit address by writing a single zero byte.
// For ADG728 devices this is a safe no-op (opens all switches).
static bool ProbeAddress(II2C& bus, uint8_t address)
{
    uint8_t dummy[]{0x00};
    return bus.Write(address, dummy);
}

static int ParseBusIndex(const char* arg)
{
    if (arg == nullptr || arg[1] != '\0' || arg[0] < '0' || arg[0] > '9')
    {
        return -1;
    }
    return arg[0] - '0';
}

static void ScanBus(int index, II2C& bus)
{
    cli_printf("bus %d (%s) scan (0x08-0x77):", index, buses[index].label);
    int found = 0;
    for (uint8_t addr = 0x08; addr <= 0x77; addr++)
    {
        if (ProbeAddress(bus, addr))
        {
            cli_printf("  0x%02X: ACK", addr);
            found++;
        }
    }
    if (found == 0)
    {
        cli_printf("  (no devices)");
    }
}

static void PrintHelp()
{
    if (bus_count == 0)
    {
        cli_printf("no I2C buses registered (see RegisterI2cCliBus)");
        return;
    }
    cli_printf("i2c scan <N>          scan 0x08-0x77 on bus N");
    cli_printf("i2c probe <N> <hex>   probe one address on bus N");
    cli_printf("buses:");
    for (int i = 0; i < bus_count; i++)
    {
        cli_printf("  %d: %s", i, buses[i].label);
    }
}

void OnI2c(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)context;

    const char* arg1 = embeddedCliGetToken(args, 1);

    if (arg1 == nullptr)
    {
        PrintHelp();
        return;
    }

    if (strcmp(arg1, "scan") == 0)
    {
        int index = ParseBusIndex(embeddedCliGetToken(args, 2));
        II2C* bus = GetBus(index);
        if (bus == nullptr)
        {
            PrintHelp();
            return;
        }
        ScanBus(index, *bus);
        return;
    }

    if (strcmp(arg1, "probe") == 0)
    {
        int index = ParseBusIndex(embeddedCliGetToken(args, 2));
        II2C* bus = GetBus(index);
        if (bus == nullptr)
        {
            PrintHelp();
            return;
        }
        const char* addr_str = embeddedCliGetToken(args, 3);
        if (addr_str == nullptr)
        {
            PrintHelp();
            return;
        }
        char* end = nullptr;
        unsigned long addr_val = strtoul(addr_str, &end, 16);
        if (end == addr_str || *end != '\0' || addr_val > 0x7F)
        {
            cli_printf("invalid address: %s (expected 00-7F hex)", addr_str);
            return;
        }
        auto addr = static_cast<uint8_t>(addr_val);
        bool ack = ProbeAddress(*bus, addr);
        const char* ack_str = "NAK";
        if (ack)
        {
            ack_str = "ACK";
        }
        cli_printf("i2c%d 0x%02X: %s", index, addr, ack_str);
        return;
    }

    PrintHelp();
}

void RegisterI2cCliCommands(EmbeddedCli* cli)
{
    static constexpr CliCommandBinding kCommandI2c = {
        .name = "i2c",
        .help = "I2C bus diagnostics (run 'i2c' for the registered bus list)",
        .tokenizeArgs = true,
        .context = NULL,
        .binding = OnI2c,
    };
    embeddedCliAddBinding(cli, &kCommandI2c);
}
