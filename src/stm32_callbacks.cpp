// stm32_callbacks.cpp - STM32 HAL weak-override callbacks
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "stm32_callbacks.h"

#include "cli/debug_stage.h"
#include "peripheral.h"

//! @brief Period elapsed callback in non-blocking mode.
//!
//! Called from HAL_TIM_IRQHandler when the TIM2 update interrupt fires. TIM2 is the
//! HAL timebase (see stm32f3xx_hal_timebase_tim.c); its 1 ms tick drives HAL_IncTick.
//! @param htim TIM handle
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM2)
    {
        HAL_IncTick();
    }
}

//! @brief CAN RX FIFO0 message-pending callback.
//!
//! All received frames land in FIFO0 (accept-all filter). The template has no RX
//! protocol yet; frames are staged for the CAN debug streamer and otherwise dropped.
//! @param hcan1 HAL CAN peripheral handle
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan1)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    if (HAL_CAN_GetRxMessage(hcan1, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
    {
        return;
    }

    if (rx_header.IDE == CAN_ID_EXT)
    {
        DebugStageCanFrame(rx_header.ExtId, rx_data, static_cast<uint8_t>(rx_header.DLC), false, true);
    }
    else
    {
        DebugStageCanFrame(rx_header.StdId, rx_data, static_cast<uint8_t>(rx_header.DLC), false, false);
    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 */
void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
