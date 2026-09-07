// cli_debug.h - Header for debug print CLI commands
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <cstdio>

#include "cli/debug_stage.h"
#include "embedded_cli.h"

//! @brief Handler for debug / d CLI command.
void onDebug(EmbeddedCli* cli, char* args, void* context);

//! @brief Register all debug CLI commands with the CLI instance.
void RegisterDebugCliCommands(EmbeddedCli* cli);

constexpr CliCommandBinding commandDebug = {
    .name = "debug", .help = "Debug print control", .tokenizeArgs = true, .context = NULL, .binding = onDebug};

constexpr CliCommandBinding commandDebugAlias = {
    .name = "d", .help = "Debug print control (alias)", .tokenizeArgs = true, .context = NULL, .binding = onDebug};
