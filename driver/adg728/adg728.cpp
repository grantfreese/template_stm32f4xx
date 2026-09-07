// adg728.cpp - Driver for the ADG728 8-channel analog multiplexer
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "adg728.h"

#include <cstdint>

namespace fw
{

Adg728::Adg728(II2C& i2c, uint8_t address) : i2c_(i2c), address_(address) {}

void Adg728::SetChannel(uint8_t channel, ChannelState state)
{
    if (channel > 7)
    {
        return;
    }

    if (state == ChannelState::kEnabled)
    {
        shadow_ |= static_cast<uint8_t>(1U << channel);
    }
    else
    {
        shadow_ &= static_cast<uint8_t>(~(1U << channel));
    }

    Write();
}

bool Adg728::SetChannels(uint8_t channel_mask)
{
    shadow_ = channel_mask;
    return Write();
}

bool Adg728::GetChannels(uint8_t& channel_mask)
{
    uint8_t received = 0;
    bool ok = i2c_.Read(address_, std::span<uint8_t>(&received, 1));
    if (ok)
    {
        shadow_ = received;
        channel_mask = received;
    }
    else
    {
        channel_mask = shadow_;
    }
    return ok;
}

bool Adg728::Reset()
{
    shadow_ = 0x00;
    return Write();
}

bool Adg728::Write()
{
    uint8_t data[]{shadow_};
    return i2c_.Write(address_, data);
}

}  // namespace fw
