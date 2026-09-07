// delay.cpp - HAL abstractions for delays
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "delay.h"

// lib includes
#include "framework/error.h"
#include "time_units.h"

// os includes

// standard includes

namespace fw
{
static IDelay* delay_implementation{nullptr};

void DelayMs(uint32_t delay_msec)
{
    runtime_assert(delay_implementation != nullptr, "no delay source");
    delay_implementation->Delay(Milliseconds(delay_msec));
}

void DelayUs(uint32_t delay_usec)
{
    runtime_assert(delay_implementation != nullptr, "no delay source");
    delay_implementation->Delay(Microseconds(delay_usec));
}

void DelayUs(uint32_t delay_usec, bool force_busy_wait)
{
    runtime_assert(delay_implementation != nullptr, "no delay source");
    delay_implementation->Delay(Microseconds(delay_usec), force_busy_wait);
}

void SetDelaySource(IDelay& delay)
{
    auto prev_delay_implementation{delay_implementation};
    delay_implementation = &delay;
    runtime_assert(prev_delay_implementation == nullptr, "delay already provided");
}

}  // namespace fw
