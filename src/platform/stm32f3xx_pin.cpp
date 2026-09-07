// stm32f3xx_pin.cpp - Pin implementation for stm32f3xx
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "stm32f3xx_pin.h"

// lib includes
#include "stm32f3xx_hal.h"

// os includes

// standard includes

namespace fw
{
IOPinSTM32F3::IOPinSTM32F3(
    Port gpio_port, Pin gpio_pin, Speed speed, Pull pull, IODirection direction, PinState asserted_state)
    : port{gpio_port}, pin{gpio_pin}, speed{speed}, pull{pull}, direction{direction}, asserted_state_{asserted_state}
{
}

IOState IOPinSTM32F3::GetState() const
{
    // reinterpret_cast is used to convert Port class-enum to STM32 HAL GPIO_TypeDef*
    PinState pin_state{HAL_GPIO_ReadPin(reinterpret_cast<GPIO_TypeDef*>(toUnder(port)), toUnder(pin)) != 0
                           ? PinState::kHigh
                           : PinState::kLow};
    return pin_state == asserted_state_ ? IOState::kAsserted : IOState::kDeasserted;
}

void IOPinSTM32F3::ReconfigurePin(IODirection direction)
{
    switch (port)
    {
        case Port::kPortA:
            __HAL_RCC_GPIOA_CLK_ENABLE();
            break;
        case Port::kPortB:
            __HAL_RCC_GPIOB_CLK_ENABLE();
            break;
        case Port::kPortC:
            __HAL_RCC_GPIOC_CLK_ENABLE();
            break;
        case Port::kPortD:
            __HAL_RCC_GPIOD_CLK_ENABLE();
            break;
        case Port::kPortE:
            __HAL_RCC_GPIOE_CLK_ENABLE();
            break;
        case Port::kPortF:
            __HAL_RCC_GPIOF_CLK_ENABLE();
            break;
    }

    this->direction = direction;
    GPIO_InitTypeDef GPIO_InitStruct{
        .Pin = toUnder(pin),
        .Mode = toUnder(direction),
        .Pull = toUnder(pull),
        .Speed = toUnder(speed),
        .Alternate = 0  // 0 == No alternate function
    };
    HAL_GPIO_Init(reinterpret_cast<GPIO_TypeDef*>(toUnder(port)), &GPIO_InitStruct);
}

InputPinSTM32F3::InputPinSTM32F3(Port gpio_port, Pin gpio_pin, Pull pull, PinState asserted_state)
    : IOPinSTM32F3{gpio_port, gpio_pin, Speed::kSpeedLow, pull, direction, asserted_state}
{
}

void InputPinSTM32F3::init() { ReconfigurePin(IODirection::kInput); }

IOState InputPinSTM32F3::GetState() const { return IOPinSTM32F3::GetState(); }

void InputPinSTM32F3::SetPull(Pull new_pull)
{
    pull = new_pull;
    ReconfigurePin(IODirection::kInput);
}

BiDirectionalPinSTM32F3::BiDirectionalPinSTM32F3(
    Port gpio_port, Pin gpio_pin, Speed speed, Pull pull, IODirection direction, PinState asserted_state)
    : IOPinSTM32F3{gpio_port, gpio_pin, speed, pull, direction, asserted_state}
{
}

void BiDirectionalPinSTM32F3::init(void) { ReconfigurePin(direction); }

void BiDirectionalPinSTM32F3::SetDirection(IODirection new_direction)
{
    direction = new_direction;
    ReconfigurePin(direction);
}

void BiDirectionalPinSTM32F3::set_pull(Pull new_pull)
{
    pull = new_pull;
    ReconfigurePin(direction);
}

IOState BiDirectionalPinSTM32F3::GetState() const { return IOPinSTM32F3::GetState(); }

void BiDirectionalPinSTM32F3::SetState(IOState state)
{
    PinState pin_state{state == IOState::kAsserted ? asserted_state_ : !asserted_state_};

    HAL_GPIO_WritePin(
        reinterpret_cast<GPIO_TypeDef*>(toUnder(port)), toUnder(pin), static_cast<GPIO_PinState>(toUnder(pin_state)));
}

OutputPinSTM32F3::OutputPinSTM32F3(Port gpio_port, Pin gpio_pin, Speed speed, Pull pull, PinState asserted_state)
    : IOPinSTM32F3{gpio_port, gpio_pin, speed, pull, direction, asserted_state}
{
}

void OutputPinSTM32F3::init() { ReconfigurePin(IODirection::kOutput); }

IOState OutputPinSTM32F3::GetState() const { return IOPinSTM32F3::GetState(); }

void OutputPinSTM32F3::SetState(IOState state)
{
    PinState pin_state{state == IOState::kAsserted ? asserted_state_ : !asserted_state_};

    HAL_GPIO_WritePin(
        reinterpret_cast<GPIO_TypeDef*>(toUnder(port)), toUnder(pin), static_cast<GPIO_PinState>(toUnder(pin_state)));
}
}  // namespace fw
