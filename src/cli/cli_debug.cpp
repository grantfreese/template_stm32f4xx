// cli_debug.cpp - Debug print CLI commands and print-loop framework
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "cli_debug.h"

#include <cstdlib>
#include <cstring>

#include "cli/debug_registry.h"
#include "cli_binding.h"
#include "cli_setup.h"
#include "stm32f3xx_hal.h"

static uint32_t debug_flags{0};
static float debug_print_rate_hz{2.0f};
static uint32_t debug_print_period_msec{500};

static constexpr int kMaxDebugLinesPerCycle = 8;

uint32_t GetDebugFlags() { return debug_flags; }

void SetDebugFlags(uint32_t flags) { debug_flags = flags; }

uint32_t GetDebugPrintPeriodMsec() { return debug_print_period_msec; }

void SetDebugPrintRateHz(float hz)
{
    if (hz < 0.1f)
    {
        hz = 0.1f;
    }
    if (hz > 20.0f)
    {
        hz = 20.0f;
    }
    debug_print_rate_hz = hz;
    debug_print_period_msec = static_cast<uint32_t>(1000.0f / hz);
}

bool IsDebugEnabled(uint32_t source_flag)
{
    return (debug_flags & kDebugMaster) != 0 && (debug_flags & source_flag) != 0;
}

void DebugPrintPending()
{
    static uint32_t suppressed_count_{0};
    static uint32_t suppressed_report_tick_{0};

    // Each entry is a weak symbol. When the corresponding debug source file is absent
    // from the build (line commented out in cmake/cli_features.cmake), the linker
    // resolves the symbol to null and the loop skips it.
    using PrintFn = int (*)(int);
    // clang-format off
    static const PrintFn kPrintModules[] = {
        DebugPrintAdc1,
        DebugPrintCan,
        DebugPrintI2c2,
    };
    // clang-format on

    int budget = kMaxDebugLinesPerCycle;
    for (auto fn : kPrintModules)
    {
        if (fn == nullptr)
        {
            continue;
        }
        if (budget > 0)
        {
            int printed = fn(budget);
            budget -= printed;
        }
        else
        {
            // Budget exhausted -- ask the module how many lines it has pending
            // and count them as suppressed.
            suppressed_count_ += static_cast<uint32_t>(fn(0));
        }
    }

    constexpr uint32_t kSuppressedReportIntervalMsec = 2000;
    uint32_t now = HAL_GetTick();
    if (suppressed_count_ > 0 && (now - suppressed_report_tick_) >= kSuppressedReportIntervalMsec)
    {
        cli_printf("[debug] suppressed %lu lines in last 2s", static_cast<unsigned long>(suppressed_count_));
        suppressed_count_ = 0;
        suppressed_report_tick_ = now;
    }
}

static const char* OnOff(uint32_t mask)
{
    if ((debug_flags & mask) != 0)
    {
        return "ON";
    }
    return "OFF";
}

static void PrintInfo()
{
    cli_printf("debug master:  %s", OnOff(kDebugMaster));
    cli_printf("debug can:     %s", OnOff(kDebugCan));
    cli_printf("debug adc1:    %s", OnOff(kDebugAdc1));
    cli_printf("debug i2c2:    %s", OnOff(kDebugI2c2));
    cli_printf("debug i2c2err: %s", OnOff(kDebugI2c2Err));
    cli_printf("debug rate:    %.1f Hz (%lu ms)",
               static_cast<double>(debug_print_rate_hz),
               static_cast<unsigned long>(debug_print_period_msec));
}

void onDebug(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)context;

    const char* arg1 = embeddedCliGetToken(args, 1);
    const char* arg2 = embeddedCliGetToken(args, 2);

    if (arg1 == NULL)
    {
        // bare "debug" or "d" -- toggle master
        debug_flags ^= kDebugMaster;
        cli_printf("debug master: %s", OnOff(kDebugMaster));
    }
    else if ((strcmp(arg1, "info") == 0 || strcmp(arg1, "i") == 0) && arg2 == NULL)
    {
        PrintInfo();
    }
    else if (strcmp(arg1, "can") == 0 && arg2 == NULL)
    {
        debug_flags ^= kDebugCan;
        cli_printf("debug can: %s", OnOff(kDebugCan));
    }
    else if (strcmp(arg1, "adc1") == 0 && arg2 == NULL)
    {
        debug_flags ^= kDebugAdc1;
        cli_printf("debug adc1: %s", OnOff(kDebugAdc1));
    }
    else if (strcmp(arg1, "i2c2") == 0 && arg2 == NULL)
    {
        debug_flags ^= kDebugI2c2;
        cli_printf("debug i2c2: %s", OnOff(kDebugI2c2));
    }
    else if (strcmp(arg1, "i2c2err") == 0 && arg2 == NULL)
    {
        debug_flags ^= kDebugI2c2Err;
        cli_printf("debug i2c2err: %s", OnOff(kDebugI2c2Err));
    }
    else if (strcmp(arg1, "rate") == 0 && arg2 != NULL)
    {
        float hz = std::stof(arg2);
        SetDebugPrintRateHz(hz);
        cli_printf("debug rate: %.1f Hz (%lu ms)",
                   static_cast<double>(debug_print_rate_hz),
                   static_cast<unsigned long>(debug_print_period_msec));
    }
    else
    {
        cli_printf("usage: debug            toggle master enable");
        cli_printf("       debug info       show debug state");
        cli_printf("       debug can        toggle CAN frame logging");
        cli_printf("       debug adc1       toggle ADC1 die-temperature prints");
        cli_printf("       debug i2c2       toggle STM32 I2C2 transaction logging");
        cli_printf("       debug i2c2err    toggle STM32 I2C2 error logging");
        cli_printf("       debug rate <n>   set print rate (0.1-20 Hz)");
    }
}

void RegisterDebugCliCommands(EmbeddedCli* cli)
{
    embeddedCliAddBinding(cli, &commandDebug);
    embeddedCliAddBinding(cli, &commandDebugAlias);
}
