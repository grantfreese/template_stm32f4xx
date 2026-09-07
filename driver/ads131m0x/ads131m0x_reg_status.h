// ads131m0x_reg_status.h - Status register for ADS131M0x ADC driver
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
/// @brief Data read bit status
enum class StatusDataReady
{
    kNoNewData = 0,
    kNewData = 1,
};

/// @brief Data word length
enum class StatusWordLength
{
    kSize16Bits = 0,
    kSize24Bits = 1,
    kSize32BitsLsbZeroPadding = 2,
    kSize32BitsMsbSignExtension = 3,
};

/// @brief Reset-has-occurred flag
enum class StatusReset
{
    kNoReset = 0,
    kReset = 1,
};

/// @brief CRC type
enum class StatusCrcType
{
    kCcitt16Bit = 0,
    kAnsi16Bit = 1,
};

/// @brief RX CRC error flag
enum class StatusCrcError
{
    kNoCrcError = 0,
    kCrcError = 1,
};

/// @brief Register map changed flag
enum class StatusRegisterMap
{
    kCrcUnchanged = 0,
    kCrcChanged = 1,
};

/// @brief Frame loss-of-sync flag
enum class StatusLossOfSyncFlag
{
    kNoFlag = 0,
    kFlag = 1,
};

/// @brief Register lock status
enum class StatusLock
{
    kUnlocked = 0,
    kLocked = 1,
};

// disable bitpacking so we can perform type punning on the struct
#pragma pack(push, 1)
typedef struct
{
    // magic number: ADS131M08 datasheet SBAS950B, p. 51, Table 8-15
    StatusDataReady data_ready_0 : 1;
    StatusDataReady data_ready_1 : 1;
    StatusDataReady data_ready_2 : 1;
    StatusDataReady data_ready_3 : 1;
    StatusDataReady data_ready_4 : 1;
    StatusDataReady data_ready_5 : 1;
    StatusDataReady data_ready_6 : 1;
    StatusDataReady data_ready_7 : 1;
    StatusWordLength word_length : 2;
    StatusReset reset : 1;
    StatusCrcType crc_type : 1;
    StatusCrcError crc_error : 1;
    StatusRegisterMap register_map : 1;
    StatusLossOfSyncFlag flag_resync : 1;
    StatusLock lock : 1;
} StatusBits;
#pragma pack(pop)

class RegisterStatus : public Adsm131m0xRegister<StatusBits, RegAddr::kStatus, AccessLevel::kReadOnly>
{
public:
    RegisterStatus(IAds131m0xSpi& spi_writer, uint16_t register_check_mask)
        : Adsm131m0xRegister(spi_writer, register_check_mask)
    {
    }
};  // class RegisterStatus

}  // namespace fw
