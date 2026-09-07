// cli_simple.cpp - `simple` CLI command: enable/disable simple CLI mode
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
//
// Simple mode disables embedded-cli autocomplete (no ghost-text suggestions on
// each keystroke, no TAB / Enter expansion of partial commands). With autocomplete
// off the library's printLiveAutocompletion bails at the top, which is the only
// path that emits cursor save/restore (\e[s / \e[u). The remaining escape sources
// in embedded-cli (cursor movement on backspace, arrow-key history navigation)
// only fire on operator input that automated drivers don't send, so the wire
// output becomes plain text without any post-output filtering required.
//
// Intended for automated drivers (HIL tests, scripts) that need a deterministic,
// redraw-free byte stream. Session-scoped: resets to OFF (full-fancy CLI) on
// every boot.

#include <cstring>

#include "cli/cli_binding.h"
#include "cli/cli_setup.h"

static bool simple_mode_enabled{false};

static bool IsEnabled() { return simple_mode_enabled; }

static void Apply(bool enabled)
{
    // Simple mode means autocomplete OFF -- that's the only knob; no post-output
    // filtering is needed (see file header).
    embeddedCliSetAutoComplete(getCliPointer(), !enabled);
    simple_mode_enabled = enabled;
}

static void PrintState()
{
    const char* state = "OFF";
    if (IsEnabled())
    {
        state = "ON";
    }
    cli_printf("simple: %s", state);
}

static void PrintUsage()
{
    cli_printf("usage: simple              toggle simple CLI mode");
    cli_printf("       simple on           enable (no autocomplete, no escape sequences)");
    cli_printf("       simple off          disable (default fancy mode)");
    cli_printf("       simple status       show current state");
}

static void OnSimple(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)context;

    const char* arg1 = embeddedCliGetToken(args, 1);

    if (arg1 == NULL)
    {
        Apply(!IsEnabled());
        PrintState();
    }
    else if (strcmp(arg1, "on") == 0)
    {
        Apply(true);
        PrintState();
    }
    else if (strcmp(arg1, "off") == 0)
    {
        Apply(false);
        PrintState();
    }
    else if (strcmp(arg1, "status") == 0)
    {
        PrintState();
    }
    else
    {
        PrintUsage();
    }
}

static constexpr CliCommandBinding kCommandSimple = {
    .name = "simple",
    .help = "Toggle simple CLI mode (on => no autocomplete, no escape sequences)",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = OnSimple,
};

void RegisterSimpleCliCommands(EmbeddedCli* cli) { embeddedCliAddBinding(cli, &kCommandSimple); }
