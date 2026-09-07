// ads131m0x_register.h - Register class for ADS131M0x ADC driver
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "ads131m0x_private.h"
#include "ads131m0x_spi.h"

// lib includes

// os includes

// standard includes
#include <cstdint>
#include <type_traits>

namespace fw
{
template <typename Tbitfield, RegAddr kAddress, AccessLevel kAccessLevel>
class Adsm131m0xRegister
{
public:
    Adsm131m0xRegister(IAds131m0xSpi& spi_writer, uint16_t register_check_mask)
        : spi_writer_{&spi_writer}, register_check_mask_{register_check_mask}
    {
    }

    /// @brief Pull register value from external device register and update bitfield struct
    void Read()
    {
        uint16_t tempval = spi_writer_->Read(kAddress);
        // memcpy for type punning
        memcpy(&bitfield_, &tempval, sizeof(bitfield_));
    }

    /// @brief Push value in register bitfield to external device register
    /// @return True on write success, false on write failure
    bool Write()
    {
        bool writeSuccess{spi_writer_->Write(kAddress, RegValue())};
        if (writeSuccess)
        {
            isDirty_ = false;
        }
        return writeSuccess;
    }

    /// @brief Write update to register if dirty bit is set
    /// @return True on 'write success' or 'no write needed', false on write failure
    bool Update()
    {
        bool writeSuccess{true};
        if (isDirty_)
        {
            writeSuccess = Write();
        }
        return writeSuccess;
    }

    /// @brief Convert and return bitfield as native register type (eg, as like <uint16_t> instead of struct)
    /// @return Register value as 16-bit type
    uint16_t RegValue() const
    {
        uint16_t tempval;
        memcpy(&tempval, &bitfield_, sizeof(tempval));  // type punning in C++ safe via memcpy
        return tempval;
    }

    /// @brief Check if local register value matches remote register value
    /// @return True when registers match, otherwise false
    bool CheckRegisterMatch()
    {
        // mask off read-only status bits
        uint16_t regLocal = RegValue() & register_check_mask_;
        Read();
        uint16_t regRemote = RegValue() & register_check_mask_;
        return regLocal == regRemote;
    }

    /// @brief Set register dirty bit
    void SetDirty() { isDirty_ = true; }

protected:
    Tbitfield bitfield_ = {};  // register bitfield
    bool isDirty_{false};      // flag to indicate if register has been modified since last write

private:
    IAds131m0xSpi* spi_writer_;     // SPI writer for ADS131M0x
    uint16_t register_check_mask_;  // mask bits which bits to check when comparing local and remote registers
};

}  // namespace fw
