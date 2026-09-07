// adg728.h - Driver for the ADG728 8-channel analog multiplexer
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "adg728/i_mux.h"
#include "hal/ii2c.h"

#include <cstdint>

namespace fw
{

//! @brief Controls whether a single ADG728 channel switch is open or closed.
enum class ChannelState
{
    kEnabled,
    kDisabled,
};

//! @brief Driver for the ADG728 8-channel analog multiplexer/switch array.
//!
//! The ADG728 is controlled by writing a single byte over I2C. Each bit in
//! the byte corresponds to one of the 8 switch channels (bit 0 = channel 1,
//! bit 7 = channel 8). Multiple channels may be enabled simultaneously.
//! The device also supports I2C read-back of its current switch state.
//!
//! A local shadow register tracks the current switch state so that
//! individual channel changes do not require a round-trip to the device.
//! GetChannels() performs an actual I2C read; on failure it falls back to
//! the shadow register.
class Adg728 : public IMux<8, MuxType::kSingleEnded>
{
public:
    //! @brief Construct the driver.
    //! @param i2c      I2C bus used to communicate with the device.
    //! @param address  7-bit I2C address of this ADG728 (set by A2/A1/A0 pins).
    Adg728(II2C& i2c, uint8_t address);

    //! @brief Enable or disable a single channel without affecting others.
    //! @param channel  0-indexed channel number (0–7 maps to channels 1–8).
    //!                 Out-of-range values are no-ops.
    //! @param state    kEnabled to close the switch, kDisabled to open it.
    void SetChannel(uint8_t channel, ChannelState state);

    // IMux<8, kSingleEnded> implementation:

    //! @brief Set all eight channels at once via a bitmask.
    //! @param channel_mask  Bit N => channel N+1 closed (bit 0 = ch 1, bit 7 = ch 8).
    //! @return True on success, false on I2C error.
    bool SetChannels(uint8_t channel_mask) override;

    //! @brief Poll hardware for the current switch state.
    //!
    //! Performs an I2C read to obtain the actual register value from the device.
    //! If the read fails, channel_mask is set from the shadow register and false
    //! is returned so callers can detect and handle the error.
    //!
    //! @param channel_mask  Output: bitmask of currently closed channels.
    //! @return True if the hardware was successfully read; false on I2C error.
    bool GetChannels(uint8_t& channel_mask) override;

    //! @brief Open all switches (write 0x00 to the device).
    //! @return True on success, false on I2C error.
    bool Reset() override;

    //! @brief Return the last-written switch state without an I2C round-trip.
    //!
    //! The shadow register is updated by SetChannels() and Reset() on every
    //! successful write. Use this when the I2C bus may be unavailable.
    //! @return Bitmask of channels that were last commanded closed.
    uint8_t GetShadow() const { return shadow_; }

private:
    II2C& i2c_;
    uint8_t address_;
    uint8_t shadow_{0x00};

    //! @brief Write the current shadow register to the device.
    //! @return True on success, false on I2C error.
    bool Write();
};

}  // namespace fw
