// can_broadcast.cpp - Periodic CAN broadcast frame builders
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "can/can_broadcast.h"

#include "can/can_tx_queue.h"
#include "cli/debug_stage.h"
#include "main.h"

// standard includes
#include <cstdint>
#include <string>

using namespace fw;

static bool can_broadcast_enabled{true};

bool IsCanBroadcastEnabled() { return can_broadcast_enabled; }

void SetCanBroadcastEnabled(bool enabled) { can_broadcast_enabled = enabled; }

// Encoding helpers — write big-endian multi-byte values into a uint8_t buffer at a given byte offset.
static void EncodeU16Be(uint8_t* buf, int offset, uint16_t value)
{
    buf[offset + 0] = static_cast<uint8_t>(value >> 8);
    buf[offset + 1] = static_cast<uint8_t>(value >> 0);
}

static void EncodeI16Be(uint8_t* buf, int offset, int16_t value)
{
    EncodeU16Be(buf, offset, static_cast<uint16_t>(value));
}

// scaling factors for CAN messages
constexpr float kScaleFactorTemperature{100.0f};

void CanBroadcast::Broadcast(CAN_HandleTypeDef* hcan, float die_temperature_c, uint16_t die_temperature_raw)
{
    if (!IsCanBroadcastEnabled())
    {
        return;
    }

    buffer_index_ = 0;

    BuildVersionFrame(kFirmwareVersionString);
    BuildTemperatureFrame(die_temperature_c, die_temperature_raw);

    for (int index = 0; index < buffer_index_; index++)
    {
        const CanBroadcastFrame& frame = frame_buffer_[index];
        can_tx_queue.Enqueue(frame.type, static_cast<uint8_t>(frame.length), frame.data);
        DebugStageCanFrame(can_tx_queue.GetExtId(frame.type),
                           frame.data,
                           static_cast<uint8_t>(frame.length),
                           /*is_tx=*/true,
                           /*is_extended=*/true);
    }
    can_tx_queue.DrainToMailboxes(hcan);
}

void CanBroadcast::BuildVersionFrame(const std::string& firmware_version_string)
{
    // CAN DBC:
    // BO_ 2678063105 FW_VERSION: 5 TEMPLATE
    //  SG_ FW_VER_MAJOR   :  7|8@0+ (1,0) [0|255] "" Vector__XXX
    //  SG_ FW_VER_MINOR   : 15|8@0+ (1,0) [0|255] "" Vector__XXX
    //  SG_ FW_VER_PATCH   : 23|8@0+ (1,0) [0|255] "" Vector__XXX
    //  SG_ FW_VER_TAGDIST : 31|8@0+ (1,0) [0|255] "" Vector__XXX
    //  SG_ FW_VER_LOCAL   : 32|1@0+ (1,0) [0|1]   "" Vector__XXX
    //  SG_ FW_VER_DIRTY   : 33|1@0+ (1,0) [0|1]   "" Vector__XXX

    // firmware_version_string format: "X.Y.Z[+n][-DL]" (n means commit count since tag, -D means dirty, -L means local
    // build)

    CanBroadcastFrame frame{.type = CanFrameType::kFirmwareVersion, .length = 5, .data = {0}};

    uint8_t major{0};
    uint8_t minor{0};
    uint8_t patch{0};
    uint8_t tagdist{0};
    uint8_t local{0};
    uint8_t dirty{0};

    size_t pos1{firmware_version_string.find('.')};
    size_t pos2{firmware_version_string.find('.', pos1 + 1)};
    size_t pos_plus{firmware_version_string.find('+')};
    size_t pos_dash{firmware_version_string.find('-')};

    if (pos1 != std::string::npos && pos2 != std::string::npos)
    {
        major = static_cast<uint8_t>(std::stoi(firmware_version_string.substr(0, pos1)));
        minor = static_cast<uint8_t>(std::stoi(firmware_version_string.substr(pos1 + 1, pos2 - pos1 - 1)));

        if (pos_plus != std::string::npos)
        {
            // Extract patch version up to the '+'
            patch = static_cast<uint8_t>(std::stoi(firmware_version_string.substr(pos2 + 1, pos_plus - pos2 - 1)));

            // Extract tagdist (commits since tag) after the '+'
            if (pos_dash != std::string::npos)
            {
                // Format: X.Y.Z+TAGDIST-DL
                tagdist = static_cast<uint8_t>(
                    std::stoi(firmware_version_string.substr(pos_plus + 1, pos_dash - pos_plus - 1)));

                std::string suffix{firmware_version_string.substr(pos_dash)};
                if (suffix.find('D') != std::string::npos)
                {
                    dirty = 1;
                }
                if (suffix.find('L') != std::string::npos)
                {
                    local = 1;  // local means non-CI build
                }
            }
            else
            {
                // Format: X.Y.Z+TAGDIST
                tagdist = static_cast<uint8_t>(std::stoi(firmware_version_string.substr(pos_plus + 1)));
            }
        }
        else if (pos_dash != std::string::npos)
        {
            // Format: X.Y.Z-DL (no tagdist)
            patch = static_cast<uint8_t>(std::stoi(firmware_version_string.substr(pos2 + 1, pos_dash - pos2 - 1)));

            std::string suffix{firmware_version_string.substr(pos_dash)};
            if (suffix.find('D') != std::string::npos)
            {
                dirty = 1;
            }
            if (suffix.find('L') != std::string::npos)
            {
                local = 1;  // local means non-CI build
            }
        }
        else
        {
            // Format: X.Y.Z (clean release)
            patch = static_cast<uint8_t>(std::stoi(firmware_version_string.substr(pos2 + 1)));
        }
    }

    // Pack version fields according to the DBC layout
    frame.data[0] = major;
    frame.data[1] = minor;
    frame.data[2] = patch;
    frame.data[3] = tagdist;
    frame.data[4] = static_cast<uint8_t>((local << 0) | (dirty << 1));

    frame_buffer_[buffer_index_++] = frame;
}

void CanBroadcast::BuildTemperatureFrame(float die_temperature_c, uint16_t die_temperature_raw)
{
    // CAN DBC:
    // BO_ 2678063106 MCU_TEMPERATURE: 4 TEMPLATE
    //  SG_ MCU_DIE_TEMP     :  7|16@0- (0.01,0) [-327.68|327.67] "degC" Vector__XXX
    //  SG_ MCU_DIE_TEMP_RAW : 23|16@0+ (1,0)    [0|4095]         ""     Vector__XXX

    CanBroadcastFrame frame{.type = CanFrameType::kMcuTemperature, .length = 4, .data = {0}};

    EncodeI16Be(frame.data, 0, static_cast<int16_t>(die_temperature_c * kScaleFactorTemperature));
    EncodeU16Be(frame.data, 2, die_temperature_raw);

    frame_buffer_[buffer_index_++] = frame;
}
