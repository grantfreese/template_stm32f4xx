// can_broadcast.h - Periodic CAN broadcast frame builders
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "can/can_tx_queue.h"

// standard includes
#include <array>
#include <cstdint>
#include <string>

constexpr int kCanBroadcastFrameBufferSize{4};

struct CanBroadcastFrame
{
    fw::CanFrameType type;
    uint16_t length;
    uint8_t data[8];
};

//! @brief Builds and transmits the periodic broadcast frames.
//!
//! Encapsulates all frame construction logic and the transmit loop.
//! Call Broadcast() once per broadcast period from the CAN task.
class CanBroadcast
{
public:
    //! @brief Build all broadcast frames and hand them to the CAN TX queue.
    //! @param hcan                HAL CAN peripheral handle.
    //! @param die_temperature_c   Latest MCU die temperature in degrees Celsius.
    //! @param die_temperature_raw Raw ADC counts behind die_temperature_c.
    void Broadcast(CAN_HandleTypeDef* hcan, float die_temperature_c, uint16_t die_temperature_raw);

private:
    void BuildVersionFrame(const std::string& firmware_version_string);
    void BuildTemperatureFrame(float die_temperature_c, uint16_t die_temperature_raw);

    std::array<CanBroadcastFrame, kCanBroadcastFrameBufferSize> frame_buffer_;
    int buffer_index_{0};
};

//! @brief Returns true if the periodic broadcast is enabled (default on at boot).
bool IsCanBroadcastEnabled();

//! @brief Enable or disable the periodic broadcast. Session-scoped; resets on boot.
//!
//! Disabling silences CanBroadcast::Broadcast but leaves the CAN peripheral, RX
//! filters, and the debug streamer untouched.
void SetCanBroadcastEnabled(bool enabled);
