// debug_adc1.cpp - ADC1 debug streamer
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include <cstdint>

#include "cli/cli_setup.h"
#include "cli/debug_stage.h"

static volatile bool adc1_pending_{false};
static volatile uint32_t adc1_tick{0};
static volatile uint16_t adc1_raw{0};
static volatile float adc1_temperature_c{0.0f};

void DebugStageAdc1(uint32_t tick_msec, uint16_t raw, float temperature_c)
{
    if (!IsDebugEnabled(kDebugAdc1))
    {
        return;
    }
    adc1_tick = tick_msec;
    adc1_raw = raw;
    adc1_temperature_c = temperature_c;
    adc1_pending_ = true;
}

int DebugPrintAdc1(int budget)
{
    if (!adc1_pending_)
    {
        return 0;
    }
    if (budget == 0)
    {
        return 1;
    }
    adc1_pending_ = false;
    cli_printf("[%6lu ms] ADC1 die temp: %.2f C (raw=%u)",
               static_cast<unsigned long>(adc1_tick),
               static_cast<double>(adc1_temperature_c),
               static_cast<unsigned int>(adc1_raw));
    return 1;
}
