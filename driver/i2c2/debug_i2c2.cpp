// debug_i2c2.cpp - STM32 I2C2 debug streamer
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include <cstdint>

#include "cli/cli_setup.h"
#include "cli/debug_stage.h"
#include "stm32f3xx_hal.h"

static constexpr int kI2cBufSize = 16;
static constexpr int kI2cBufMask = kI2cBufSize - 1;
static constexpr char kHexDigits[] = "0123456789ABCDEF";

struct I2c2Entry
{
    uint32_t tick_;
    uint8_t addr_;
    uint8_t data_[4];
    uint8_t data_len_;
    char error_[9];  // error label or empty, e.g. "TIMEOUT"
    bool is_read_;
    bool is_error_;
};

static I2c2Entry i2c_buf[kI2cBufSize];
static volatile int i2c_head{0};
static volatile int i2c_tail{0};

static void StageEntry(uint8_t addr, bool is_read, bool is_error, const uint8_t* data, uint8_t len, const char* error)
{
    int next = (i2c_head + 1) & kI2cBufMask;
    if (next == i2c_tail)
    {
        return;
    }

    I2c2Entry& entry = i2c_buf[i2c_head];
    entry.tick_ = HAL_GetTick();
    entry.addr_ = addr;
    entry.is_read_ = is_read;
    entry.is_error_ = is_error;

    uint8_t copy_len = len;
    if (len > 4)
    {
        copy_len = 4;
    }
    for (uint8_t j = 0; j < copy_len; j++)
    {
        entry.data_[j] = data[j];
    }
    entry.data_len_ = copy_len;

    if (is_error && error != nullptr)
    {
        int e = 0;
        for (; error[e] != '\0' && e < 8; e++)
        {
            entry.error_[e] = error[e];
        }
        entry.error_[e] = '\0';
    }
    else
    {
        entry.error_[0] = '\0';
    }

    i2c_head = next;
}

void DebugStageI2c2Data(uint8_t addr, bool is_read, const uint8_t* data, uint8_t len)
{
    if (!IsDebugEnabled(kDebugI2c2))
    {
        return;
    }
    StageEntry(addr, is_read, false, data, len, nullptr);
}

void DebugStageI2c2Error(uint8_t addr, const char* error)
{
    if (!IsDebugEnabled(kDebugI2c2Err))
    {
        return;
    }
    StageEntry(addr, false, true, nullptr, 0, error);
}

static void PrintEntry(const I2c2Entry& entry)
{
    if (entry.is_error_)
    {
        cli_printf("[%6lu ms] I2C2    ERR %02X %s", static_cast<unsigned long>(entry.tick_), entry.addr_, entry.error_);
    }
    else
    {
        char hex[13];  // up to 4 bytes: "XX XX XX XX" + null
        int pos = 0;
        for (int i = 0; i < entry.data_len_; i++)
        {
            if (i > 0)
            {
                hex[pos++] = ' ';
            }
            hex[pos++] = kHexDigits[entry.data_[i] >> 4];
            hex[pos++] = kHexDigits[entry.data_[i] & 0x0F];
        }
        hex[pos] = '\0';

        const char* rw_str = "W";
        if (entry.is_read_)
        {
            rw_str = "R";
        }
        cli_printf("[%6lu ms] I2C2    %s %02X [%d] %s",
                   static_cast<unsigned long>(entry.tick_),
                   rw_str,
                   entry.addr_,
                   entry.data_len_,
                   hex);
    }
}

int DebugPrintI2c2(int budget)
{
    int pending = (i2c_head - i2c_tail) & kI2cBufMask;
    if (budget == 0)
    {
        return pending;
    }

    int printed = 0;
    while (i2c_tail != i2c_head && printed < budget)
    {
        I2c2Entry entry = i2c_buf[i2c_tail];
        i2c_tail = (i2c_tail + 1) & kI2cBufMask;
        PrintEntry(entry);
        printed++;
    }
    return printed;
}
