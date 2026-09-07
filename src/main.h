// main.h - Application-wide globals and the NVM storage record
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// standard includes
#include <cstdint>
#include <string>

extern std::string kFirmwareNameString;
extern std::string kFirmwareVersionString;

//! Magic cookie written to the last field of NonvolatileStorage_t. If the stored value does not
//! match this constant (e.g. on first boot or after a chip erase), the entire struct is treated
//! as uninitialized and reset to safe defaults before any fields are consumed.
//!
//! Bump this whenever the struct layout changes so a single byte added inside an existing
//! padding gap can't reuse a stale zeroed pad as its initial value (which silently reads as
//! "disabled" for any uint8_t-encoded enable flag).
constexpr uint32_t kNvmMagic{0x54454D50};

//! First-boot default for the placeholder test value.
constexpr float kTestValueDefault{1.234f};

//! Persistent settings record. Projects built from this template replace test_value with their
//! real fields (and bump kNvmMagic on every layout change). nvm_sequence and magic are NVM
//! infrastructure and must remain the last two fields.
typedef struct
{
    // Placeholder proving the persistence path; modifiable via `nvm set test <value>` + `save`.
    float test_value;
    // Monotonic write counter. Bumped by the Nvm class on every successful Write() so the
    // boot-time slot-select logic can distinguish "this slot was written more recently" from
    // "this slot is the older copy".
    uint32_t nvm_sequence;
    // Must remain last. Set to kNvmMagic on every valid write. A mismatch on boot in BOTH slots
    // triggers a full reinit to defaults so no field is ever consumed with an erased-flash or
    // corrupted value.
    uint32_t magic;
} NonvolatileStorage_t;
