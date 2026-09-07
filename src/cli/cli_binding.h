// cli_binding.h - CLI bindings
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// lib includes
#include "embedded_cli.h"

/// @brief CLI command to clear terminal
/// @param cli CLI handle
/// @param args Optional arguments (unused)
/// @param context Optional context (unused)
void onClearCLI(EmbeddedCli* cli, char* args, void* context);

/// @brief Load command bindings
void initCliBinding();
