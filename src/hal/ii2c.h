// ii2c.h - HAL abstraction for I2C signals
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// standard includes
#include <cstdint>
#include <span>

namespace fw
{
//! @brief Pure virtual interface for I2C bus operations.
//!
//! Models a single I2C master that can address one or more peripheral devices.
//! Concrete implementations encapsulate the underlying hardware (e.g. a native
//! STM32 I2C peripheral, or an I2C bridge on an external chip).
class II2C
{
public:
    virtual ~II2C() = default;

    //! @brief Perform a blocking I2C write transaction.
    //!
    //! Sends a START, the write address, one or more data bytes, and a STOP.
    //!
    //! @param address  7-bit I2C device address. The implementation appends the R/W bit.
    //! @param data     Bytes to transmit.
    //! @return True on success, false on NAK or other error.
    virtual bool Write(uint8_t address, std::span<const uint8_t> data) = 0;

    //! @brief Perform a blocking I2C read transaction.
    //!
    //! Sends a START, the read address, clocks in data bytes, and sends a STOP.
    //!
    //! @param address  7-bit I2C device address. The implementation appends the R/W bit.
    //! @param data     Buffer to receive bytes into.
    //! @return True on success, false on NAK or other error.
    virtual bool Read(uint8_t address, std::span<uint8_t> data) = 0;

    //! @brief Perform a combined write then repeated-start read transaction.
    //!
    //! Sends a START, write address, write data, repeated START, read address,
    //! clocks in read data, and sends a STOP.
    //!
    //! @param address     7-bit I2C device address. The implementation appends the R/W bit.
    //! @param write_data  Bytes to transmit (e.g. register address).
    //! @param read_data   Buffer to receive bytes into.
    //! @return True on success, false on NAK or other error.
    virtual bool WriteRead(uint8_t address, std::span<const uint8_t> write_data, std::span<uint8_t> read_data) = 0;
};
}  // namespace fw
