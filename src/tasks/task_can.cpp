// task_can.cpp - CAN broadcast FreeRTOS task implementation
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "tasks/task_can.h"

#include "adc1/adc1.h"
#include "can/bus.h"
#include "can/can_broadcast.h"
#include "cli/debug_stage.h"
#include "cmsis_os.h"
#include "peripheral.h"

// clang-format off
static StaticTask_t taskCan;
static StackType_t stackCan[kStackSizeWordsCan];

osThreadId_t taskCan_h;

const osThreadAttr_t taskAttributesCan = {
    .name       = "can",
    .attr_bits  = 0,
    .cb_mem     = &taskCan,
    .cb_size    = sizeof(taskCan),
    .stack_mem  = stackCan,
    .stack_size = kStackSizeWordsCan * sizeof(StackType_t),
    .priority   = (osPriority_t)osPriorityNormal,
    .tz_module  = 0,
    .reserved   = 0
};
// clang-format on

static fw::Adc1 adc1_;
static CanBroadcast can_broadcast_;

// Written only by this task; the reset flag keeps 'status reset' from the CLI
// task out of the stats fields (aligned single-word flag, atomic on Cortex-M4).
static fw::TaskStats task_stats;
static volatile bool stats_reset_pending{false};

// Written only by the CAN task; read by the CLI task. Aligned 32-bit accesses are atomic
// on Cortex-M4, so volatile is sufficient.
static volatile float die_temperature_c{0.0f};
static volatile uint16_t die_temperature_raw{0};

float GetMcuDieTemperatureCelsius() { return die_temperature_c; }

uint16_t GetMcuDieTemperatureRaw() { return die_temperature_raw; }

const fw::TaskStats& GetCanTaskStats() { return task_stats; }

void RequestCanTaskStatsReset() { stats_reset_pending = true; }

void TaskCan(void* argument)
{
    (void)argument;

    adc1_.Initialize();

    uint32_t next_broadcast_tick{0};

    // thread loop
    for (;;)
    {
        if (stats_reset_pending)
        {
            task_stats.Reset();
            stats_reset_pending = false;
        }

        // Record the work portion only (delay excluded): a deadline miss means the
        // work alone exceeded the period.
        const uint32_t work_start_tick = osKernelGetTickCount();

        // Assert the bus run state every tick: a no-op once running, and the retry path
        // after a failed init-mode exit (bus stuck dominant at boot or during bus-off).
        fw::SetCanMode(fw::CanMode::kRunning);

        uint32_t tick = osKernelGetTickCount();
        if (tick >= next_broadcast_tick)
        {
            next_broadcast_tick = tick + kCanBroadcastPeriodMsec;

            // Sample the die temperature even when the broadcast is gated off so the
            // CLI always reports a fresh value.
            uint16_t raw = adc1_.ReadDieTemperatureRaw();
            die_temperature_raw = raw;
            die_temperature_c = fw::Adc1::DieTemperatureCelsiusFromRaw(raw);

            DebugStageAdc1(tick, raw, die_temperature_c);

            can_broadcast_.Broadcast(&hcan, die_temperature_c, raw);
        }

        task_stats.RecordTick(osKernelGetTickCount() - work_start_tick, kCanTaskPeriodMsec);

        osDelay(kCanTaskPeriodMsec);
    }
}
