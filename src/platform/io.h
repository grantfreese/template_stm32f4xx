// io.h - IO signal implementation
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "hal/io.h"

// lib includes

// os includes

// standard includes
#include <cstdint>

namespace fw
{
class Gpio : public fw::ISettableDigitalSignal
{
public:
    Gpio(GPIO_TypeDef* const base, uint16_t pins) : base_{base}, pins_{pins} {}
    void SetState(IOState state) override
    {
        if (state == IOState::kAsserted)
        {
            HAL_GPIO_WritePin(base_, pins_, GPIO_PIN_SET);
        }
        else if (state == IOState::kDeasserted)
        {
            HAL_GPIO_WritePin(base_, pins_, GPIO_PIN_RESET);
        }
        else
        {
            // throw HALException("Invalid IOState state");
        }
    }
    IOState GetState() const override
    {
        if (HAL_GPIO_ReadPin(base_, pins_) == GPIO_PIN_RESET)
        {
            return IOState::kDeasserted;
        }
        else
        {
            return IOState::kAsserted;
        }
    }

private:
    GPIO_TypeDef* const base_;
    uint16_t pins_;
};
}  // namespace fw
