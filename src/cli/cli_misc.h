// cli_misc.h - Header for misc CLI commands
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes

// lib includes
#include "embedded_cli.h"

// os includes

// standard includes
#include <cstdio>
#include <string>

//! @brief Store the firmware name and version strings for use by onClearCLI.
//!
//! Must be called once before the CLI is ready to process commands.
//! @param name Pointer to the firmware name string. Must remain valid for the lifetime of the CLI.
//! @param version Pointer to the firmware version string. Must remain valid for the lifetime of the CLI.
void SetCliFirmwareStrings(const std::string* name, const std::string* version);

void onClearCLI(EmbeddedCli* cli, char* args, void* context);
void onReset(EmbeddedCli* cli, char* args, void* context);
void onSave(EmbeddedCli* cli, char* args, void* context);

constexpr CliCommandBinding commandClear = {
    .name = "clear", .help = "Clears the console", .tokenizeArgs = true, .context = NULL, .binding = onClearCLI};

constexpr CliCommandBinding commandReset = {
    .name = "reset", .help = "Reset the board", .tokenizeArgs = true, .context = NULL, .binding = onReset};

constexpr CliCommandBinding commandSave = {
    .name = "save", .help = "Save settings to flash", .tokenizeArgs = true, .context = NULL, .binding = onSave};
