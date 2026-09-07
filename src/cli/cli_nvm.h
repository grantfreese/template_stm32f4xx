// cli_nvm.h - Header for NVM CLI commands
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// lib includes
#include "embedded_cli.h"

// standard includes
#include <cstdio>

//! @brief Help for nvm command
void helpNvm();

/// @brief Handler for nvm command
void onNvm(EmbeddedCli* cli, char* args, void* context);

//! @brief Register all NVM CLI commands with the CLI instance.
void RegisterNvmCliCommands(EmbeddedCli* cli);

constexpr CliCommandBinding commandNvm = {
    .name = "nvm", .help = "Non-volatile memory", .tokenizeArgs = true, .context = NULL, .binding = onNvm};
