// platform.h - Platform virtual interface
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#pragma once

// local includes

// lib includes
#include "hal/delay.h"
#include "hal/system_time.h"

// os includes

// standard includes

#pragma once

namespace fw
{

/// @brief Represents a target platform
class IPlatform
{
public:
    /// @brief Initializes the platform. This handles setting up any framework specified by the HAL or OS
    /// layers. For example, providing singletons for IDelay and ISystemTime.
    virtual void Initialize() = 0;

    /// @brief Get the system time object
    /// @return Gets an object which can report the sytsem runtime
    virtual ISystemTime& GetSystemTime() = 0;

    /// @brief Get the system delay object
    /// @return Gets an object which can be used to block a thread for a specified amount of time
    virtual IDelay& GetDelay() = 0;

    /// @brief Use the current clock rate to set the timer based interrupt period.
    virtual void UpdateTimerInterruptPeriod() = 0;
};

/// @brief Get the platform
IPlatform& GetPlatform();

/// @brief Initialize the platform
void PlatformInitialize();

}  // namespace fw
