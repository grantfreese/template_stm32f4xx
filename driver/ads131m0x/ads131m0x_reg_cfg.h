// ads131m0x_reg_cfg.h - Config register for ADS131M0x ADC driver
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
/// @brief Current-detect enable
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-20
enum class ConfigCurrentDetectEnable
{
    kDisabled = 0,
    kEnabled = 1,
};

/// @brief Current-detect length selection
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-20
enum class ConfigCurrentDetectLength
{
    kCurrentDetectLength_128 = 0,
    kCurrentDetectLength_256 = 1,
    kCurrentDetectLength_512 = 2,
    kCurrentDetectLength_768 = 3,
    kCurrentDetectLength_1280 = 4,
    kCurrentDetectLength_1792 = 5,
    kCurrentDetectLength_2560 = 6,
    kCurrentDetectLength_3584 = 7
};

/// @brief Number of current-detect exceeded thresholds to trigger a detection
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-20
enum class ConfigCurrentDetectNumber
{
    kCurrentDetectThreshold_1 = 0,
    kCurrentDetectThreshold_2 = 1,
    kCurrentDetectThreshold_4 = 2,
    kCurrentDetectThreshold_8 = 3,
    kCurrentDetectThreshold_16 = 4,
    kCurrentDetectThreshold_32 = 5,
    kCurrentDetectThreshold_64 = 6,
    kCurrentDetectThreshold_128 = 7
};

/// @brief Current-detect channels selection
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-20
enum class ConfigCurrentDetectChannelSelection
{
    kAnyChannel = 0,
    kAllChannels = 1,
};

/// @brief Global-chop enable
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-20
enum class ConfigGlobalChopEnable
{
    kDisabled = 0,
    kEnabled = 1,
};

/// @brief Global-chop delay selection
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-20
enum class ConfigGlobalChopDelay
{
    kGlobalChopDelay_2 = 0,
    kGlobalChopDelay_4 = 1,
    kGlobalChopDelay_8 = 2,
    kGlobalChopDelay_16 = 3,
    kGlobalChopDelay_32 = 4,
    kGlobalChopDelay_64 = 5,
    kGlobalChopDelay_128 = 6,
    kGlobalChopDelay_256 = 7,
    kGlobalChopDelay_512 = 8,
    kGlobalChopDelay_1024 = 9,
    kGlobalChopDelay_2048 = 10,
    kGlobalChopDelay_4096 = 11,
    kGlobalChopDelay_8192 = 12,
    kGlobalChopDelay_16384 = 13,
    kGlobalChopDelay_32768 = 14,
    kGlobalChopDelay_65536 = 15
};

/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-20
// disable bitpacking so we can perform type punning on the struct
#pragma pack(push, 1)
typedef struct
{
    ConfigCurrentDetectEnable current_detect_enable : 1;
    ConfigCurrentDetectLength current_detect_length : 3;
    ConfigCurrentDetectNumber current_detect_number : 3;
    ConfigCurrentDetectChannelSelection current_detect_channel_selection : 1;
    ConfigGlobalChopEnable global_chop_enable : 1;
    ConfigGlobalChopDelay global_chop_delay : 4;
    int : 3;
} ConfigBits;
#pragma pack(pop)

class RegisterConfig : public Adsm131m0xRegister<ConfigBits, RegAddr::kCfg, AccessLevel::kReadWrite>
{
public:
    RegisterConfig(IAds131m0xSpi& spi_writer, uint16_t register_check_mask)
        : Adsm131m0xRegister(spi_writer, register_check_mask)
    {
    }
    /// @brief Enable current-detect mode
    void EnableCurrentDetect() { this->bitfield_.current_detect_enable = ConfigCurrentDetectEnable::kEnabled; }

    /// @brief Disable current-detect mode
    void DisableCurrentDetect() { this->bitfield_.current_detect_enable = ConfigCurrentDetectEnable::kDisabled; }

    /// @brief Enable global-chop mode
    void SetGlobalChop(ConfigGlobalChopEnable enable) { this->bitfield_.global_chop_enable = enable; }

    /// @brief Set global chop delay between measurements
    void SetGlobalChopDelay(ConfigGlobalChopDelay delay) { this->bitfield_.global_chop_delay = delay; }

    /// @brief Current-detect channel selection (any or all channels)
    void SetCurrentDetectChannel(ConfigCurrentDetectChannelSelection channel)
    {
        this->bitfield_.current_detect_channel_selection = channel;
    }

    /// @brief Set number of current-detect exceeded thresholds to trigger a detection
    void SetCurrentDetectThresholdNum(ConfigCurrentDetectNumber num) { this->bitfield_.current_detect_number = num; }

    /// @brief Set current-detect measurement length (in conversion periods)
    void SetCurrentDetectThresholdLen(ConfigCurrentDetectLength len) { this->bitfield_.current_detect_length = len; }
};  // class RegisterConfig

}  // namespace fw
