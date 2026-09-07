// debug_stage.h - Debug staging API for drivers and tasks
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
//
// This header provides the debug flag enum, flag queries, and staging functions
// without pulling in CLI or embedded_cli dependencies. Drivers should include
// this header; application code that also needs CLI bindings includes cli_debug.h.

#pragma once

#include <cstdint>

//! @brief Bit positions in the debug flags bitfield.
enum DebugFlag : uint32_t
{
    kDebugMaster = (1U << 0),   //!< Global gate for all debug output
    kDebugCan = (1U << 1),      //!< CAN TX/RX frame logging
    kDebugAdc1 = (1U << 2),     //!< ADC1 voltage/raw periodic print
    kDebugI2c2 = (1U << 3),     //!< STM32 I2C2 transaction data logging
    kDebugI2c2Err = (1U << 4),  //!< STM32 I2C2 error logging (NAK, timeout)
};

//! @brief Get the current debug flags bitfield.
uint32_t GetDebugFlags();

//! @brief Set the debug flags bitfield.
void SetDebugFlags(uint32_t flags);

//! @brief Get the debug print period in milliseconds.
uint32_t GetDebugPrintPeriodMsec();

//! @brief Set the debug print rate in Hz.
void SetDebugPrintRateHz(float hz);

//! @brief Check if a specific debug source should print (master AND source both set).
//! @param source_flag The DebugFlag bit to check.
//! @return True if both master enable and the source bit are set.
bool IsDebugEnabled(uint32_t source_flag);

//! @brief Stage an ADC1 debug sample for printing by the CLI task.
//!
//! The values are stored and a pending flag is set. The CLI task calls
//! DebugPrintPending() to drain the buffer via cli_printf.
//! @param tick_msec  Current RTOS tick in milliseconds.
//! @param raw        12-bit ADC raw code (0-4095).
//! @param voltage    Converted voltage in volts.
void DebugStageAdc1(uint32_t tick_msec, uint16_t raw, float voltage);

//! @brief Stage a CAN frame for debug printing.
//!
//! Safe to call from both task and ISR context. Frames are stored in a ring
//! buffer and printed by the CLI task via DebugPrintPending().
//!
//! @param id          CAN arbitration ID (11-bit standard or 29-bit extended).
//! @param data        Pointer to the frame payload (up to 8 bytes).
//! @param dlc         Data length code (0-8).
//! @param is_tx       True for transmitted frames, false for received frames.
//! @param is_extended True for 29-bit extended ID, false for 11-bit standard ID.
void DebugStageCanFrame(uint32_t id, const uint8_t* data, uint8_t dlc, bool is_tx, bool is_extended);

//! @brief Stage a successful STM32 I2C2 transaction for debug printing.
//!
//! @param addr    7-bit I2C device address.
//! @param is_read True for read transactions, false for writes.
//! @param data    Pointer to the transaction payload.
//! @param len     Number of payload bytes (up to 4 stored).
void DebugStageI2c2Data(uint8_t addr, bool is_read, const uint8_t* data, uint8_t len);

//! @brief Stage a STM32 I2C2 error for debug printing.
//!
//! @param addr   7-bit I2C device address.
//! @param error  Short error label (e.g., "TIMEOUT").
void DebugStageI2c2Error(uint8_t addr, const char* error);

//! @brief Print any pending debug messages via cli_printf.
//!
//! Must be called from the CLI task loop so that UART writes are serialized
//! with the embedded-cli cursor management.
void DebugPrintPending();
