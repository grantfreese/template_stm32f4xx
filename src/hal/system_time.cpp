// system_time.cpp - Implementation of IDelay
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "system_time.h"

#include "time_units.h"

// lib includes

// os includes
#include "framework/error.h"

// standard includes

namespace fw
{
static ISystemTime* system_time_implementation{nullptr};

uint32_t GetSystemTime()
{
    runtime_assert(system_time_implementation != nullptr, "no system time source");
    return system_time_implementation->GetSystemTimeMicroseconds();
}

void SetSystemTimeSource(ISystemTime& system_time)
{
    auto prev_system_time_implementation{system_time_implementation};
    system_time_implementation = &system_time;
    runtime_assert(prev_system_time_implementation == nullptr, "system time source already provided");
}

}  // namespace fw
