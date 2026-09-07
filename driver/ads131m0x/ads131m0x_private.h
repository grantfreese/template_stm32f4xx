// ads131m0x_private.h - Private header for ADS131M0x ADC driver
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "hal/io.h"

// lib includes

// os includes

// standard includes
#include <cstdint>
#include <type_traits>

namespace fw
{
/// @brief  Device list for ADS131M0x-family ADCs
enum class Ads131m0xDevice : int
{
    kADS131M04 = 4,
    kADS131M08 = 8
};

/// @brief Command words for ADS131M0x-family ADCs
/// Data source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-11
enum class Ads131m0xCommandWord : uint16_t
{
    // clang-format off
    // Note: The 16-bit values in this enum will be left-aligned in the 24-bit command frame
    kNull    = 0x0000,            //@ No operation, returns STATUS register
    kReset   = 0x0011,            //@ Resets device
    kStandby = 0x0022,            //@ Puts device into standby mode
    kWakeup  = 0x0033,            //@ Wakes device from standby mode to conversion mode
    kLock    = 0x0555,            //@ locks interface such that only NULL, UNLOCK, and RREG commands are valid
    kUnlock  = 0x0655,            //@ Unlocks interface from locked state
    kRReg    = 0xa000,            //@ Read register. NOTE: read command word also needs reg addr and count OR'd to it
    kWReg    = 0x6000,            //@ Write register. NOTE: write command word also needs reg addr and count OR'd to it
    kWRegResponseMask = 0x5F80  //@ Mask to validate write response
};

/// @brief  Register access-levels
enum class AccessLevel
{
    kReadOnly,
    kWriteOnly,
    kReadWrite
};

/// @brief  Register addresses for ADS131M0x-family ADCs
/// Data source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-12
enum class RegAddr : uint8_t
{
    kId     = 0x00,
    kStatus = 0x01,
    kMode   = 0x02,
    kClock  = 0x03,
    kGain1  = 0x04,
    kGain2  = 0x05,
    kCfg    = 0x06,
    kThrshldMsb = 0x07,
    kThrshldLsb = 0x08,

    kCh0Cfg     = 0x09,
    kCh0OCalMsb = 0x0A,
    kCh0OCalLsb = 0x0B,
    kCh0GCalMsb = 0x0C,
    kCh0GCalLsb = 0x0D,

    kCh1Cfg     = 0x0E,
    kCh1OCalMsb = 0x0F,
    kCh1OCalLsb = 0x10,
    kCh1GCalMsb = 0x11,
    kCh1GCalLsb = 0x12,

    kCh2Cfg     = 0x13,
    kCh2OCalMsb = 0x14,
    kCh2OCalLsb = 0x15,
    kCh2GCalMsb = 0x16,
    kCh2GCalLsb = 0x17,

    kCh3Cfg     = 0x18,
    kCh3OCalMsb = 0x19,
    kCh3OCalLsb = 0x1A,
    kCh3GCalMsb = 0x1B,
    kCh3GCalLsb = 0x1C,

    kCh4Cfg     = 0x1D,
    kCh4OCalMsb = 0x1E,
    kCh4OCalLsb = 0x1F,
    kCh4GCalMsb = 0x20,
    kCh4GCalLsb = 0x21,

    kCh5Cfg     = 0x22,
    kCh5OCalMsb = 0x23,
    kCh5OCalLsb = 0x24,
    kCh5GCalMsb = 0x25,
    kCh5GCalLsb = 0x26,

    kCh6Cfg     = 0x27,
    kCh6OCalMsb = 0x28,
    kCh6OCalLsb = 0x29,
    kCh6GCalMsb = 0x2A,
    kCh6GCalLsb = 0x2B,

    kCh7Cfg     = 0x2C,
    kCh7OCalMsb = 0x2D,
    kCh7OCalLsb = 0x2E,
    kCh7GCalMsb = 0x2F,
    kCh7GCalLsb = 0x30,

    kRegmapCrc = 0x3E
};
// clang-format on

/// @brief Gain settings for ADS131M0x-family ADCs
/// Data source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-18
enum class AdcGain : uint32_t
{
    kGain1 = 0,
    kGain2 = 1,
    kGain4 = 2,
    kGain8 = 3,
    kGain16 = 4,
    kGain32 = 5,
    kGain64 = 6,
    kGain128 = 7,
};

/// @brief Channel settings for ADS131M0x-family ADCs
enum class AdcChannel
{
    kChan0,
    kChan1,
    kChan2,
    kChan3,
    kChan4,
    kChan5,
    kChan6,
    kChan7
};

/// @brief converts strongly-typed enums to underlying datatype
/// @return underlying datatype
template <typename E>
constexpr auto GetUnderlyingType(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

/// @brief Convert AdcGain enum power-of-two value to gain reciprocal to avoid division during conversion
/// @param gain AdcGain enum value
/// @return gain multiple
constexpr float AdcGainToGainReciprocal(AdcGain gain)
{
    switch (gain)
    {
        case AdcGain::kGain1:
            return 1 / 1.0f;
        case AdcGain::kGain2:
            return 1 / 2.0f;
        case AdcGain::kGain4:
            return 1 / 4.0f;
        case AdcGain::kGain8:
            return 1 / 8.0f;
        case AdcGain::kGain16:
            return 1 / 16.0f;
        case AdcGain::kGain32:
            return 1 / 32.0f;
        case AdcGain::kGain64:
            return 1 / 64.0f;
        case AdcGain::kGain128:
            return 1 / 128.0f;
        default:
            return 0.0f;
    }
}

}  // namespace fw
