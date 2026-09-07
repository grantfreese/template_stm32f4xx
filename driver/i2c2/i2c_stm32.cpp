// i2c_stm32.cpp - I2C master driver using STM32 I2C2 peripheral
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "i2c2/i2c_stm32.h"

#include "cli/debug_stage.h"

namespace fw
{

void I2cStm32::Initialize()
{
    // Enable GPIOF clock (I2C2 SCL/SDA on PF6 / PF7)
    __HAL_RCC_GPIOF_CLK_ENABLE();

    // Configure PF6 (SCL) and PF7 (SDA) as open-drain, AF4 (I2C2), with pull-up
    GPIO_InitTypeDef gpio_init{};
    gpio_init.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio_init.Mode = GPIO_MODE_AF_OD;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOF, &gpio_init);

    // Enable I2C2 peripheral clock
    __HAL_RCC_I2C2_CLK_ENABLE();

    // Initialize I2C2 in master mode, 100 kHz standard mode
    handle_.Instance = I2C2;
    handle_.Init.Timing = kTimingValue;
    handle_.Init.OwnAddress1 = 0;
    handle_.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    handle_.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    handle_.Init.OwnAddress2 = 0;
    handle_.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    handle_.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    handle_.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&handle_);
}

bool I2cStm32::Write(uint8_t address, std::span<const uint8_t> data)
{
    // HAL expects the 7-bit address left-shifted by 1
    uint16_t dev_addr = static_cast<uint16_t>(address) << 1U;
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        &handle_, dev_addr, const_cast<uint8_t*>(data.data()), static_cast<uint16_t>(data.size()), kTimeoutMsec);
    if (status == HAL_OK)
    {
        DebugStageI2c2Data(address, false, data.data(), static_cast<uint8_t>(data.size()));
        return true;
    }
    DebugStageI2c2Error(address, "TIMEOUT");
    return false;
}

bool I2cStm32::Read(uint8_t address, std::span<uint8_t> data)
{
    uint16_t dev_addr = static_cast<uint16_t>(address) << 1U;
    HAL_StatusTypeDef status =
        HAL_I2C_Master_Receive(&handle_, dev_addr, data.data(), static_cast<uint16_t>(data.size()), kTimeoutMsec);
    if (status == HAL_OK)
    {
        DebugStageI2c2Data(address, true, data.data(), static_cast<uint8_t>(data.size()));
        return true;
    }
    DebugStageI2c2Error(address, "TIMEOUT");
    return false;
}

bool I2cStm32::WriteRead(uint8_t address, std::span<const uint8_t> write_data, std::span<uint8_t> read_data)
{
    uint16_t dev_addr = static_cast<uint16_t>(address) << 1U;

    // Transmit phase (register address or command)
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&handle_,
                                                       dev_addr,
                                                       const_cast<uint8_t*>(write_data.data()),
                                                       static_cast<uint16_t>(write_data.size()),
                                                       kTimeoutMsec);
    if (status != HAL_OK)
    {
        DebugStageI2c2Error(address, "TIMEOUT");
        return false;
    }

    // Receive phase
    status = HAL_I2C_Master_Receive(
        &handle_, dev_addr, read_data.data(), static_cast<uint16_t>(read_data.size()), kTimeoutMsec);
    if (status == HAL_OK)
    {
        DebugStageI2c2Data(address, true, read_data.data(), static_cast<uint8_t>(read_data.size()));
        return true;
    }
    DebugStageI2c2Error(address, "TIMEOUT");
    return false;
}

}  // namespace fw
