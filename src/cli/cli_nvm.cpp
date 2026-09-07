// cli_nvm.cpp - Implementation of NVM CLI commands
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "cli_nvm.h"

#include "cli_binding.h"
#include "cli_setup.h"
#include "main.h"
#include "nvm/nvm.h"

// lib includes
#include "embedded_cli.h"

// standard includes
#include <array>
#include <cstdlib>
#include <cstring>

namespace
{
// One row per settable key. `show`, `get`, and `set` share the SAME key, so a
// name read off `nvm show` can be fed straight back to `nvm set`. get() reads
// the persisted record; set() updates the in-RAM record -- `save` then writes
// it to flash.
struct NvmParam
{
    const char* key;
    const char* unit;
    float (*get)();
    void (*set)(float value);
};

constexpr std::array<NvmParam, 1> kNvmParams{{
    {"test",
     "",
     [] { return nvm_h->Get().test_value; },
     [](float value) { nvm_h->Mutate().test_value = value; }},
}};

const NvmParam* FindNvmParam(const char* key)
{
    for (const auto& param : kNvmParams)
    {
        if (std::strcmp(param.key, key) == 0)
        {
            return &param;
        }
    }
    return nullptr;
}

void PrintNvmValue(const char* key, float value, const char* unit)
{
    cli_printf("%-16s%f  %s", key, static_cast<double>(value), unit);
}

void ShowNvm()
{
    for (const auto& param : kNvmParams)
    {
        PrintNvmValue(param.key, param.get(), param.unit);
    }

    cli_printf("nvm-sequence    %lu", static_cast<unsigned long>(nvm_h->Get().nvm_sequence));

    const char* magic_state = " (INVALID)";
    if (nvm_h->Get().magic == kNvmMagic)
    {
        magic_state = " (valid)";
    }
    cli_printf("magic           0x%08lX%s", static_cast<unsigned long>(nvm_h->Get().magic), magic_state);
}

void GetNvm(const char* key)
{
    const NvmParam* param = FindNvmParam(key);
    if (param == nullptr)
    {
        cli_printf("nvm get: unknown key '%s'", key);
        return;
    }
    PrintNvmValue(param->key, param->get(), param->unit);
}

void SetNvm(const char* key, const char* value_str)
{
    const NvmParam* param = FindNvmParam(key);
    if (param == nullptr)
    {
        cli_printf("nvm set: unknown key '%s'", key);
        helpNvm();
        return;
    }
    char* end = nullptr;
    float value = std::strtof(value_str, &end);
    if (end == value_str || *end != '\0')
    {
        cli_printf("nvm set: invalid value '%s'", value_str);
        return;
    }
    param->set(value);
    cli_printf("%s = %s (run `save` to persist)", key, value_str);
}
}  // namespace

void helpNvm()
{
    cli_printf("usage: nvm show                 show all fields");
    cli_printf("       nvm get <key>            show one field");
    cli_printf("       nvm set <key> <value>    set a field");
    cli_printf("keys:  test");
    cli_printf("NOTE: run 'save' after set");
}

void onNvm(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)context;

    const char* action = embeddedCliGetToken(args, 1);
    const char* arg2 = embeddedCliGetToken(args, 2);
    const char* arg3 = embeddedCliGetToken(args, 3);
    const char* arg4 = embeddedCliGetToken(args, 4);

    if (action == NULL)
    {
        helpNvm();
    }
    else if (std::strcmp(action, "show") == 0 && arg2 == NULL)
    {
        ShowNvm();
    }
    else if (std::strcmp(action, "get") == 0 && arg2 != NULL && arg3 == NULL)
    {
        GetNvm(arg2);
    }
    else if (std::strcmp(action, "set") == 0 && arg2 != NULL && arg3 != NULL && arg4 == NULL)
    {
        SetNvm(arg2, arg3);
    }
    else
    {
        helpNvm();
    }
}

void RegisterNvmCliCommands(EmbeddedCli* cli) { embeddedCliAddBinding(cli, &commandNvm); }
