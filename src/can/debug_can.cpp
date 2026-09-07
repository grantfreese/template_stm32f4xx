// debug_can.cpp - CAN debug streamer
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include <cstdint>
#include <cstring>

#include "cli/cli_setup.h"
#include "cli/debug_stage.h"
#include "stm32f3xx_hal.h"

static constexpr int kCanBufSize = 32;
static constexpr int kCanBufMask = kCanBufSize - 1;
static constexpr char kHexDigits[] = "0123456789ABCDEF";

struct CanEntry
{
    uint32_t tick_;
    uint32_t id_;
    uint8_t data_[8];
    uint8_t dlc_;
    uint8_t flags_;  // bit 0: is_tx, bit 1: is_extended
};

static CanEntry can_buf[kCanBufSize];
static volatile int can_head{0};
static volatile int can_tail{0};
static volatile uint32_t can_dropped_count{0};

void DebugStageCanFrame(uint32_t id, const uint8_t* data, uint8_t dlc, bool is_tx, bool is_extended)
{
    if (!IsDebugEnabled(kDebugCan))
    {
        return;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    int next = (can_head + 1) & kCanBufMask;
    if (next != can_tail)
    {
        can_buf[can_head].tick_ = HAL_GetTick();
        can_buf[can_head].id_ = id;
        uint8_t len = dlc;
        if (dlc > 8)
        {
            len = 8;
        }
        memcpy(can_buf[can_head].data_, data, len);
        can_buf[can_head].dlc_ = dlc;
        uint8_t flags = 0;
        if (is_tx)
        {
            flags |= 1U;
        }
        if (is_extended)
        {
            flags |= 2U;
        }
        can_buf[can_head].flags_ = flags;
        can_head = next;
    }
    else
    {
        can_dropped_count = can_dropped_count + 1;
    }

    __set_PRIMASK(primask);
}

static void PrintCanEntry(const CanEntry& entry)
{
    const char* dir = "RX";
    if (entry.flags_ & 1U)
    {
        dir = "TX";
    }

    char hex[25];
    int pos = 0;
    for (int i = 0; i < entry.dlc_ && i < 8; i++)
    {
        if (i > 0)
        {
            hex[pos++] = ' ';
        }
        hex[pos++] = kHexDigits[entry.data_[i] >> 4];
        hex[pos++] = kHexDigits[entry.data_[i] & 0x0F];
    }
    hex[pos] = '\0';

    if (entry.flags_ & 2U)
    {
        cli_printf("[%6lu ms] CAN %s %08lX [%d] %s",
                   static_cast<unsigned long>(entry.tick_),
                   dir,
                   static_cast<unsigned long>(entry.id_),
                   entry.dlc_,
                   hex);
    }
    else
    {
        cli_printf("[%6lu ms] CAN %s %03lX [%d] %s",
                   static_cast<unsigned long>(entry.tick_),
                   dir,
                   static_cast<unsigned long>(entry.id_),
                   entry.dlc_,
                   hex);
    }
}

int DebugPrintCan(int budget)
{
    uint32_t dropped = can_dropped_count;
    int pending = (can_head - can_tail) & kCanBufMask;
    if (dropped > 0)
    {
        pending++;  // dropped-frame summary line
    }

    if (budget == 0)
    {
        return pending;
    }

    int printed = 0;

    if (dropped > 0 && printed < budget)
    {
        can_dropped_count = 0;
        cli_printf("[debug] dropped %lu CAN frames", static_cast<unsigned long>(dropped));
        printed++;
    }

    while (can_tail != can_head && printed < budget)
    {
        CanEntry entry = can_buf[can_tail];
        can_tail = (can_tail + 1) & kCanBufMask;
        PrintCanEntry(entry);
        printed++;
    }

    return printed;
}
