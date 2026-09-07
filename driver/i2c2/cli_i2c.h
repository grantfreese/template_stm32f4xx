// cli_i2c.h - I2C bus diagnostic CLI command declarations
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "embedded_cli.h"
#include "hal/ii2c.h"

//! @brief Register a bus with the 'i2c' CLI command's scan/probe verbs.
//!
//! Call once per bus at init, before the CLI starts processing commands.
//! @param label Short human-readable bus name shown in the bus list. Must remain valid forever.
//! @param bus   The bus. Must remain valid forever.
void RegisterI2cCliBus(const char* label, fw::II2C& bus);

//! @brief Handler for the 'i2c' CLI command.
void OnI2c(EmbeddedCli* cli, char* args, void* context);

//! @brief Register the I2C CLI command with the CLI instance.
void RegisterI2cCliCommands(EmbeddedCli* cli);
