#pragma once

#include "ads131m0x_private.h"
#include "ads131m0x_register.h"

namespace fw
{
/// @brief Power mode selection
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-17
enum class ClockPowerMode
{
    kVeryLow = 0,
    kLow = 1,
    kHighResolution = 2
};

/// @brief Oversample ratio selection
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-17
enum class ClockOversampleRatio
{
    kOversample_128 = 0,
    kOversample_256 = 1,
    kOversample_512 = 2,
    kOversample_1024 = 3,
    kOversample_2048 = 4,
    kOversample_4096 = 5,
    kOversample_8192 = 6,
    kOversample_16256 = 7,
};

/// @brief External reference enable
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-17
enum class ClockExternalReferenceEnable
{
    kDisabled = 0,
    kEnabled = 1,
};

/// @brief Crystal oscillator disable
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-17
enum class ClockCrystalDisable
{
    kEnabled = 0,
    kDisabled = 1,
};

/// @brief ADC channel enable
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-17
enum class ClockChannelEnable
{
    kDisabled = 0,
    kEnabled = 1,
};

/// @brief CLOCK register bitfield definitions
/// Source: Texas Instruments ADS131M08 datasheet, SBAS950B Oct-2019, Table 8-17
// disable bitpacking so we can perform type punning on the struct
#pragma pack(push, 1)
typedef struct
{
    // magic number: ADS131M08 datasheet SBAS950B, p. 55, Table 8-17
    ClockPowerMode power_mode : 2;
    ClockOversampleRatio oversample_ratio : 3;
    int : 1;
    ClockExternalReferenceEnable external_reference_enable : 1;
    ClockCrystalDisable crystal_disable : 1;
    ClockChannelEnable channel_0_enable : 1;
    ClockChannelEnable channel_1_enable : 1;
    ClockChannelEnable channel_2_enable : 1;
    ClockChannelEnable channel_3_enable : 1;
    ClockChannelEnable channel_4_enable : 1;
    ClockChannelEnable channel_5_enable : 1;
    ClockChannelEnable channel_6_enable : 1;
    ClockChannelEnable channel_7_enable : 1;
} ClockBits;
#pragma pack(pop)

template <Ads131m0xDevice kDevice>
class RegisterClock : public Adsm131m0xRegister<ClockBits, RegAddr::kClock, AccessLevel::kReadWrite>
{
public:
    RegisterClock(IAds131m0xSpi& spi_writer, uint16_t register_check_mask)
        : Adsm131m0xRegister(spi_writer, register_check_mask)
    {
    }
    /// @brief Set oversample ratio for all channels
    void SetOSR(ClockOversampleRatio osr) { this->bitfield_.oversample_ratio = osr; }

    /// @brief Enable/disable external voltage reference
    void SetExternalReference(ClockExternalReferenceEnable enable)
    {
        this->bitfield_.external_reference_enable = enable;
    }

    /// @brief Enable/disable crystal oscillator (disable when using external clock)
    void SetCrystalDisable(ClockCrystalDisable enable) { this->bitfield_.crystal_disable = enable; }

    /// @brief Enable/disable adc channel
    void SetChannelEnable(AdcChannel channel, ClockChannelEnable enable)
    {
        // ADS131M04 has only 4 channels, so we need to check if channel is valid
        if (kDevice == Ads131m0xDevice::kADS131M04 && channel > AdcChannel::kChan3)
        {
            NotSupportedException("Channel not supported by ADS131M04");
        }
        else
        {
            switch (channel)
            {
                case AdcChannel::kChan0:
                    this->bitfield_.channel_0_enable = enable;
                    break;
                case AdcChannel::kChan1:
                    this->bitfield_.channel_1_enable = enable;
                    break;
                case AdcChannel::kChan2:
                    this->bitfield_.channel_2_enable = enable;
                    break;
                case AdcChannel::kChan3:
                    this->bitfield_.channel_3_enable = enable;
                    break;
                case AdcChannel::kChan4:
                    this->bitfield_.channel_4_enable = enable;
                    break;
                case AdcChannel::kChan5:
                    this->bitfield_.channel_5_enable = enable;
                    break;
                case AdcChannel::kChan6:
                    this->bitfield_.channel_6_enable = enable;
                    break;
                case AdcChannel::kChan7:
                    this->bitfield_.channel_7_enable = enable;
                    break;
                default:
                    NotSupportedException("Invalid ADC channel");
            }
        }
    }
};  // class RegisterClock

}  // namespace fw
