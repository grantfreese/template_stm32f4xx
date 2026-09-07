// bus.cpp - CAN bus run state (running / halted)
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "can/bus.h"

#include "can/can_tx_queue.h"
#include "stm32f3xx_hal.h"

extern CAN_HandleTypeDef hcan;

namespace fw
{

// Cold-boot initial state is kHalted: MX_CAN_Init leaves the peripheral in init mode and
// the pre-RTOS path never calls HAL_CAN_Start (its unbounded INAK-clear wait can hang the
// boot when CAN_RX is held dominant by an external node). The first SetCanMode(kRunning)
// from the CAN task performs the bounded ExitInitMode; failure is retried on the next
// task tick, so the bus comes up as soon as it is free.
static CanMode can_mode{CanMode::kHalted};

const char* CanModeToString(CanMode mode)
{
    switch (mode)
    {
        case CanMode::kRunning:
            return "running";
        case CanMode::kHalted:
            return "halted";
    }
    return "unknown";
}

CanMode GetCanMode() { return can_mode; }

bool IsCanRunning() { return can_mode == CanMode::kRunning; }

// Enter CAN initialization mode: set MCR.INRQ and wait for MSR.INAK.
// Returns false on timeout. Independent of HAL state -- safe to call whether the
// peripheral was running or already stopped.
static bool EnterInitMode(uint32_t timeout_ms)
{
    SET_BIT(hcan.Instance->MCR, CAN_MCR_INRQ);
    uint32_t deadline = HAL_GetTick() + timeout_ms;
    while ((hcan.Instance->MSR & CAN_MSR_INAK) == 0)
    {
        if (HAL_GetTick() > deadline)
        {
            return false;
        }
    }
    return true;
}

// Exit CAN initialization mode: clear MCR.INRQ and wait for MSR.INAK to clear. Fails if the
// bus is held dominant (the peripheral needs 11 consecutive recessive bits to synchronize).
static bool ExitInitMode(uint32_t timeout_ms)
{
    CLEAR_BIT(hcan.Instance->MCR, CAN_MCR_INRQ);
    uint32_t deadline = HAL_GetTick() + timeout_ms;
    while ((hcan.Instance->MSR & CAN_MSR_INAK) != 0)
    {
        if (HAL_GetTick() > deadline)
        {
            return false;
        }
    }
    return true;
}

bool SetCanMode(CanMode mode)
{
    if (mode == can_mode)
    {
        return true;
    }

    if (mode == CanMode::kHalted)
    {
        can_tx_queue.Flush();
        // Abort hardware TX mailboxes so nothing transmits after the peripheral is parked.
        SET_BIT(hcan.Instance->TSR, CAN_TSR_ABRQ0 | CAN_TSR_ABRQ1 | CAN_TSR_ABRQ2);
        if (!EnterInitMode(10))
        {
            return false;
        }
        // Suppress the TX-mailbox-empty interrupt so the TX queue drainer doesn't fire while
        // parked; kRunning re-activates it.
        HAL_CAN_DeactivateNotification(&hcan, CAN_IT_TX_MAILBOX_EMPTY);
        can_mode = CanMode::kHalted;
        return true;
    }

    // kRunning: leave init mode. Filter banks and bit timing were configured by
    // MX_CAN_Init at boot and survive init mode untouched, so there is nothing to
    // reconfigure here.
    if (!ExitInitMode(10))
    {
        return false;
    }
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_TX_MAILBOX_EMPTY);
    can_mode = CanMode::kRunning;
    return true;
}

}  // namespace fw
