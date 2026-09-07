// debug_stubs.cpp - Weak no-op implementations of all DebugStageXxx() functions.
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
//
// Always compiled as part of the CLI infrastructure. When a debug module's source
// file is included in the build (via cmake/cli_features.cmake), its strong definition
// of the staging function takes precedence and the stub is discarded by the linker.
// When the module is absent, the stub silently discards the call -- callers never
// need a null guard.

#include <cstdint>

#include "cli/debug_stage.h"

// clang-format off
__attribute__((weak)) void DebugStageAdc1(uint32_t, uint16_t, float) {}
__attribute__((weak)) void DebugStageCanFrame(uint32_t, const uint8_t*, uint8_t, bool, bool) {}
__attribute__((weak)) void DebugStageI2c2Data(uint8_t, bool, const uint8_t*, uint8_t) {}
__attribute__((weak)) void DebugStageI2c2Error(uint8_t, const char*) {}
// clang-format on
