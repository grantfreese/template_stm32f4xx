// task_cli.h - CLI FreeRTOS task declaration
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "cmsis_os.h"

extern osThreadId_t taskCli_h;
extern const osThreadAttr_t taskAttributesCli;

//! Stack size for the CLI task, exposed so the `status` command can report
//! peak usage as a fraction of the declared size.
constexpr unsigned int kStackSizeWordsCli{768};

void TaskCli(void* argument);

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
