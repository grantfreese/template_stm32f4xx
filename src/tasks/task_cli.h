// task_cli.h - CLI FreeRTOS task declaration
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "cmsis_os.h"
#include "tasks/task_stats.h"

extern osThreadId_t taskCli_h;
extern const osThreadAttr_t taskAttributesCli;

//! @brief CLI task stack size in words.
constexpr unsigned int kStackSizeWordsCli{768};

//! @brief CLI task loop period in milliseconds.
constexpr uint32_t kCliTaskPeriodMsec{50};

void TaskCli(void* argument);

//! @brief Loop statistics for the CLI task. Written only by the CLI task.
const fw::TaskStats& GetCliTaskStats();

//! @brief Request a reset of the CLI task's loop statistics on its next iteration.
void RequestCliTaskStatsReset();

//! @brief Ask the CLI task to perform one final output-buffer drain and exit.
//!
//! Set on shutdown so any pending `cli_printf` bytes reach UART before power
//! is removed or the MCU resets. The CLI task picks up the flag on its next
//! 50 ms wake, drains the embeddedCli buffer + DebugPrintPending one more
//! time, signals complete via IsCliShutdownComplete, then parks itself. After
//! this point any `cli_printf` calls queue into `cliBuffer` with no
//! consumer; the buffer is static so nothing leaks, but the bytes never
//! reach the wire.
void RequestCliShutdown();

//! @brief True once the CLI task has drained its final output and exited.
//!
//! Polled by the shutdown path to confirm anything queued before the request
//! made it to UART before power is removed.
bool IsCliShutdownComplete();
