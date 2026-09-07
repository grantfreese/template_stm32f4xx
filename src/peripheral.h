// peripheral.h - Clock tree and MCU peripheral init
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "pindefs.h"

// prototypes
void SystemClock_Config(void);
void MX_CAN_Init(void);

extern CAN_HandleTypeDef hcan;
