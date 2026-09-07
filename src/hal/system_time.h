// system_time.h - HAL abstractions for system time
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes

// lib includes
#include "time.h"

// os includes

// standard includes
#include <cstdint>

namespace fw
{

class ISystemTime
{
public:
    /// @brief System time as the number of microseconds since start. (time will overflow)
    /// @return the system time in microseconds
    virtual uint32_t GetSystemTimeMicroseconds() = 0;
};

/// @brief Gets the system time (time since boot, will overflow).
/// @return time in microseconds
uint32_t GetSystemTime();

/// @brief used to provide a system time source for the free functions above.
/// @param system_time the system time object
void SetSystemTimeSource(ISystemTime& system_time);

}  // namespace fw
