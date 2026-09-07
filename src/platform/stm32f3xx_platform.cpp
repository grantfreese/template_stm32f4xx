// stm32f3xx_platform.cpp - Implementation of platform for stm32f3xx
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "stm32f3xx_platform.h"

// lib includes

// os includes

// standard includes

namespace fw
{
IPlatform& GetPlatform()
{
    static PlatformSTM32F3 platform{};
    return platform;
}

void PlatformInitialize() { GetPlatform().Initialize(); }
}  // namespace fw
