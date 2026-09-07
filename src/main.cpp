// main.cpp - Main application entry point
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "main.h"

#include "nvm/nvm.h"
#include "peripheral.h"
#include "tasks/task_can.h"
#include "tasks/task_cli.h"
#include "version.h"

// lib includes
#include "cmsis_os.h"
#include "platform/stm32f3xx_platform.h"

std::string kFirmwareNameString{"stm32f4-template"};
std::string kFirmwareVersionString{PROJECT_VERSION};

static Nvm nvm;
Nvm* nvm_h{&nvm};

int main(void)
{
    fw::PlatformInitialize();
    HAL_Init();
    SystemClock_Config();

    // Scan both NVM slots and pick the higher-sequence valid one. If neither
    // slot has matching magic (first boot, chip erase, or kNvmMagic bump),
    // Init() seeds slot 0 with first-boot defaults internally. After Init()
    // returns, nvm.Get() yields a consistent record.
    nvm.Init();

    // init MCU peripherals
    MX_CAN_Init();

    // init scheduler
    osKernelInitialize();

    taskCli_h = osThreadNew(TaskCli, NULL, &taskAttributesCli);
    taskCan_h = osThreadNew(TaskCan, NULL, &taskAttributesCan);

    // start scheduler
    osKernelStart();

    // should be unreachable since system will never exit scheduler
    while (1)
    {
    }
}
