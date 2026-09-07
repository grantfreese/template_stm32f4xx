// i_mux.h - Abstract interface for analog multiplexer/switch array drivers
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <cstdint>

namespace fw
{

//! @brief Topology classification for analog multiplexer types.
enum class MuxType
{
    kSingleEnded,  //!< N:1 switch array (e.g. ADG728: 8 independent SPST switches)
    kDifferential, //!< N differential-pair switch array (e.g. ADG729: 4 differential pairs)
    kMatrix,       //!< NxM crosspoint switch array
};

//! @brief Abstract interface for an analog multiplexer or switch array.
//!
//! Template parameters encode the channel count and topology at compile time
//! so that driver subclasses (e.g. Adg728, Adg729) carry this information in
//! their type. The channel_mask convention is: bit N corresponds to channel N+1
//! (bit 0 = channel 1, bit 7 = channel 8).
//!
//! @tparam ChannelCount  Number of independently-controllable channels.
//! @tparam Type          Mux topology (kSingleEnded, kDifferential, or kMatrix).
template<uint8_t ChannelCount, MuxType Type = MuxType::kSingleEnded>
class IMux
{
public:
    //! @brief Number of independently-controllable channels.
    static constexpr uint8_t kChannels = ChannelCount;

    //! @brief Topology of this mux.
    static constexpr MuxType kType = Type;

    virtual ~IMux() = default;

    //! @brief Set which channels are closed via bitmask.
    //!
    //! Bit N in channel_mask corresponds to channel N+1 (bit 0 = ch 1, bit 7 = ch 8).
    //! Channels whose bits are clear are opened; channels whose bits are set are closed.
    //!
    //! @param channel_mask  Bitmask of channels to close.
    //! @return True on success, false on I2C error.
    virtual bool SetChannels(uint8_t channel_mask) = 0;

    //! @brief Poll the mux hardware and return the current switch state.
    //!
    //! @param channel_mask  Output: bitmask of currently closed channels.
    //! @return True on success (hardware was read); false on I2C error (channel_mask
    //!         is still populated from the shadow register in that case).
    virtual bool GetChannels(uint8_t& channel_mask) = 0;

    //! @brief Open all channels (write 0x00 to the device).
    //! @return True on success, false on I2C error.
    virtual bool Reset() = 0;
};

}  // namespace fw
