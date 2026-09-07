// stm32f3xx_system_time.h - Implementation of system time for stm32f3xx
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#pragma once

// local includes
#include "hal/system_time.h"

// lib includes
#include "framework/error.h"

// os includes

// standard includes

namespace fw
{

// void vConfigureTimerForRunTimeStats(void)
// {
//     // TODO: implement
// }

// unsigned long ulGetRunTimeCounterValue(void)
// {
//     // TODO implement
//     return 0UL;
// }

class SystemTimeSTM32F3 : public ISystemTime
{
public:
    uint32_t GetSystemTimeMicroseconds() override
    {
        // TODO: temp disabled
        return 0;
        // return ulGetRunTimeCounterValue();
    };
};
}  // namespace fw
