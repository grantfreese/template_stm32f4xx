// uart_ring_drain.h - host-testable ring-buffer drain helper
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
//
// Split out of stm32f3xx_uart so the index walk can be exercised by a unit
// test without pulling in stm32f3xx_hal.h (which only compiles for ARM).
// UartSTM32F3::DrainRx delegates the byte walk here.

#pragma once

#include <cstddef>
#include <cstdint>

namespace fw
{
using UartRxByteHandler = void (*)(uint8_t byte, void* user_data);

//! @brief Walk @p ring from @p read_index_inout up to @p write_index (modulo
//! @p ring_size), invoking @p handler for each byte. Updates the read index
//! and returns the count drained. The DMA-fed caller computes write_index
//! as (ring_size - DMA::CNDTR).
inline std::size_t DrainRingTo(const std::uint8_t* ring,
                               std::size_t ring_size,
                               std::size_t& read_index_inout,
                               std::size_t write_index,
                               UartRxByteHandler handler,
                               void* user_data)
{
    std::size_t drained = 0;
    while (read_index_inout != write_index)
    {
        const std::uint8_t byte = ring[read_index_inout];
        read_index_inout = (read_index_inout + 1) % ring_size;
        if (handler != nullptr)
        {
            handler(byte, user_data);
        }
        ++drained;
    }
    return drained;
}

}  // namespace fw
