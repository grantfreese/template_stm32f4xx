// ads131m0x_reg_id.h - ID register for ADS131M0x ADC driver
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "ads131m0x_private.h"
#include "ads131m0x_register.h"

// lib includes

// os includes

// standard includes

namespace fw
{
// disable bitpacking so we can perform type punning on the struct
#pragma pack(push, 1)
typedef struct
{
    /// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-14
    int : 8;
    uint32_t channel_count : 4;
    uint32_t device_id : 4;
} IdBits;
#pragma pack(pop)

class RegisterId : public Adsm131m0xRegister<IdBits, RegAddr::kId, AccessLevel::kReadOnly>
{
public:
    RegisterId(IAds131m0xSpi& spi_writer, uint16_t register_check_mask)
        : Adsm131m0xRegister(spi_writer, register_check_mask)
    {
    }
    bool VerifyID(int channel_count)
    {
        return (this->bitfield_.device_id == kID) && (this->bitfield_.channel_count == channel_count);
    }

private:
    /// @brief ID register returns (0x02) to identify device
    /// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-14
    static constexpr int kID{0x02};
};  // class RegisterId

}  // namespace fw
