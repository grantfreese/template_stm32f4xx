// ads131m0x.h - ADS131M0x ADC driver
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "ads131m0x_private.h"
#include "ads131m0x_reg_cfg.h"
#include "ads131m0x_reg_clock.h"
#include "ads131m0x_reg_gain.h"
#include "ads131m0x_reg_id.h"
#include "ads131m0x_reg_mode.h"
#include "ads131m0x_reg_status.h"
#include "ads131m0x_spi.h"

// lib includes
#include "cmsis_os.h"
#include "framework/error.h"
#include "hal/ispi.h"
#include "hal/signals.h"
#include "platform/io.h"
#include "platform/stm32f3xx_delay.h"

// os includes

// standard includes
#include <cstdint>
#include <cstring>  // memset

namespace fw
{
constexpr uint32_t kDataReadyTimeoutMsec{10};

/// @brief Representation of single ADC channel measurement
class Ads131m0xChannel : public IADCChannel
{
public:
    /// @brief Get the raw ADC count
    /// @return Int containing raw ADC count
    int GetRawCount() const override { return rawCount_; }

    /// @brief  Get the voltage at the pin of the ADC input
    /// @return Float voltage of pin
    float GetVoltage() const override
    {
        static constexpr float kAdcVref{1.2f * 2};  // vref=1.2v, 2x multiplier is since ADC has -1.2 - +1.2v range
        static constexpr int kAdcBits{24};
        static constexpr float kAdcResolution = kAdcVref / (1 << kAdcBits);
        return (gainReciprocal_ * static_cast<float>(rawCount_) * kAdcResolution);
    }

    /// @brief Set gain on ADC channel
    /// @param gain Gain to set
    void SetGain(float gain) { this->gainReciprocal_ = gain; }

    /// @brief Set raw ADC count
    /// @param raw_count Raw ADC count
    void SetRawCount(int raw_count) { this->rawCount_ = raw_count; }

private:
    int rawCount_{0};
    float gainReciprocal_;
};

/// @brief  Generic driver for ADS131M0x-family of ADCs
/// @tparam kNumChannels Number of channels on the ADC
template <Ads131m0xDevice kDevice>
class Ads131m0x
{
public:
    /// @brief Constructor
    Ads131m0x(ISPI& spi,
              ISettableDigitalSignal& spi_cs,
              ISettableDigitalSignal& pin_sync_reset,
              IDigitalSignal& pin_data_ready)
        : spi_writer_{spi, spi_cs}, pin_sync_reset_{&pin_sync_reset}, pin_data_ready_{&pin_data_ready}
    {
    }

    /// @brief Initalizes ADC
    /// @return True on success, false on failure
    bool Initialize(bool do_reset = true)
    {
        bool ret{true};
        if (do_reset)
        {
            Reset();
        }

        spi_writer_.Standby();

        // manually set all regmode fields since we will write it to ADC before reading it
        register_mode_.DisableRegisterCrc();
        register_mode_.DisableRxCrc();
        register_mode_.SetCrcType(ModeCrcType::kCcitt16Bit);
        register_mode_.ClearResetFlag();
        register_mode_.SetWordLength(ModeWordLength::kSize24Bits);
        register_mode_.SetTimeout(ModeTimeoutEnable::kDisabled);
        register_mode_.SetDataReadySourceSelect(ModeDataReadySourceSelect::kMostLaggingEnabledChannel);
        register_mode_.SetDataReadyHighZ(ModeDataReadyHighZ::kLogicHigh);
        register_mode_.SetDataReadyFormat(ModeDataReadyFormat::kLogicLow);
        register_mode_.Write();

        ReadRegisters();

        // verify that we're talking to the correct device
        ret &= register_id_.VerifyID(GetUnderlyingType(kDevice));
        if (!ret)
        {
            std::runtime_error("ADC ID does not match driver");
        }

        return ret;
    }

    /// @brief Sends reset pulse to ADCs
    void Reset()
    {
        pin_sync_reset_->SetState(IOState::kAsserted);
        DelayUs(kResetHoldDuration);
        pin_sync_reset_->SetState(IOState::kDeasserted);
        DelayUs(kPostResetDelayUs);
    }

    /// @brief Sends sync pulse to ADCs
    void Sync()
    {
        pin_sync_reset_->SetState(IOState::kAsserted);
        pin_sync_reset_->SetState(IOState::kDeasserted);
    }

    /// @brief Put ADC into standby mode
    void Standby() { spi_writer_.Standby(); }

    /// @brief Wake ADC from standby mode
    void Wake() { spi_writer_.Wake(); }

    /// @brief Retrieves samples from ADC
    /// @return Span of Ads131m0xChannel objects
    std::span<Ads131m0xChannel> GetSamples() { return values_; }

    /// @brief Read ADC status register
    uint16_t ReadStatusRegister()
    {
        register_status_.Read();
        return register_status_.RegValue();
    }

    /// @brief Reads ADC registers
    void ReadRegisters()
    {
        register_id_.Read();
        register_status_.Read();
        register_mode_.Read();
        register_clock_.Read();
        register_gain_1_.Read();
        if (kDevice == Ads131m0xDevice::kADS131M08)
        {
            register_gain_2_.Read();
        }
        register_config_.Read();
    }

    /// @brief Test ADC communication
    bool TestCommunication()
    {
        bool commTestSuccess{true};
        commTestSuccess &= register_id_.CheckRegisterMatch();
        commTestSuccess &= register_mode_.CheckRegisterMatch();
        commTestSuccess &= register_clock_.CheckRegisterMatch();
        commTestSuccess &= register_gain_1_.CheckRegisterMatch();
        if (kDevice == Ads131m0xDevice::kADS131M08)
        {
            commTestSuccess &= register_gain_2_.CheckRegisterMatch();
        }
        commTestSuccess &= register_config_.CheckRegisterMatch();
        return commTestSuccess;
    }

    /// @brief Dump local and remote ADC registers
    void DumpRegisters()
    {
        uint16_t regOld;

        regOld = register_id_.RegValue();
        register_id_.Read();
        printf("reg    | local | remote\r\n");
        printf("-------+-------+-------\r\n");
        printf("id     | %04x  | %04x\r\n", regOld, register_id_.RegValue());

        regOld = register_status_.RegValue();
        register_status_.Read();
        printf("status | %04x  | %04x\r\n", regOld, register_status_.RegValue());

        regOld = register_mode_.RegValue();
        register_mode_.Read();
        printf("mode   | %04x  | %04x\r\n", regOld, register_mode_.RegValue());

        regOld = register_clock_.RegValue();
        register_clock_.Read();
        printf("clock  | %04x  | %04x\r\n", regOld, register_clock_.RegValue());

        regOld = register_gain_1_.RegValue();
        register_gain_1_.Read();
        printf("gain1  | %04x  | %04x\r\n", regOld, register_gain_1_.RegValue());

        if (kDevice == Ads131m0xDevice::kADS131M08)
        {
            regOld = register_gain_2_.RegValue();
            register_gain_2_.Read();
            printf("gain2  | %04x  | %04x\r\n", regOld, register_gain_2_.RegValue());
        }

        regOld = register_config_.RegValue();
        register_config_.Read();
        printf("config | %04x  | %04x\r\n\n", regOld, register_config_.RegValue());
    }

    /// @brief  Retrieves raw samples from external ADC over SPI
    void PollSamples(bool do_sync = false)
    {
        std::array<int, GetUnderlyingType(kDevice)> raw_samples = {};
        if (do_sync)
        {
            Sync();
        }

        uint32_t tickDeadline{osKernelGetTickCount() + kDataReadyTimeoutMsec};
        while (pin_data_ready_->GetState() != IOState::kAsserted && (osKernelGetTickCount() < tickDeadline))
        {
            // busy wait
        }

        // if no timeout
        if (osKernelGetTickCount() < tickDeadline)
        {
            spi_writer_.GetSamples(raw_samples);

            auto itr_value{values_.begin()};
            for (auto sample : raw_samples)
            {
                itr_value->SetRawCount(sample);
                ++itr_value;
            }
        }
    }

    bool SetChannelEnable(AdcChannel channel, ClockChannelEnable enable, bool updateRegisters = true)
    {
        register_clock_.SetChannelEnable(channel, enable);
        bool writeSuccess{true};
        if (updateRegisters)
        {
            writeSuccess = register_clock_.Write();
        }
        else
        {
            register_clock_.SetDirty();
        }
        return writeSuccess;
    }

    /// @brief Enables/Disables global chop mode
    /// @param enable Enables global chop mode when true, disables when false
    /// @param updateRegisters If true, writes the register to the ADC, otherwise just sets the dirty bit
    /// @return True on success, false on failure
    bool SetGlobalChop(ConfigGlobalChopEnable enable, bool updateRegisters = true)
    {
        register_config_.SetGlobalChop(enable);
        bool writeSuccess{true};
        if (updateRegisters)
        {
            writeSuccess = register_config_.Write();
        }
        else
        {
            register_config_.SetDirty();
        }
        return writeSuccess;
    }

    /// @brief Sets oversample rate on ADC
    /// @param osr Oversample rate to set (all channels)
    /// @param updateRegisters If true, writes the register to the ADC, otherwise just sets the dirty bit
    /// @return True on success, false on failure
    bool SetOSR(ClockOversampleRatio osr, bool updateRegisters = true)
    {
        register_clock_.SetOSR(osr);

        bool writeSuccess{true};
        if (updateRegisters)
        {
            writeSuccess = register_clock_.Write();
        }
        else
        {
            register_clock_.SetDirty();
        }
        return writeSuccess;
    }

    /// @brief Sets gain on an ADC channel
    /// @param channel Channel of ADC to set gain on
    /// @param gain Gain value to set
    /// @return True on success, false on failure
    bool SetGain(AdcChannel channel, AdcGain gain, bool updateRegisters = true)
    {
        // runtime_assert(((channel > AdcChannel::kChan3) && (kDevice == Ads131m0xDevice::kADS131M04)),
        //                "Invalid channel for this device");

        bool writeSuccess{true};
        // set gain in ADC register
        switch (channel)
        {
            case AdcChannel::kChan0:
            case AdcChannel::kChan1:
            case AdcChannel::kChan2:
            case AdcChannel::kChan3:
                register_gain_1_.SetGain(channel, gain);
                if (updateRegisters)
                {
                    writeSuccess &= register_gain_1_.Write();
                }
                else
                {
                    register_gain_1_.SetDirty();
                }
                break;

            case AdcChannel::kChan4:
            case AdcChannel::kChan5:
            case AdcChannel::kChan6:
            case AdcChannel::kChan7:
                register_gain_2_.SetGain(channel, gain);
                if (updateRegisters)
                {
                    writeSuccess &= register_gain_2_.Write();
                }
                else
                {
                    register_gain_2_.SetDirty();
                }
                break;

            default:
                NotSupportedException("Invalid ADC channel");
                break;
        }

        values_[static_cast<int>(channel)].SetGain(AdcGainToGainReciprocal(gain));
        return writeSuccess;
    }

protected:
    // register classes
    // magic numbers are bitmasks to avoid comparing read-only bits from ADC registers
    RegisterId register_id_{spi_writer_, 0xFF00};
    RegisterStatus register_status_{spi_writer_, 0x0000};
    RegisterMode register_mode_{spi_writer_, 0x3F1F};
    RegisterClock<kDevice> register_clock_{spi_writer_, 0xFFDF};
    RegisterGain1 register_gain_1_{spi_writer_, 0x7777};
    RegisterGain2 register_gain_2_{spi_writer_, 0x7777};
    RegisterConfig register_config_{spi_writer_, 0x1FFF};

private:
    // TODO: parameterize this delay based on the ADC clock frequency (not SPI clock frequency)
    // minimum_value: 2048 clock cycles @ 8.192 MHz (250 us)
    // max_value: 2 * minimum_value
    // Note: when max_value is exceeded, we get continual pulses on DRDY pin which datasheet does not mention
    uint32_t kResetHoldDuration{300};

    // TODO: parameterize this delay based on the ADC clock frequency (not SPI clock frequency)
    uint32_t kPostResetDelayUs{6};

    Ads131m0xSpi<kDevice> spi_writer_;
    std::array<Ads131m0xChannel, GetUnderlyingType(kDevice)> values_ = {};

    ISettableDigitalSignal* pin_sync_reset_;  // reset/sync pin
    IDigitalSignal* pin_data_ready_;          // data ready pin
};

/// @brief Driver for 8-channel ADS131M08 ADC
class Ads131m08 : public Ads131m0x<Ads131m0xDevice::kADS131M08>
{
public:
    Ads131m08(ISPI& spi,
              ISettableDigitalSignal& spi_cs,
              ISettableDigitalSignal& pin_sync_reset,
              IDigitalSignal& pin_data_ready)
        : Ads131m0x<Ads131m0xDevice::kADS131M08>(spi, spi_cs, pin_sync_reset, pin_data_ready)
    {
    }

    bool SetExternalReference(ClockExternalReferenceEnable enable, bool updateRegisters)
    {
        register_clock_.SetExternalReference(enable);

        bool writeSuccess{true};
        if (updateRegisters)
        {
            writeSuccess = register_clock_.Write();
        }
        else
        {
            register_clock_.SetDirty();
        }
        return writeSuccess;
    }

    bool SetCrystalDisable(ClockCrystalDisable disable, bool updateRegisters)
    {
        register_clock_.SetCrystalDisable(disable);

        bool writeSuccess{true};
        if (updateRegisters)
        {
            writeSuccess = register_clock_.Write();
        }
        else
        {
            register_clock_.SetDirty();
        }
        return writeSuccess;
    }
};

/// @brief Driver for 4-channel ADS131M04 ADC
class Ads131m04 : public Ads131m0x<Ads131m0xDevice::kADS131M04>
{
public:
    Ads131m04(ISPI& spi,
              ISettableDigitalSignal& spi_cs,
              ISettableDigitalSignal& pin_sync_reset,
              IDigitalSignal& pin_data_ready)
        : Ads131m0x<Ads131m0xDevice::kADS131M04>(spi, spi_cs, pin_sync_reset, pin_data_ready)
    {
    }
};

}  // namespace fw
