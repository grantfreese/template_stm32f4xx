// stm32f3xx_spi.h - SPI class for stm32f3xx
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "stm32f3xx_pin.h"

// lib includes
#include "hal/ispi.h"
#include "stm32f3xx_hal.h"

// os includes

// standard includes
#include <stddef.h>

#include <span>
#include <vector>

namespace fw
{
extern "C"
{
    void SPI1InterruptHandler();
    void SPI2InterruptHandler();
    void SPI3InterruptHandler();
}

class SpiSTM32F3 : public ISPI
{
public:
    SpiSTM32F3(uint32_t base_address, uint32_t clock_speed, uint8_t clock_polarity, uint8_t clock_phase);

    void Initialize() override;

    void Deinitialize() override;

    SPI_HandleTypeDef* GetHandle() { return &spiHandle; }

    bool Write(std::span<std::byte> data_out, uint8_t num_bytes) override;

    bool Transfer(std::span<std::byte> data_out, std::span<std::byte> data_in, uint8_t num_bytes) override;

    bool WriteNonBlocking(std::span<const std::byte> data_out,
                          uint8_t num_bytes,
                          CallbackWrite callback,
                          void* user_data) override;

    bool TransferNonBlocking(std::span<const std::byte> data_out,
                             std::span<std::byte> data_in,
                             uint8_t num_bytes,
                             CallbackTransfer callback,
                             void* user_data) override;

    void ClearReceiveBuffer();

    friend void SPI1InterruptHandler();
    friend void SPI2InterruptHandler();
    friend void SPI3InterruptHandler();

protected:
    enum Blocking : bool
    {
        non_blocking = false,
        blocking = true
    };

    struct Transaction
    {
        std::span<std::byte> data_in;
        size_t bytes_in;
        std::span<const std::byte> data_out;
        size_t bytes_out;
        size_t size;
        std::vector<std::byte> data_out_nb;
    };

    static constexpr size_t num_spi_ports{3};
    static SpiSTM32F3* spi_ports[num_spi_ports];

    const uint32_t spi_base_address;  // base address of SPI bus

    CallbackWrite callback_function_write{nullptr};
    CallbackTransfer callback_function_transfer{nullptr};

    Transaction transaction;

    Blocking is_blocking{blocking};

    void* callbackContext{nullptr};
    bool is_write_only{false};

    static uint32_t GetInterruptNumber(uint32_t spi_base_address);
    static void handle_interrupt(size_t port_index);

    virtual void isr();

    uint8_t GetClkPolarity() const;
    uint8_t GetClkPhase() const;

private:
    const uint32_t clock_speed;
    const uint8_t clock_polarity;
    const uint8_t clock_phase;

    SPI_HandleTypeDef spiHandle;

    bool startTransaction(std::span<const std::byte> data_out);
    bool DoTransaction(std::span<const std::byte> data_out, std::span<std::byte> data_in, uint8_t num_bytes);
};
}  // namespace fw
