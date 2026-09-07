// stm32f3xx_spi.cpp - SPI class implementation for stm32f3xx
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "stm32f3xx_spi.h"

// lib includes
#include "framework/error.h"
#include "stm32f3xx_hal.h"

// os includes

// standard includes
#include <stdbool.h>
#include <stdint.h>

#include <cstring>
#include <memory>
#include <span>

namespace fw
{
SpiSTM32F3* SpiSTM32F3::spi_ports[num_spi_ports];

void SPI1InterruptHandler() { SpiSTM32F3::handle_interrupt(1); }

void SPI2InterruptHandler() { SpiSTM32F3::handle_interrupt(2); }

void SPI3InterruptHandler() { SpiSTM32F3::handle_interrupt(3); }

SpiSTM32F3::SpiSTM32F3(uint32_t base_address, uint32_t clock_speed, uint8_t clock_polarity, uint8_t clock_phase)
    : spi_base_address{base_address}, clock_speed{clock_speed}, clock_polarity{clock_polarity}, clock_phase{clock_phase}
{
}

// Pick the smallest BaudRatePrescaler enum value such that pclk / 2^(BR+1) <= target.
// Falls back to /256 (the slowest available) when no divider satisfies the bound,
// which protects a downstream peripheral from overshoot at the cost of running
// slower than requested.
static uint32_t SelectBaudRatePrescaler(uint32_t pclk, uint32_t target)
{
    static constexpr uint32_t kPrescalerEnum[] = {
        SPI_BAUDRATEPRESCALER_2,
        SPI_BAUDRATEPRESCALER_4,
        SPI_BAUDRATEPRESCALER_8,
        SPI_BAUDRATEPRESCALER_16,
        SPI_BAUDRATEPRESCALER_32,
        SPI_BAUDRATEPRESCALER_64,
        SPI_BAUDRATEPRESCALER_128,
        SPI_BAUDRATEPRESCALER_256,
    };
    for (uint32_t i = 0; i < sizeof(kPrescalerEnum) / sizeof(kPrescalerEnum[0]); i++)
    {
        const uint32_t sck = pclk >> (i + 1);
        if (sck <= target)
        {
            return kPrescalerEnum[i];
        }
    }
    return SPI_BAUDRATEPRESCALER_256;
}

void SpiSTM32F3::Initialize()
{
    GPIO_InitTypeDef GPIO_InitStruct;

    switch (spi_base_address)
    {
        case SPI1_BASE:
            spi_ports[0] = this;  // static SPI class handles

            // SPI peripheral settings
            spiHandle.Instance = SPI1;
            spiHandle.Init.Mode = SPI_MODE_MASTER;
            spiHandle.Init.Direction = SPI_DIRECTION_2LINES;
            spiHandle.Init.DataSize = SPI_DATASIZE_8BIT;
            spiHandle.Init.CLKPolarity = clock_polarity * 2;  // 0 = SPI_POLARITY_LOW, 2 = SPI_POLARITY_HIGH
            spiHandle.Init.CLKPhase = clock_phase;
            spiHandle.Init.NSS = SPI_NSS_SOFT;
            // SPI1 is on APB2 -> PCLK2.
            spiHandle.Init.BaudRatePrescaler = SelectBaudRatePrescaler(HAL_RCC_GetPCLK2Freq(), clock_speed);
            spiHandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
            spiHandle.Init.TIMode = SPI_TIMODE_DISABLE;
            spiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
            spiHandle.Init.CRCPolynomial = 7;
            spiHandle.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
            spiHandle.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

            __HAL_RCC_SPI1_CLK_ENABLE();
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_GPIOB_CLK_ENABLE();

            // init SPI pins
            GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = GPIO_PIN_0;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
            break;

        case SPI2_BASE:
            spi_ports[1] = this;  // static SPI class handles

            // SPI peripheral settings
            spiHandle.Instance = SPI2;
            spiHandle.Init.Mode = SPI_MODE_MASTER;
            spiHandle.Init.Direction = SPI_DIRECTION_2LINES;
            spiHandle.Init.DataSize = SPI_DATASIZE_8BIT;
            spiHandle.Init.CLKPolarity = clock_polarity * 2;  // 0 = SPI_POLARITY_LOW, 2 = SPI_POLARITY_HIGH
            spiHandle.Init.CLKPhase = clock_phase;
            spiHandle.Init.NSS = SPI_NSS_SOFT;
            // SPI2 is on APB1 -> PCLK1.
            spiHandle.Init.BaudRatePrescaler = SelectBaudRatePrescaler(HAL_RCC_GetPCLK1Freq(), clock_speed);
            spiHandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
            spiHandle.Init.TIMode = SPI_TIMODE_DISABLE;
            spiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
            spiHandle.Init.CRCPolynomial = 7;
            spiHandle.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
            spiHandle.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

            __HAL_RCC_SPI2_CLK_ENABLE();
            __HAL_RCC_GPIOB_CLK_ENABLE();
            __HAL_RCC_GPIOD_CLK_ENABLE();

            // init SPI pins
            GPIO_InitStruct.Pin = GPIO_PIN_8;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
            HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
            break;

        case SPI3_BASE:
            spi_ports[2] = this;
            break;
    }

    if (HAL_SPI_Init(&spiHandle) != HAL_OK)
    {
        Error_Handler();
    }

    // send dummy packet to put pins in correct state for SPI mode
    HAL_SPI_Transmit(&spiHandle, (uint8_t*)0, 1, 0);
}

void SpiSTM32F3::Deinitialize()
{
    // TODO: implement
}

bool SpiSTM32F3::Write(std::span<std::byte> data_out, uint8_t num_bytes)
{
    HAL_StatusTypeDef spiTxStatus =
        HAL_SPI_Transmit(&spiHandle, reinterpret_cast<uint8_t*>(data_out.data()), num_bytes, HAL_MAX_DELAY);
    return spiTxStatus == HAL_OK;
}

bool SpiSTM32F3::Transfer(std::span<std::byte> data_out, std::span<std::byte> data_in, uint8_t num_bytes)
{
    HAL_StatusTypeDef spiTxStatus = HAL_SPI_TransmitReceive(&spiHandle,
                                                            reinterpret_cast<uint8_t*>(data_out.data()),
                                                            reinterpret_cast<uint8_t*>(data_in.data()),
                                                            num_bytes,
                                                            HAL_MAX_DELAY);
    return spiTxStatus == HAL_OK;
}

bool SpiSTM32F3::WriteNonBlocking(std::span<const std::byte> data_out,
                                  uint8_t num_bytes,
                                  CallbackWrite callback,
                                  void* callbackContext)
{
    bool success = true;

    if (HAL_SPI_GetState(&spiHandle) != HAL_SPI_STATE_READY || data_out.empty() || num_bytes == 0)
    {
        // SPI Bus is busy with another transaction or some arguments have
        // invalid values. Cannot write
        success = false;
    }
    else
    {  // SPI Bus is available

        // Flag transaction as a write operation
        is_write_only = true;

        // Store Transaction Information
        transaction.bytes_out = 0;
        transaction.size = num_bytes;
        callback_function_write = callback;
        this->callbackContext = callbackContext;

        success = startTransaction(data_out);
    }

    return success;
}

bool SpiSTM32F3::TransferNonBlocking(std::span<const std::byte> data_out,
                                     std::span<std::byte> data_in,
                                     uint8_t num_bytes,
                                     CallbackTransfer callback,
                                     void* callbackContext)
{
    bool success = true;

    if (HAL_SPI_GetState(&spiHandle) != HAL_SPI_STATE_READY || data_out.empty() || num_bytes == 0
        || callback == nullptr)
    {
        // SPI Bus is busy with another transaction or some arguments have
        // invalid values. Cannot write
        success = false;
    }
    else
    {
        // Flag transaction as a transfer operation
        is_write_only = false;

        // Store Transaction Information
        transaction.bytes_in = 0;
        transaction.bytes_out = 0;
        transaction.data_in = data_in;
        transaction.size = num_bytes;
        callback_function_transfer = callback;
        this->callbackContext = callbackContext;

        // Make sure RX buffer is empty
        ClearReceiveBuffer();

        success = startTransaction(data_out);
    }

    return success;
}

void SpiSTM32F3::ClearReceiveBuffer() { NotImplementedException(); }

uint32_t SpiSTM32F3::GetInterruptNumber(uint32_t spi_base_address)
{
    switch (spi_base_address)
    {
        case SPI1_BASE:
            return SPI1_IRQn;
        case SPI2_BASE:
            return SPI2_IRQn;
        case SPI3_BASE:
            return SPI3_IRQn;
        default:
            runtime_assert(false, "Unknown SPI Base address");
            return SPI1_IRQn;
    }
}

void SpiSTM32F3::handle_interrupt(size_t port_index)
{
    if (port_index < num_spi_ports)  // port_index is unsigned, so no need to check for negative values
    {
        auto port = spi_ports[port_index];
        if (!port)
        {
            return;
        }
        port->isr();
    }
}

uint8_t SpiSTM32F3::GetClkPolarity() const { return clock_polarity; }

uint8_t SpiSTM32F3::GetClkPhase() const { return clock_phase; }

// ************ private methods ************

bool SpiSTM32F3::startTransaction(std::span<const std::byte> data_out)
{
    bool success = true;

    (void)data_out;  // TODO: temporary to disable warning

    // // Flag transaction as non-blocking
    // is_blocking = Blocking::non_blocking;

    // // Check if the transaction will exceed the FIFO's size
    // if (transaction.size > tiva_fifo_size)
    // {
    //     // Make sure the FIFO half full interrupt is active
    //     HWREG(spi_base_address + SPI_O_CR1) &= ~SPI_CR1_EOT;

    //     // Disable global interrupts
    //     bool already_disabled = IntMasterDisable();

    //     // Enable the TX interrupt
    //     SPIIntEnable(spi_base_address, SPI_TXFF);

    //     // Limit the number of bytes we can put on the FIFO to avoid losing data on the RX FIFO
    //     const size_t kDataPutLimit{(is_write_only ? transaction.size : tiva_fifo_size)};
    //     // Write the first chunk of data
    //     while (
    //         (transaction.bytes_out < kDataPutLimit)
    //         && ((bool)SPIDataPutNonBlocking(spi_base_address,
    //         static_cast<uint32_t>(data_out[transaction.bytes_out]))))
    //     {
    //         transaction.bytes_out++;
    //     }

    //     /*  If the number of bytes exceeds the FIFO's size, we copy
    //         the data that did not fit in case that the caller's scope ends
    //         while the transaction is ongoing. */
    //     if (transaction.bytes_out < transaction.size)
    //     {
    //         std::memcpy(&transaction.data_out_nb[transaction.bytes_out],
    //                     &data_out[transaction.bytes_out],
    //                     (transaction.size - transaction.bytes_out) * sizeof(std::byte));
    //     }

    //     // If global interrupts were not disabled previously in the function stack,
    //     // re-enable them
    //     if (!already_disabled)
    //     {
    //         IntMasterEnable();
    //     }

    //     success = (bool)transaction.bytes_out;
    // }
    // else
    // {
    //     // Enable interrupt for end of transmission
    //     HWREG(spi_base_address + SPI_O_CR1) |= SPI_CR1_EOT;

    //     // Disable global interrupts
    //     bool already_disabled = IntMasterDisable();

    //     // Enable the TX interrupt
    //     SPIIntEnable(spi_base_address, SPI_TXFF);

    //     // We can write all of the data into the FIFO
    //     for (; transaction.bytes_out < transaction.size; transaction.bytes_out++)
    //     {
    //         if (!((bool)SPIDataPutNonBlocking(spi_base_address,
    //                                           static_cast<uint32_t>(data_out[transaction.bytes_out]))))
    //         {
    //             success = false;
    //         }
    //     }

    //     // If global interrupts were not disabled previously in the function stack,
    //     // re-enable them
    //     if (!already_disabled)
    //     {
    //         IntMasterEnable();
    //     }
    // }

    return success;
}

void SpiSTM32F3::isr(void)
{
    // // Process Interrupts for TX EOT
    // if (HWREG(spi_base_address + SPI_O_CR1) & SPI_CR1_EOT)
    // {
    //     // It is the End of Transmission -> Disable TX interrupt (callback may
    //     // re-enable it)
    //     SPIIntDisable(spi_base_address, SPI_TXFF);

    //     // Clear interrupt
    //     // note - you can't clear the TX interrupt (HW controlled)

    //     // Write Transaction
    //     if (is_write_only)
    //     {
    //         // Flush the RX buffer
    //         ClearReceiveBuffer();

    //         // Send a callback to user
    //         if (callback_function_write != nullptr)
    //         {
    //             callback_function_write(callbackContext);
    //         }
    //     }
    //     // Transfer Transaction
    //     else
    //     {
    //         uint32_t transaction_data{0};
    //         // Read remaining data from RX FIFO
    //         while (transaction.bytes_in < transaction.size
    //                && (bool)(SPIDataGetNonBlocking(spi_base_address, &transaction_data)))
    //         {
    //             transaction.data_in[transaction.bytes_in++] = gsl::narrow_cast<std::byte>(transaction_data);
    //         }

    //         // Send callback to user
    //         callback_function_transfer(transaction.data_in, transaction.size, callbackContext);
    //     }
    // }

    // // Process Interrupt for TX FIFO Half Full or less
    // if (!(HWREG(spi_base_address + SPI_O_CR1) & SPI_CR1_EOT))
    // {
    //     // Clear interrupt
    //     // note - you can't clear the TX interrupt (HW controlled)

    //     // Read the RX buffer if this is a transfer transaction
    //     if (!is_write_only)
    //     {
    //         uint32_t transaction_data{0};
    //         while ((transaction.bytes_in < transaction.size)
    //                && ((bool)SPIDataGetNonBlocking(spi_base_address, &transaction_data)))
    //         {
    //             transaction.data_in[transaction.bytes_in++] = gsl::narrow_cast<std::byte>(transaction_data);
    //         }
    //     }
    //     else  // Flush the RX buffer
    //     {
    //         ClearReceiveBuffer();
    //     }

    //     // Limit the number of bytes we can put on the FIFO to avoid losing data on the RX FIFO
    //     const size_t kDataPutLimit{
    //         (is_write_only ? transaction.size : std::min(transaction.size, (transaction.bytes_out +
    //         tiva_fifo_size)))};
    //     // Load the FIFO with remaining data
    //     while ((transaction.bytes_out < kDataPutLimit)
    //            && ((bool)SPIDataPutNonBlocking(spi_base_address,
    //                                            static_cast<uint32_t>(transaction.data_out_nb[transaction.bytes_out]))))

    //     {
    //         transaction.bytes_out++;
    //     }

    //     // If all data has been loaded, enable the EOT interrupt
    //     if (transaction.bytes_out == transaction.size)
    //     {
    //         HWREG(spi_base_address + SPI_O_CR1) |= SPI_CR1_EOT;
    //     }
    // }
}
}  // namespace fw
