// can_tx_queue.h - Interrupt-driven CAN TX ring buffer
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <array>
#include <cstdint>

#include "stm32f3xx_hal.h"

// Broadcast message IDs are 29-bit extended IDs in a vendor-private range, leaving the
// entire 11-bit standard-ID space free for other traffic on the bus.
constexpr uint32_t kCanIdBroadcastPrefix{0x1FA00000};
constexpr uint32_t kCanIdFirmwareVersion{kCanIdBroadcastPrefix | 0x001};
constexpr uint32_t kCanIdMcuTemperature{kCanIdBroadcastPrefix | 0x002};

namespace fw
{

//! @brief Identifies one CAN frame type. Used as a compact key into the ExtId table so
//! queue slots do not need to store a full CAN_TxHeaderTypeDef.
enum class CanFrameType : uint8_t
{
    kFirmwareVersion = 0,
    kMcuTemperature,
};

static constexpr int kCanFrameTypeCount{2};

//! @brief One slot in the TX ring buffer — 10 bytes.
struct CanTxFrame
{
    CanFrameType type;  //!< Resolved to ExtId at drain time via the descriptor table.
    uint8_t dlc;        //!< Number of payload bytes (1–8).
    uint8_t data[8];
};

static_assert(sizeof(CanTxFrame) == 10, "CanTxFrame must be 10 bytes");

//! @brief Software ring buffer for CAN TX frames, drained by the TX-complete ISR.
//!
//! Producers call Enqueue() under a critical section. The TX-complete ISR calls
//! DrainToMailboxes() to pop the next frame(s) into free hardware mailboxes. This
//! removes all blocking waits from task-context CAN TX paths.
//!
//! Overflow policy: newest frames are silently dropped. CAN telemetry is best-effort.
class CanTxQueue
{
public:
    //! @brief Enqueue one CAN frame for transmission.
    //!
    //! Safe to call from task context only. Uses a critical section to protect
    //! the tail pointer against concurrent ISR access.
    //!
    //! @param type   Frame type; resolved to ExtId at drain time.
    //! @param dlc    Number of payload bytes (1–8).
    //! @param data   Frame payload; up to dlc bytes are copied.
    //! @return True if enqueued, false if the queue is full (frame dropped).
    bool Enqueue(CanFrameType type, uint8_t dlc, const uint8_t* data);

    //! @brief Pop pending frames into free CAN TX mailboxes.
    //!
    //! Resolves ExtId from the descriptor table, builds CAN_TxHeaderTypeDef on the
    //! stack, and calls HAL_CAN_AddTxMessage. Safe to call from both task context
    //! (initial kick after Enqueue) and ISR context (TX-complete callback). Does
    //! nothing if the queue is empty or all mailboxes are occupied.
    //!
    //! @param hcan  HAL CAN peripheral handle.
    void DrainToMailboxes(CAN_HandleTypeDef* hcan);

    //! @brief Discard all queued frames. Call before halting the CAN peripheral.
    void Flush();

    //! @brief Return the 29-bit extended ID for a frame type.
    uint32_t GetExtId(CanFrameType type) const;

private:
    static constexpr int kCapacity{32};

    std::array<CanTxFrame, kCapacity> buf_{};
    volatile int head_{0};
    volatile int tail_{0};

    bool IsEmpty() const { return head_ == tail_; }
    bool IsFull() const { return ((tail_ + 1) % kCapacity) == head_; }
};

}  // namespace fw

//! @brief Global CAN TX queue shared by all broadcast producers.
extern fw::CanTxQueue can_tx_queue;
