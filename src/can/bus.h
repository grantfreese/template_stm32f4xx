// bus.h - CAN bus run state (running / halted)
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <cstdint>

namespace fw
{

//! @brief Whether the CAN peripheral is transmitting/receiving or parked silent.
//!
//! - kRunning: peripheral out of init mode; TX and RX active.
//! - kHalted: peripheral parked in init mode. No TX, no RX.
//!
//! The wire bitrate is fixed (see kCanBitrateHz); only the run state varies.
enum class CanMode
{
    kRunning,
    kHalted,
};

//! @brief The CAN bitrate in Hz. Bit timing is configured in MX_CAN_Init.
constexpr uint32_t kCanBitrateHz = 1000000;

//! @brief Human-readable name for a CanMode.
const char* CanModeToString(CanMode mode);

//! @brief Current bus run state.
CanMode GetCanMode();

//! @brief True when the peripheral is out of init mode and able to transmit.
bool IsCanRunning();

//! @brief Move the bus to @p mode. Re-asserting the current mode is a no-op.
//!
//! kHalted aborts pending TX and parks the peripheral in init mode. kRunning performs a
//! bounded init-mode exit; on a stuck-dominant bus it fails and is retried on the next
//! call, so the peripheral recovers once the bus is freed.
//!
//! @return true on success, false if the peripheral did not reach the requested state.
bool SetCanMode(CanMode mode);

}  // namespace fw
