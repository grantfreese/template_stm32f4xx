// delay.h - HAL abstractions for delays
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "time_units.h"

// lib includes

// os includes

// standard includes
#include <cstdint>

namespace fw
{
class IDelay
{
public:
    /// @brief Blocking delay method for milliseconds
    /// @param ms number of milliseconds to block
    /// @param force_busy_wait if true, use busy-wait loop instead of RTOS delay
    virtual void Delay(Milliseconds ms, bool force_busy_wait = false) = 0;

    /// @brief Blocking delay method for microseconds
    /// @param us number of microseconds to block
    /// @param force_busy_wait if true, use busy-wait loop instead of RTOS delay
    virtual void Delay(Microseconds us, bool force_busy_wait = false) = 0;

    // void Delay(Microseconds us, bool busyWait);
};

/// @brief Blocking delay function for milliseconds
/// @param delay_msec the number of milliseconds to block
void DelayMs(uint32_t delay_msec);

/// @brief Blocking delay function for microseconds
/// @param delay_usec the number of microseconds to block
/// @param force_busy_wait if true, use busy-wait loop instead of RTOS delay
void DelayUs(uint32_t delay_usec, bool force_busy_wait = false);

/// @brief Set delay implementation for above functions to use
/// @param delay the delay object
void SetDelaySource(IDelay& delay);

}  // namespace fw
