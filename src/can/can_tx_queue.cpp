// can_tx_queue.cpp - Interrupt-driven CAN TX ring buffer and TX-complete callbacks
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "can/can_tx_queue.h"

#include <cstring>

#include "FreeRTOS.h"
#include "task.h"

fw::CanTxQueue can_tx_queue;

namespace fw
{

// ExtId lookup table indexed by the numeric value of CanFrameType. Values must stay in
// sync with the kCanId* constants in can_tx_queue.h.
static constexpr uint32_t kExtIds[kCanFrameTypeCount] = {
    kCanIdFirmwareVersion,  // kFirmwareVersion
    kCanIdMcuTemperature,   // kMcuTemperature
};

bool CanTxQueue::Enqueue(CanFrameType type, uint8_t dlc, const uint8_t* data)
{
    taskENTER_CRITICAL();
    bool full = IsFull();
    if (!full)
    {
        buf_[tail_].type = type;
        buf_[tail_].dlc = dlc;
        std::memcpy(buf_[tail_].data, data, dlc);
        tail_ = (tail_ + 1) % kCapacity;
    }
    taskEXIT_CRITICAL();
    return !full;
}

void CanTxQueue::DrainToMailboxes(CAN_HandleTypeDef* hcan)
{
    uint32_t mailbox;
    while (!IsEmpty() && HAL_CAN_GetTxMailboxesFreeLevel(hcan) > 0)
    {
        const CanTxFrame& slot = buf_[head_];

        CAN_TxHeaderTypeDef header;
        header.StdId = 0;
        header.ExtId = GetExtId(slot.type);
        header.IDE = CAN_ID_EXT;
        header.RTR = CAN_RTR_DATA;
        header.DLC = slot.dlc;
        header.TransmitGlobalTime = DISABLE;

        if (HAL_CAN_AddTxMessage(hcan, &header, slot.data, &mailbox) == HAL_OK)
        {
            head_ = (head_ + 1) % kCapacity;
        }
        else
        {
            break;
        }
    }
}

void CanTxQueue::Flush()
{
    taskENTER_CRITICAL();
    head_ = 0;
    tail_ = 0;
    taskEXIT_CRITICAL();
}

uint32_t CanTxQueue::GetExtId(CanFrameType type) const { return kExtIds[static_cast<int>(type)]; }

}  // namespace fw

// TX-complete callbacks — called by the HAL CAN IRQ handler for each mailbox.
// Each pops the next pending frame(s) from the software queue into the now-free mailbox.

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef* hcan) { can_tx_queue.DrainToMailboxes(hcan); }

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef* hcan) { can_tx_queue.DrainToMailboxes(hcan); }

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef* hcan) { can_tx_queue.DrainToMailboxes(hcan); }
