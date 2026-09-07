// cli_status.cpp - Top-level 'status' CLI command for task loop statistics and stack usage
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include <cstring>

#include "FreeRTOS.h"
#include "cli/cli_setup.h"
#include "task.h"
#include "tasks/task_can.h"
#include "tasks/task_cli.h"
#include "tasks/task_stats.h"

namespace
{

struct TaskEntry
{
    const char* name;
    osThreadId_t* handle;
    uint32_t stack_words;
    uint32_t period_ms;
    const fw::TaskStats& (*stats)();
    void (*request_reset)();
};

constexpr TaskEntry kTasks[] = {
    {"cli", &taskCli_h, kStackSizeWordsCli, kCliTaskPeriodMsec, GetCliTaskStats, RequestCliTaskStatsReset},
    {"can", &taskCan_h, kStackSizeWordsCan, kCanTaskPeriodMsec, GetCanTaskStats, RequestCanTaskStatsReset},
};

void PrintEntry(const TaskEntry& entry)
{
    if (*entry.handle == nullptr)
    {
        cli_printf("%s: not created", entry.name);
        return;
    }

    const fw::TaskStats& stats = entry.stats();
    const uint32_t remaining_words = uxTaskGetStackHighWaterMark((TaskHandle_t)*entry.handle);
    const uint32_t used_words = entry.stack_words - remaining_words;
    const uint32_t used_percent = (used_words * 100) / entry.stack_words;

    cli_printf("%s: loops=%lu work min/avg/max=%lu/%lu/%lu ms period=%lu ms misses=%lu stack=%lu/%lu words (%lu%%)",
               entry.name,
               stats.GetCount(),
               stats.GetMinMs(),
               stats.GetAvgMs(),
               stats.GetMaxMs(),
               entry.period_ms,
               stats.GetDeadlineMissCount(),
               used_words,
               entry.stack_words,
               used_percent);
}

void OnStatus(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)context;

    const char* token = embeddedCliGetToken(args, 1);
    if (token != nullptr && strcmp(token, "reset") == 0)
    {
        for (const auto& entry : kTasks)
        {
            entry.request_reset();
        }
        cli_printf("Task statistics reset requested");
        return;
    }
    if (token != nullptr)
    {
        cli_printf("Error: unknown argument '%s'; usage: status [reset]", token);
        return;
    }

    for (const auto& entry : kTasks)
    {
        PrintEntry(entry);
    }
}

}  // namespace

void RegisterStatusCliCommands(EmbeddedCli* cli)
{
    static constexpr CliCommandBinding kCommandStatus = {
        .name = "status",
        .help = "status [reset] -- per-task loop statistics and stack usage",
        .tokenizeArgs = true,
        .context = NULL,
        .binding = OnStatus,
    };
    embeddedCliAddBinding(cli, &kCommandStatus);
}
