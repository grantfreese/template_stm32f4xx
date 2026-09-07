// ads131m0x_reg_mode.h - Mode register for ADS131M0x ADC driver
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
/// @brief Register CRC enable
enum class ModeRegisterCrcEnable
{
    kDisabled = 0,
    kEnabled = 1,
};

/// @brief RX CRC enable
enum class ModeReceiveCrcEnable
{
    kDisabled = 0,
    kEnabled = 1,
};

/// @brief CRC type selection
enum class ModeCrcType
{
    kCcitt16Bit = 0,
    kAnsi16Bit = 1,
};

/// @brief Reset-has-occurred flag
enum class ModeReset
{
    kNoReset = 0,
    kReset = 1,
};

/// @brief Data word length selection
enum class ModeWordLength
{
    kSize16Bits = 0,
    kSize24Bits = 1,
    kSize32BitsLsbZeroPadding = 2,
    kSize32BitsMsbSignExtension = 3,
};

/// @brief SPI timeout enable/disable
enum class ModeTimeoutEnable
{
    kDisabled = 0,
    kEnabled = 1,
};

/// @brief DRDY pin signal source selection
enum class ModeDataReadySourceSelect
{
    kMostLaggingEnabledChannel = 0,
    kLogicalOrOfEnabledChannel = 1,
    kMostLeadingEnabledChannel = 2,
};

/// @brief DRDY pin state when conversion data is not available
enum class ModeDataReadyHighZ
{
    kLogicHigh = 0,
    kHighImpedance = 1,
};

/// @brief DRDY signal format when conversion data is available
enum class ModeDataReadyFormat
{
    kLogicLow = 0,
    kLowPulseFixedDuration = 1,
};

// disable bitpacking so we can perform type punning on the struct
#pragma pack(push, 1)
typedef struct
{
    ModeDataReadyFormat data_ready_format : 1;
    ModeDataReadyHighZ data_ready_high_z : 1;
    ModeDataReadySourceSelect data_ready_source_select : 2;
    ModeTimeoutEnable timeout_enable : 1;
    int : 3;
    ModeWordLength word_length : 2;
    ModeReset reset : 1;
    ModeCrcType crc_type : 1;
    ModeReceiveCrcEnable receive_crc_enable : 1;
    ModeRegisterCrcEnable register_crc_enable : 1;
    int : 2;
} ModeBits;
#pragma pack(pop)

class RegisterMode : public Adsm131m0xRegister<ModeBits, RegAddr::kMode, AccessLevel::kReadWrite>
{
public:
    RegisterMode(IAds131m0xSpi& spi_writer, uint16_t register_check_mask)
        : Adsm131m0xRegister(spi_writer, register_check_mask)
    {
    }
    /// @brief Clear reset flag (flag is set during reset)
    void ClearResetFlag() { this->bitfield_.reset = ModeReset::kNoReset; }

    /// @brief Disable Register CRC
    void DisableRegisterCrc() { this->bitfield_.register_crc_enable = ModeRegisterCrcEnable::kDisabled; }

    /// @brief Enable Register CRC
    void EnableRegisterCrc() { this->bitfield_.register_crc_enable = ModeRegisterCrcEnable::kEnabled; }

    /// @brief Disable RX CRC
    void DisableRxCrc() { this->bitfield_.receive_crc_enable = ModeReceiveCrcEnable::kDisabled; }

    /// @brief Enable RX CRC
    void EnableRxCrc() { this->bitfield_.receive_crc_enable = ModeReceiveCrcEnable::kEnabled; }

    /// @brief Sets the CRC type for SPI communications
    /// @param type Type of CRC (CCITT-16 or ANSI-16)
    void SetCrcType(ModeCrcType type) { this->bitfield_.crc_type = type; }

    /// @brief  Sets the data word length for SPI communications
    /// @param  length Data word length (16 bit, 24 bit, 32 bit left-aligned, or 32 bit right-aligned)
    void SetWordLength(ModeWordLength length) { this->bitfield_.word_length = length; }

    /// @brief Sets timeout enable/disable for SPI communications
    /// @param timeout Timeout enable/disable
    void SetTimeout(ModeTimeoutEnable timeout) { this->bitfield_.timeout_enable = timeout; }

    /// @brief Sets DRDY pin signal source selection
    /// @param source DRDY pin signal source selection (most lagging, logical OR, or most leading)
    void SetDataReadySourceSelect(ModeDataReadySourceSelect source)
    {
        this->bitfield_.data_ready_source_select = source;
    }

    /// @brief Sets DRDY pin state when conversion data is not available
    /// @param state DRDY pin state (logic high or high impedance)
    void SetDataReadyHighZ(ModeDataReadyHighZ state) { this->bitfield_.data_ready_high_z = state; }

    /// @brief Sets DRDY pin state when conversion data is available
    /// @param state DRDY pin state (logic low or low pulse fixed duration)
    void SetDataReadyFormat(ModeDataReadyFormat state) { this->bitfield_.data_ready_format = state; }

    /// @brief  Returns true when CRC is enabled for RX SPI comms
    bool IsCrcRxEnabled() { return GetUnderlyingType(this->bitfield_.receive_crc_enable); }

    /// @brief  Returns true when CRC is enabled for TX SPI comms
    bool IsCrcRegisterEnabled() { return GetUnderlyingType(this->bitfield_.register_crc_enable); }
};  // class RegisterMode

}  // namespace fw
