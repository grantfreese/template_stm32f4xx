// task_can.h - CAN broadcast FreeRTOS task declaration
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <cstdint>

#include "cmsis_os.h"
#include "tasks/task_stats.h"

extern osThreadId_t taskCan_h;
extern const osThreadAttr_t taskAttributesCan;

constexpr unsigned int kStackSizeWordsCan{512};

//! @brief CAN task loop period in milliseconds.
constexpr uint32_t kCanTaskPeriodMsec{100};

//! @brief Broadcast period for the FW_VERSION and MCU_TEMPERATURE messages.
constexpr uint32_t kCanBroadcastPeriodMsec{1000};

void TaskCan(void* argument);

//! @brief Loop statistics for the CAN task. Written only by the CAN task.
const fw::TaskStats& GetCanTaskStats();

//! @brief Request a reset of the CAN task's loop statistics on its next iteration.
void RequestCanTaskStatsReset();

//! @brief Latest MCU die temperature sampled by the CAN task, in degrees Celsius.
float GetMcuDieTemperatureCelsius();

//! @brief Raw ADC counts behind GetMcuDieTemperatureCelsius().
uint16_t GetMcuDieTemperatureRaw();
