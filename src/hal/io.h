// io.h - HAL abstractions for IO signals
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes

// lib includes

// os includes

// standard includes

namespace fw
{

enum class IOState : bool
{
    kDeasserted = false,
    kAsserted = true,
};

constexpr IOState operator!(IOState state)
{
    switch (state)
    {
        case IOState::kAsserted:
            return IOState::kDeasserted;
        case IOState::kDeasserted:
            return IOState::kAsserted;
    }
    return IOState::kAsserted;
}

enum class PinState : bool
{
    kLow = false,
    kHigh = true
};

constexpr PinState operator!(PinState state)
{
    switch (state)
    {
        case PinState::kHigh:
            return PinState::kLow;
        case PinState::kLow:
            return PinState::kHigh;
    }
    return PinState::kHigh;
}

enum class IODirection : unsigned int
{
    kInput = 0,
    kOutput = 1
};

class IDigitalSignal
{
public:
    virtual IOState GetState() const = 0;
};

class ISettableDigitalSignal : public virtual IDigitalSignal
{
public:
    virtual void SetState(IOState state) = 0;
};

class IBiDirectionalDigitalSignal : public virtual ISettableDigitalSignal
{
public:
    virtual void SetDirection(IODirection direction) = 0;
};

}  // namespace fw
