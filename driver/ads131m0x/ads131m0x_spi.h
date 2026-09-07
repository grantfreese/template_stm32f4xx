// ads131m0x_spi.h - SPI header for ADS131M0x ADC driver
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "ads131m0x_private.h"
#include "helpers.h"

// lib includes
#include "framework/error.h"
#include "hal/ispi.h"
#include "platform/io.h"

// os includes

// standard includes
#include <cstdint>
#include <cstring>  // memcpy

namespace fw
{
class IAds131m0xSpi
{
public:
    // write and verify register
    virtual bool Write(RegAddr reg_addr, uint16_t reg_value) = 0;

    // read register
    virtual uint16_t Read(RegAddr reg_addr) = 0;
};

template <Ads131m0xDevice kDevice>
class Ads131m0xSpi : public IAds131m0xSpi
{
public:
    Ads131m0xSpi(ISPI& spi, ISettableDigitalSignal& spi_cs) : spi_{&spi}, spi_cs_{&spi_cs} {}

    void Standby()
    {
        uint16_t command = static_cast<uint16_t>(Ads131m0xCommandWord::kStandby);
        uint16_t command_network = htons(command);  // swap to network endianness

        // memcpy for type punning
        ClearBuffer();
        memcpy(spi_buffer_short_frame_.data(), &command_network, sizeof(command_network));

        SendFrames();
    }

    /// @brief Send wake command to device
    void Wake()
    {
        uint16_t command = static_cast<uint16_t>(Ads131m0xCommandWord::kWakeup);
        uint16_t command_network = htons(command);  // swap to network endianness

        // memcpy for type punning
        ClearBuffer();
        memcpy(spi_buffer_short_frame_.data(), &command_network, sizeof(command_network));

        SendFrames();
    }

    /// @brief Send TX frame followed by RX frame
    /// @return Read/write response register value (host network order)
    uint16_t SendFrames()
    {
        uint16_t register_value_network{0};

        // clock out tx frame
        spi_cs_->SetState(IOState::kAsserted);
        spi_->Write(spi_buffer_short_frame_, kFrameSizeShort);
        spi_cs_->SetState(IOState::kDeasserted);

        // clock in rx frame (use rx_data_array_ as tx buffer to clock in data)
        ClearBuffer();
        spi_cs_->SetState(IOState::kAsserted);
        spi_->Transfer(spi_buffer_short_frame_, spi_buffer_short_frame_, kFrameSizeShort);
        spi_cs_->SetState(IOState::kDeasserted);

        // memcpy for type punning
        memcpy(&register_value_network, spi_buffer_short_frame_.data(), sizeof(register_value_network));
        return htons(register_value_network);
    }

    /// @brief Reads an external device register over SPI
    /// @param reg_addr Address of register
    /// @return Register value of type uint16_t in host byte order
    uint16_t Read(RegAddr reg_addr)
    {
        uint16_t command = static_cast<uint16_t>(Ads131m0xCommandWord::kRReg);
        command = command | (static_cast<uint16_t>(toUnder(reg_addr) << 7));
        uint16_t command_network = htons(command);  // swap to network endianness

        // memcpy for type punning
        ClearBuffer();
        memcpy(spi_buffer_short_frame_.data(), &command_network, sizeof(command_network));

        return SendFrames();
    }

    /// @brief Reads an external device register over SPI
    /// @param reg_addr Address of register
    /// @param reg_value Value to write to register
    bool Write(RegAddr reg_addr, uint16_t reg_value)
    {
        uint16_t command = static_cast<uint16_t>(Ads131m0xCommandWord::kWReg);

        // insert register address and register write count into command word
        command = command | (static_cast<uint16_t>(toUnder(reg_addr) << 7));

        // swap to network endianness
        uint16_t command_network = htons(command);
        uint16_t register_value_network = htons(reg_value);

        // memcpy for type punning
        ClearBuffer();
        memcpy(spi_buffer_short_frame_.data(), &command_network, sizeof(command_network));
        memcpy(spi_buffer_short_frame_.data() + sizeof(command_network) + 1,
               &register_value_network,
               sizeof(register_value_network));

        // verify response to WREG command (should be same as command packet but with bit 13 cleared)
        const uint16_t masked_command = command & GetUnderlyingType(Ads131m0xCommandWord::kWRegResponseMask);
        const uint16_t response = SendFrames();
        return (response == masked_command);
    }

    /// @brief Retrieves samples from external device over SPI
    bool GetSamples(std::span<int> adc_samples)
    {
        // (3 bytes per sample/register) * (8 samples + 1 status reg) + (2 crc)
        static constexpr size_t kNumFrameBytesSample{(3 * (GetUnderlyingType(kDevice) + 2))};
        constexpr uint16_t kCommand = static_cast<uint16_t>(Ads131m0xCommandWord::kNull);
        std::array<std::byte, kNumFrameBytesSample> spi_buffer{std::byte{0}};

        // swap to network endianness
        constexpr uint16_t kCommandNetwork = htons(kCommand);

        runtime_assert(adc_samples.size() == GetUnderlyingType(kDevice),
                       "Array passed into GetSamples() does not match ADC channel count");

        // memcpy for type punning
        ClearBuffer();
        memcpy(spi_buffer.data(), &kCommandNetwork, sizeof(kCommandNetwork));

        // clock in rx frame
        spi_cs_->SetState(IOState::kAsserted);
        spi_->Transfer(spi_buffer, spi_buffer, kNumFrameBytesSample);
        spi_cs_->SetState(IOState::kDeasserted);

        uint16_t crc_calculated = CalcCRC();
        uint16_t crc_rx = 0;

        // extract samples from SPI buffer
        auto itr_input = spi_buffer.begin() + 3;  // skip status response byte for now
        for (auto itr_output = adc_samples.begin(); itr_output < adc_samples.end(); ++itr_output)
        {
            int byte0 = static_cast<int>(itr_input[2]);
            int byte1 = static_cast<int>(itr_input[1]);
            int byte2 = static_cast<int>(itr_input[0]);
            int byte3;

            // handle 2's complement sign extension for negative values
            if ((byte2 & (1 << 7)) != 0)
            {
                byte3 = 0xFF;
            }
            else
            {
                byte3 = 0x00;
            }

            int32_t sample_network = byte0 << 24 | byte1 << 16 | byte2 << 8 | byte3 << 0;
            *itr_output = ntohl(sample_network);  // convert to host byte order
            itr_input = std::next(itr_input, 3);  // manually increment by three bytes to next 24-bit sample
        }

        crc_rx = static_cast<uint16_t>(static_cast<uint32_t>(*(itr_input + 0)) << 8
                                       | static_cast<uint32_t>(*(itr_input + 1)));
        if (crc_rx == crc_calculated)
        {
            ++count_crc_valid_;
            return true;
        }
        else
        {
            ++count_crc_invalid_;
            return false;
        }
    }

private:
    // constants
    static constexpr size_t kFrameSizeShort{6};  // <3 command><3 data>

    ISPI* spi_;
    ISettableDigitalSignal* spi_cs_;
    std::array<std::byte, kFrameSizeShort> spi_buffer_short_frame_{std::byte{0}};
    int count_crc_valid_{0};
    int count_crc_invalid_{0};

    /// @brief Calculate CRC-16/AUG-CCITT of SPI buffer
    // Automatically generated CRC function
    // polynomial: 0x11021
    uint16_t CalcCRC()
    {
        // CRC-16/CCITT-FALSE
        static constexpr uint16_t kTable[256] = {
            0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50A5U, 0x60C6U, 0x70E7U, 0x8108U, 0x9129U, 0xA14AU, 0xB16BU,
            0xC18CU, 0xD1ADU, 0xE1CEU, 0xF1EFU, 0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52B5U, 0x4294U, 0x72F7U, 0x62D6U,
            0x9339U, 0x8318U, 0xB37BU, 0xA35AU, 0xD3BDU, 0xC39CU, 0xF3FFU, 0xE3DEU, 0x2462U, 0x3443U, 0x0420U, 0x1401U,
            0x64E6U, 0x74C7U, 0x44A4U, 0x5485U, 0xA56AU, 0xB54BU, 0x8528U, 0x9509U, 0xE5EEU, 0xF5CFU, 0xC5ACU, 0xD58DU,
            0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76D7U, 0x66F6U, 0x5695U, 0x46B4U, 0xB75BU, 0xA77AU, 0x9719U, 0x8738U,
            0xF7DFU, 0xE7FEU, 0xD79DU, 0xC7BCU, 0x48C4U, 0x58E5U, 0x6886U, 0x78A7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
            0xC9CCU, 0xD9EDU, 0xE98EU, 0xF9AFU, 0x8948U, 0x9969U, 0xA90AU, 0xB92BU, 0x5AF5U, 0x4AD4U, 0x7AB7U, 0x6A96U,
            0x1A71U, 0x0A50U, 0x3A33U, 0x2A12U, 0xDBFDU, 0xCBDCU, 0xFBBFU, 0xEB9EU, 0x9B79U, 0x8B58U, 0xBB3BU, 0xAB1AU,
            0x6CA6U, 0x7C87U, 0x4CE4U, 0x5CC5U, 0x2C22U, 0x3C03U, 0x0C60U, 0x1C41U, 0xEDAEU, 0xFD8FU, 0xCDECU, 0xDDCDU,
            0xAD2AU, 0xBD0BU, 0x8D68U, 0x9D49U, 0x7E97U, 0x6EB6U, 0x5ED5U, 0x4EF4U, 0x3E13U, 0x2E32U, 0x1E51U, 0x0E70U,
            0xFF9FU, 0xEFBEU, 0xDFDDU, 0xCFFCU, 0xBF1BU, 0xAF3AU, 0x9F59U, 0x8F78U, 0x9188U, 0x81A9U, 0xB1CAU, 0xA1EBU,
            0xD10CU, 0xC12DU, 0xF14EU, 0xE16FU, 0x1080U, 0x00A1U, 0x30C2U, 0x20E3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
            0x83B9U, 0x9398U, 0xA3FBU, 0xB3DAU, 0xC33DU, 0xD31CU, 0xE37FU, 0xF35EU, 0x02B1U, 0x1290U, 0x22F3U, 0x32D2U,
            0x4235U, 0x5214U, 0x6277U, 0x7256U, 0xB5EAU, 0xA5CBU, 0x95A8U, 0x8589U, 0xF56EU, 0xE54FU, 0xD52CU, 0xC50DU,
            0x34E2U, 0x24C3U, 0x14A0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U, 0xA7DBU, 0xB7FAU, 0x8799U, 0x97B8U,
            0xE75FU, 0xF77EU, 0xC71DU, 0xD73CU, 0x26D3U, 0x36F2U, 0x0691U, 0x16B0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
            0xD94CU, 0xC96DU, 0xF90EU, 0xE92FU, 0x99C8U, 0x89E9U, 0xB98AU, 0xA9ABU, 0x5844U, 0x4865U, 0x7806U, 0x6827U,
            0x18C0U, 0x08E1U, 0x3882U, 0x28A3U, 0xCB7DU, 0xDB5CU, 0xEB3FU, 0xFB1EU, 0x8BF9U, 0x9BD8U, 0xABBBU, 0xBB9AU,
            0x4A75U, 0x5A54U, 0x6A37U, 0x7A16U, 0x0AF1U, 0x1AD0U, 0x2AB3U, 0x3A92U, 0xFD2EU, 0xED0FU, 0xDD6CU, 0xCD4DU,
            0xBDAAU, 0xAD8BU, 0x9DE8U, 0x8DC9U, 0x7C26U, 0x6C07U, 0x5C64U, 0x4C45U, 0x3CA2U, 0x2C83U, 0x1CE0U, 0x0CC1U,
            0xEF1FU, 0xFF3EU, 0xCF5DU, 0xDF7CU, 0xAF9BU, 0xBFBAU, 0x8FD9U, 0x9FF8U, 0x6E17U, 0x7E36U, 0x4E55U, 0x5E74U,
            0x2E93U, 0x3EB2U, 0x0ED1U, 0x1EF0U,
        };

        int len = kFrameSizeShort - 3;
        uint8_t* data = reinterpret_cast<uint8_t*>(spi_buffer_short_frame_.data());
        uint16_t crc = 0xFFFF;  // magic number: use 0xFFFF as initial CRC

        while (len > 0)
        {
            crc = kTable[*data ^ (uint8_t)(crc >> 8)] ^ static_cast<uint16_t>(crc << 8);
            ++data;
            --len;
        }

        return crc;
    }

    /// @brief Zero out SPI buffer
    void ClearBuffer()
    {
        for (auto& byte : spi_buffer_short_frame_)
        {
            byte = std::byte{0};
        }
    }
};

}  // namespace fw
