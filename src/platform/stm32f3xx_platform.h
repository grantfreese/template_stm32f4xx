// stm32f3xx_platform.h - Platform header for stm32f3xx
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#pragma once

// local includes
#include "platform.h"

// lib includes
#include "FreeRTOS.h"
#include "hal/delay.h"
#include "platform/stm32f3xx_delay.h"
#include "stm32f3xx_system_time.h"

// os includes

// standard includes

namespace fw
{

// TODO: figure out which FreeRTOS header can bring this prototype in
void vPortSetupTimerInterrupt(void);

class PlatformSTM32F3 : public IPlatform
{
public:
    void Initialize() override
    {
        SetSystemTimeSource(system_time_);
        SetDelaySource(delay_);
    }

    ISystemTime& GetSystemTime() override { return system_time_; }

    IDelay& GetDelay() override { return delay_; }

    void UpdateTimerInterruptPeriod() override
    {
        // TODO: fix this
        // vPortSetupTimerInterrupt();
    }

private:
    SystemTimeSTM32F3 system_time_{};
    DelaySTM32F3 delay_{};
};

IPlatform& GetPlatform();

void PlatformInitialize();

}  // namespace fw
