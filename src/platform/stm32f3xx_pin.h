// stm32f3xx_pin.h - GPIO management class for stm32f3xx
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "helpers.h"

// lib includes
#include "hal/io.h"
#include "pindefs.h"
#include "stm32f3xx_hal.h"

// os includes

// standard includes
#include <cstdint>

namespace fw
{
struct PinConfigSTM32F3
{
    uint32_t pin_config;
    uint32_t gpio_port;
    uint16_t gpio_pin;
};

class IOPinSTM32F3 : public virtual IDigitalSignal
{
public:
    enum class Pull : uint32_t
    {
        kStandard = GPIO_NOPULL,
        kPullUp = GPIO_PULLUP,
        kPullDown = GPIO_PULLDOWN
    };

    enum class Speed : uint32_t
    {
        kSpeedLow = GPIO_SPEED_FREQ_LOW,        /// 0-2 MHz
        kSpeedMedium = GPIO_SPEED_FREQ_MEDIUM,  /// 4-10 MHz
        kSpeedHigh = GPIO_SPEED_FREQ_HIGH       /// 10-50 MHz
    };

    enum class Port : uint32_t
    {
        kPortA = GPIOA_BASE,
        kPortB = GPIOB_BASE,
        kPortC = GPIOC_BASE,
        kPortD = GPIOD_BASE,
        kPortE = GPIOE_BASE,
        kPortF = GPIOF_BASE
    };

    enum class Pin : uint16_t
    {
        k0 = GPIO_PIN_0,
        k1 = GPIO_PIN_1,
        k2 = GPIO_PIN_2,
        k3 = GPIO_PIN_3,
        k4 = GPIO_PIN_4,
        k5 = GPIO_PIN_5,
        k6 = GPIO_PIN_6,
        k7 = GPIO_PIN_7,
        k8 = GPIO_PIN_8,
        k9 = GPIO_PIN_9,
        k10 = GPIO_PIN_10,
        k11 = GPIO_PIN_11,
        k12 = GPIO_PIN_12,
        k13 = GPIO_PIN_13,
        k14 = GPIO_PIN_14,
        k15 = GPIO_PIN_15
    };
    IOPinSTM32F3(Port gpio_port,
                 Pin gpio_pin,
                 Speed speed,
                 Pull pull,
                 IODirection direction,
                 PinState asserted_state = PinState::kHigh);

    IOState GetState() const override;

    void set_asserted_state(PinState asserted_state) { asserted_state_ = asserted_state; }

protected:
    Port port;
    Pin pin;
    Speed speed;
    Pull pull;
    IODirection direction;
    PinState asserted_state_{PinState::kHigh};

    /// @brief Initialize or reconfigure a GPIO pin
    void ReconfigurePin(IODirection direction);
};

class BiDirectionalPinSTM32F3 : public IOPinSTM32F3, public virtual IBiDirectionalDigitalSignal
{
public:
    BiDirectionalPinSTM32F3(Port gpio_port,
                            Pin gpio_pin,
                            Speed speed,
                            Pull pull,
                            IODirection direction,
                            PinState asserted_state = PinState::kHigh);

    void init();

    void SetDirection(IODirection new_direction) override;
    void set_pull(Pull new_pull);

    IOState GetState() const override;
    void SetState(IOState) override;
};

class InputPinSTM32F3 : public IOPinSTM32F3
{
public:
    InputPinSTM32F3(Port gpio_port, Pin gpio_pin, Pull pull, PinState asserted_state = PinState::kHigh);

    void init();

    IOState GetState() const override;

    void SetPull(Pull new_pull);
};

class OutputPinSTM32F3 : public IOPinSTM32F3, public virtual ISettableDigitalSignal
{
public:
    OutputPinSTM32F3(Port gpio_port, Pin gpio_pin, Speed speed, Pull pull, PinState asserted_state = PinState::kHigh);

    void init();

    IOState GetState() const override;
    void SetState(IOState) override;
};
}  // namespace fw
