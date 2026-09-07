// debug_registry.h - Weak declarations of per-module debug print functions.
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
//
// Each function is declared weak. If the corresponding debug source file is absent
// from the build (line commented out in cmake/cli_features.cmake), the linker resolves
// the symbol to null and DebugPrintPending() skips it -- the same mechanism used for
// CLI registrars in cli_registry.h.
//
// budget > 0: print up to budget lines; return the number of lines printed.
// budget == 0: do not print; return the number of lines currently pending.
//              Used by DebugPrintPending() to account for suppressed lines when the
//              per-cycle budget is exhausted before this module is reached.

#pragma once

// clang-format off
int DebugPrintAdc1(int budget)    __attribute__((weak));
int DebugPrintCan(int budget)     __attribute__((weak));
int DebugPrintI2c2(int budget)    __attribute__((weak));
// clang-format on
