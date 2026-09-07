// spi.h - HAL abstractions for SPI signals
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes

// lib includes

// os includes

// standard includes
#include <cstdint>
#include <span>

namespace fw
{
typedef void (*CallbackWrite)(void* user_data);
typedef void (*CallbackTransfer)(std::span<const uint8_t> data_in, uint8_t num_bytes, void* user_data);

class ISPI
{
public:
    // virtual methods
    virtual void Initialize() = 0;

    virtual void Deinitialize() = 0;

    virtual bool Write(std::span<std::byte> data_out, uint8_t num_bytes) = 0;

    virtual bool Transfer(std::span<std::byte> data_out, std::span<std::byte> data_in, uint8_t num_bytes) = 0;

    virtual bool WriteNonBlocking(std::span<const std::byte> data_out,
                                  uint8_t num_bytes,
                                  CallbackWrite callback,
                                  void* user_data) = 0;

    virtual bool TransferNonBlocking(std::span<const std::byte> data_out,
                                     std::span<std::byte> data_in,
                                     uint8_t num_bytes,
                                     CallbackTransfer callback,
                                     void* user_data) = 0;
};
}  // namespace fw
