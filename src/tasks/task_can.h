// task_can.h - CAN broadcast FreeRTOS task declaration
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <cstdint>

#include "cmsis_os.h"

extern osThreadId_t taskCan_h;
extern const osThreadAttr_t taskAttributesCan;

constexpr unsigned int kStackSizeWordsCan{512};

//! Broadcast period for the FW_VERSION and MCU_TEMPERATURE messages.
constexpr uint32_t kCanBroadcastPeriodMsec{1000};

void TaskCan(void* argument);

//! @brief Latest MCU die temperature sampled by the CAN task, in degrees Celsius.
float GetMcuDieTemperatureCelsius();

//! @brief Raw ADC counts behind GetMcuDieTemperatureCelsius().
uint16_t GetMcuDieTemperatureRaw();
