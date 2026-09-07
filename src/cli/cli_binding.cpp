// cli_binding.cpp - CLI command registration
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "cli_binding.h"

#include "cli_misc.h"
#include "cli_registry.h"
#include "cli_setup.h"
#include "embedded_cli.h"

void initCliBinding()
{
    EmbeddedCli* cli = getCliPointer();

    // Infrastructure commands are always registered -- they cannot be disabled.
    embeddedCliAddBinding(cli, &commandClear);
    embeddedCliAddBinding(cli, &commandReset);
    embeddedCliAddBinding(cli, &commandSave);

    // Optional CLI modules -- each registrar is non-null only when its source
    // file is compiled in via cmake/cli_features.cmake. See cli_registry.h for
    // an explanation of how the weak-symbol mechanism works.
    using RegFn = void (*)(EmbeddedCli*);
    static const RegFn kOptionalRegistrars[] = {
        RegisterCanCliCommands,
        RegisterDebugCliCommands,
        RegisterI2cCliCommands,
        RegisterNvmCliCommands,
        RegisterSimpleCliCommands,
        RegisterSpiCliCommands,
        RegisterStatusCliCommands,
        RegisterUartCliCommands,
    };

    for (auto fn : kOptionalRegistrars)
    {
        if (fn != nullptr)
        {
            fn(cli);
        }
    }
}
