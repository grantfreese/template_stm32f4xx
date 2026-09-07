// cli_misc.cpp - Implementation of misc CLI commands
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "cli_misc.h"

#include "cli_binding.h"
#include "cli_setup.h"
#include "nvm/nvm.h"

// lib includes
#include "embedded_cli.h"
#include "stm32f3xx_hal.h"

// os includes

// standard includes
#include <cstring>
#include <string>

static const std::string* cli_name_string{nullptr};
static const std::string* cli_version_string{nullptr};

void SetCliFirmwareStrings(const std::string* name, const std::string* version)
{
    cli_name_string = name;
    cli_version_string = version;
}

void onClearCLI(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)args;
    (void)context;
    cli_printf("\33[2J");
    if (cli_name_string != nullptr)
    {
        cli_printf("%s", cli_name_string->c_str());
    }
    const char* version_str = "unknown";
    if (cli_version_string != nullptr)
    {
        version_str = cli_version_string->c_str();
    }
    cli_printf("version: %s", version_str);
}

void onReset(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)args;
    (void)context;

    cli_printf("Resetting board...");
    HAL_NVIC_SystemReset();
}

void onSave(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)context;

    if (embeddedCliGetToken(args, 1) != NULL)
    {
        cli_printf("Error: Save command does not take any arguments");
        return;
    }
    if (nvm_h->Write())
    {
        cli_printf("Saved parameters to flash");
    }
    else
    {
        cli_printf("Error: NVM write failed");
    }
}
