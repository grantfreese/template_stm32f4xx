// stm32f3xx_uart.h - USART class for stm32f3xx with DMA-backed RX
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "stm32f3xx_hal.h"
#include "uart_ring_drain.h"

namespace fw
{
extern "C"
{
    void USART1ErrorInterruptHandler();
}

//! @brief USART driver with a DMA-backed RX ring buffer.
//!
//! Replaces the byte-at-a-time HAL_UART_Receive_IT path, which lost bytes to
//! USART_ISR.ORE under sustained TX load (per-byte ISR couldn't complete
//! before the next RX byte arrived). DMA1 channel 5 copies USART_RDR into a
//! 512-byte ring continuously; the CLI task drains the ring on its own
//! cadence. TX polls TXE then writes TDR. HAL is used only for one-shot
//! RCC enables and GPIO AF setup; the hot path uses direct register access.
class UartSTM32F3
{
public:
    UartSTM32F3(USART_TypeDef* uart, uint32_t baud_rate);

    //! @brief Configure GPIO, clocks, USART, and start the DMA RX ring.
    //! Safe to call exactly once; idempotency is not required.
    void Initialize();

    using RxByteHandler = void (*)(uint8_t byte, void* user_data);

    //! @brief Drain bytes captured since the last call, invoking @p handler
    //! for each in receive order. Task-context only; not ISR-safe. Also
    //! clears any latched USART_ISR.ORE so a transient overrun does not
    //! permanently silence further RX.
    size_t DrainRx(RxByteHandler handler, void* user_data);

    //! @brief Blocking single-byte transmit: polls TXE, writes TDR.
    void WriteByte(uint8_t byte);

    //! @brief Blocking transmit of @p data byte by byte. Waits for TC on the
    //! final byte so callers can assume the line is idle on return.
    void Write(std::span<const uint8_t> data);

    //! @brief Latched overrun count. A healthy bench should stay at zero.
    uint32_t GetOverrunCount() const { return overrun_count_; }

    friend void USART1ErrorInterruptHandler();

private:
    static constexpr size_t kRxRingSize = 256;
    static UartSTM32F3* instance_usart1_;

    USART_TypeDef* const uart_;
    const uint32_t baud_rate_;

    //! DMA write position is derived from CNDTR at drain time.
    uint8_t rx_ring_[kRxRingSize]{};
    size_t rx_read_index_{0};

    DMA_Channel_TypeDef* rx_dma_channel_{nullptr};
    uint32_t rx_dma_clear_flags_{0};

    volatile uint32_t overrun_count_{0};

    void ConfigureRcc();
    void ConfigureGpio();
    void ConfigureUart();
    void ConfigureRxDma();
    void RegisterIsr();
    void HandleErrorIsr();
};

}  // namespace fw
