// stm32f3xx_delay.h - Implementation of IDelay for stm32f3xx
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes

// lib includes
#include "FreeRTOSConfig.h"
#include "framework/error.h"
#include "hal/delay.h"
#include "hal/time_units.h"
#include "stm32f3xx_hal.h"

// os includes
#include "FreeRTOS.h"
#include "task.h"

// standard includes
#include <cmath>
#include <cstdint>

namespace fw
{
class DelaySTM32F3 : public IDelay
{
public:
    void Delay(Milliseconds ms, bool forceBusyWait = false) override
    {
        DelayMs(static_cast<uint32_t>(ms.count()), forceBusyWait);
    }

    void Delay(Microseconds us, bool forceBusyWait = false) override
    {
        DelayUs(static_cast<uint32_t>(us.count()), forceBusyWait);
    }

    // void Delay(Microseconds us, bool busyWait) { DelayUs(static_cast<uint32_t>(us.count()), busyWait); }

private:
    static constexpr uint32_t kBusyWaitThresholdUs{static_cast<uint32_t>(TICKS_TO_MSEC(1) * 1000.0f)};

    void DelayMs(uint32_t delayMsec, bool forceBusyWait)
    {
        if (forceBusyWait)
        {
            DelayUs(delayMsec * 1000, true);
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(delayMsec));
        }
    }

    void DelayUs(uint32_t delayUsec, bool forceBusyWait)
    {
        if ((delayUsec >= kBusyWaitThresholdUs) && !forceBusyWait)
        {
            vTaskDelay(pdMS_TO_TICKS(static_cast<float>(delayUsec) * 1e-3f));
        }
        else
        {
            constexpr int kMicroSecondsPerSeconds{1'000'000};
            constexpr int kMinimumDelay{12};              // time in usec to execute this function without the busy loop
            constexpr float kClockCyclesPerDelay{16.0f};  // number of clock cycles per delay loop iteration

            const float clock_cycles_per_us{static_cast<float>(HAL_RCC_GetHCLKFreq()) / kMicroSecondsPerSeconds};
            const float clock_cycles{clock_cycles_per_us
                                     * static_cast<float>(static_cast<int>(delayUsec) - kMinimumDelay)};

            const int32_t cycles{
                static_cast<int32_t>(std::round(static_cast<float>(clock_cycles) / kClockCyclesPerDelay))};

            // skip single cycle delays
            if (cycles > 0)
            {
                // busy wait
                for (int32_t i = 0; i < cycles; ++i)
                {
                    __NOP();
                }
            }
        }
    }
};
}  // namespace fw
