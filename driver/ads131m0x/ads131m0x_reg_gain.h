// ads131m0x_reg_gain.h - Gain register for ADS131M0x ADC driver
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "ads131m0x_private.h"
#include "ads131m0x_register.h"

// lib includes
#include "framework/error.h"

// os includes

// standard includes
#include <array>

namespace fw
{
/// @brief GAIN1 register bitfield definitions
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-18
// disable bitpacking so we can perform type punning on the struct
#pragma pack(push, 1)
typedef struct
{
    AdcGain gain_0 : 3;
    int : 1;
    AdcGain gain_1 : 3;
    int : 1;
    AdcGain gain_2 : 3;
    int : 1;
    AdcGain gain_3 : 3;
    int : 1;
} Gain1Bits;
#pragma pack(pop)

/// @brief GAIN2 register bitfield definitions
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-19
// disable bitpacking so we can perform type punning on the struct
#pragma pack(push, 1)
typedef struct
{
    AdcGain gain_4 : 3;
    int : 1;
    AdcGain gain_5 : 3;
    int : 1;
    AdcGain gain_6 : 3;
    int : 1;
    AdcGain gain_7 : 3;
    int : 1;
} Gain2Bits;
#pragma pack(pop)

/// @brief GAIN1 register
class RegisterGain1 : public Adsm131m0xRegister<Gain1Bits, RegAddr::kGain1, AccessLevel::kReadWrite>
{
public:
    RegisterGain1(IAds131m0xSpi& spi_writer, uint16_t register_check_mask)
        : Adsm131m0xRegister(spi_writer, register_check_mask)
    {
    }
    /// @brief Set gain for ADC channel
    /// @param channel ADC channel
    /// @param gain Gain setting
    void SetGain(AdcChannel channel, AdcGain gain)
    {
        switch (channel)
        {
            case AdcChannel::kChan0:
                this->bitfield_.gain_0 = gain;
                break;
            case AdcChannel::kChan1:
                this->bitfield_.gain_1 = gain;
                break;
            case AdcChannel::kChan2:
                this->bitfield_.gain_2 = gain;
                break;
            case AdcChannel::kChan3:
                this->bitfield_.gain_3 = gain;
                break;
            default:
                NotSupportedException("Invalid gain setting");
                break;
        }
    }

    /// @brief Get gain for all ADC channels
    /// @return Array of gain values for four channels
    std::array<uint32_t, 4> GetGain()
    {
        std::array<uint32_t, 4> gain_array = {1U << GetUnderlyingType(this->bitfield_.gain_0),
                                              1U << GetUnderlyingType(this->bitfield_.gain_1),
                                              1U << GetUnderlyingType(this->bitfield_.gain_2),
                                              1U << GetUnderlyingType(this->bitfield_.gain_3)};
        return gain_array;
    }
};  // class RegisterGain1

/// @brief GAIN2 register
class RegisterGain2 : public Adsm131m0xRegister<Gain2Bits, RegAddr::kGain2, AccessLevel::kReadWrite>
{
public:
    RegisterGain2(IAds131m0xSpi& spi_writer, uint16_t register_check_mask)
        : Adsm131m0xRegister(spi_writer, register_check_mask)
    {
    }
    /// @brief Set gain for ADC channel
    /// @param channel ADC channel
    /// @param gain Gain setting
    void SetGain(AdcChannel channel, AdcGain gain)
    {
        switch (channel)
        {
            case AdcChannel::kChan4:
                this->bitfield_.gain_4 = gain;
                break;
            case AdcChannel::kChan5:
                this->bitfield_.gain_5 = gain;
                break;
            case AdcChannel::kChan6:
                this->bitfield_.gain_6 = gain;
                break;
            case AdcChannel::kChan7:
                this->bitfield_.gain_7 = gain;
                break;
            default:
                NotSupportedException("Invalid gain setting");
                break;
        }
    }
    std::array<uint32_t, 4> GetGain()
    {
        std::array<uint32_t, 4> gain_array = {1U << GetUnderlyingType(this->bitfield_.gain_4),
                                              1U << GetUnderlyingType(this->bitfield_.gain_5),
                                              1U << GetUnderlyingType(this->bitfield_.gain_6),
                                              1U << GetUnderlyingType(this->bitfield_.gain_7)};
        return gain_array;
    }
};  // class RegisterGain2

}  // namespace fw
