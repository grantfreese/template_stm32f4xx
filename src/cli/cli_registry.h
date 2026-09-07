// cli_registry.h - Weak declarations for optional CLI module registrars
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
//
// Each declaration here corresponds to one optional CLI source file listed in
// cmake/cli_features.cmake. cli_binding.cpp iterates all registrars and calls
// the ones that are not null.
//
// How this works without #ifdef:
//   __attribute__((weak)) tells the linker that this symbol is optional. If the
//   source file that provides the strong definition is compiled in (i.e., its line
//   in cli_features.cmake is active), the real function is called. If the source
//   file is absent, the linker sets the symbol's address to null, and cli_binding.cpp
//   skips it. No preprocessor conditionals are needed anywhere.
//
// To add a new CLI module:
//   1. Add a weak declaration here (one line).
//   2. Add a target_sources() line in cmake/cli_features.cmake.
//   3. Implement RegisterXxxCliCommands() in the new source file.

#pragma once

#include "embedded_cli.h"

// clang-format off

// Optional CLI module registrars -- present when the corresponding source file
// is compiled in via cmake/cli_features.cmake, null otherwise.
void RegisterCanCliCommands(EmbeddedCli* cli)    __attribute__((weak));
void RegisterDebugCliCommands(EmbeddedCli* cli)  __attribute__((weak));
void RegisterI2cCliCommands(EmbeddedCli* cli)    __attribute__((weak));
void RegisterNvmCliCommands(EmbeddedCli* cli)    __attribute__((weak));
void RegisterSimpleCliCommands(EmbeddedCli* cli) __attribute__((weak));
void RegisterSpiCliCommands(EmbeddedCli* cli)    __attribute__((weak));
void RegisterStatusCliCommands(EmbeddedCli* cli) __attribute__((weak));
void RegisterUartCliCommands(EmbeddedCli* cli)   __attribute__((weak));

// clang-format on
