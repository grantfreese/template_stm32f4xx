// task_cli.cpp - CLI FreeRTOS task implementation
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "tasks/task_cli.h"

#include "cli/cli_debug.h"
#include "cli/cli_setup.h"
#include "cmsis_os.h"
#include "embedded_cli.h"
#include "main.h"

// clang-format off
static StaticTask_t taskCli;
static StackType_t stackCli[kStackSizeWordsCli];

osThreadId_t taskCli_h;

const osThreadAttr_t taskAttributesCli = {
    .name       = "cli",
    .attr_bits  = 0,
    .cb_mem     = &taskCli,
    .cb_size    = sizeof(taskCli),
    .stack_mem  = stackCli,
    .stack_size = kStackSizeWordsCli * sizeof(StackType_t),
    .priority   = (osPriority_t)osPriorityNormal,
    .tz_module  = 0,
    .reserved   = 0
};
// clang-format on

// volatile is sufficient on Cortex-M4: aligned single-word read/write is
// atomic, and the flag is one-way (false -> true, no roll-back). No need for
// a full mutex / atomic.
static volatile bool cli_shutdown_requested{false};
static volatile bool cli_shutdown_complete{false};

void RequestCliShutdown() { cli_shutdown_requested = true; }

bool IsCliShutdownComplete() { return cli_shutdown_complete; }

void TaskCli(void* argument)
{
    (void)argument;

    // initialize CLI
    CliSetup(&kFirmwareNameString, &kFirmwareVersionString);

    // thread loop
    for (;;)
    {
        DrainCliUart();
        embeddedCliProcess(getCliPointer());
        DebugPrintPending();

        if (cli_shutdown_requested)
        {
            // Final drain. DebugPrintPending() *queues* lines via cli_printf
            // and a second embeddedCliProcess() is what actually pushes those
            // bytes to UART. Order matters: queue first, then drain. Without
            // the trailing drain, anything DebugPrintPending queued sits in
            // cliBuffer and is lost when power goes away.
            DebugPrintPending();
            DrainCliUart();
            embeddedCliProcess(getCliPointer());
            cli_shutdown_complete = true;
            // Idle from here on. Cannot call osThreadTerminate -- the firmware
            // does not enable INCLUDE_vTaskDelete (would pull in vTaskDelete +
            // dynamic-alloc bookkeeping for a one-shot path). Sitting in a
            // long osDelay parks the task until the shutdown path removes
            // power or resets.
            for (;;)
            {
                osDelay(1000);
            }
        }

        osDelay(50);
    }
}
