// i2c_stm32.h - I2C master driver using STM32 I2C2 peripheral
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "hal/ii2c.h"
#include "stm32f3xx_hal.h"

namespace fw
{

//! @brief I2C master driver backed by the STM32 I2C2 peripheral.
//!
//! Configures I2C2 on PF6 (SCL) / PF7 (SDA) in standard mode (100 kHz).
//! All transactions are blocking with a bounded timeout. The I2C2 kernel
//! clock is sourced from HSI (8 MHz); SystemClock_Config selects this via
//! I2c2ClockSelection = RCC_I2C2CLKSOURCE_HSI.
class I2cStm32 : public II2C
{
public:
    //! @brief Initialize the I2C2 peripheral and GPIO.
    //!
    //! Enables GPIOF and I2C2 clocks, configures PF6 / PF7 as open-drain alternate
    //! function (AF4), and initializes the I2C peripheral in master mode.
    //! Must be called before any Read/Write/WriteRead operations.
    void Initialize();

    bool Write(uint8_t address, std::span<const uint8_t> data) override;
    bool Read(uint8_t address, std::span<uint8_t> data) override;
    bool WriteRead(uint8_t address, std::span<const uint8_t> write_data, std::span<uint8_t> read_data) override;

private:
    I2C_HandleTypeDef handle_{};

    //! @brief Timeout for blocking HAL I2C calls, in milliseconds.
    static constexpr uint32_t kTimeoutMsec{100};

    //! @brief I2C timing register value for 100 kHz standard mode from the 8 MHz HSI.
    //!
    //! Computed by STM32CubeMX for I2CCLK = HSI = 8 MHz, target SCL = 100 kHz:
    //! PRESC=8, SCLDEL=4, SDADEL=2, SCLH=0x13, SCLL=0x13. The same value worked
    //! on the prior board's I2C1 because the kernel clock source was also HSI;
    //! the on-the-wire timing is governed by the I2C kernel clock (HSI), not the
    //! peripheral bus clock (PCLK), so the rename to I2C2 does not change it.
    static constexpr uint32_t kTimingValue{0x80421313U};
};

}  // namespace fw
