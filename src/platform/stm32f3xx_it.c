/**
 ******************************************************************************
 * @file    stm32f3xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* External variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan;
extern TIM_HandleTypeDef htim2;

// USART1 is owned by fw::UartSTM32F3; the IRQ vector routes through the
// driver's error handler instead of HAL_UART_IRQHandler.
extern void USART1ErrorInterruptHandler(void);

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Prefetch fault, memory access fault.
 */
void BusFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {}

/******************************************************************************/
/* STM32F3xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f373c8tx.s).                */
/******************************************************************************/

/**
 * @brief This function handles CAN TX mailbox empty interrupt.
 */
void CAN_TX_IRQHandler(void) { HAL_CAN_IRQHandler(&hcan); }

/**
 * @brief This function handles CAN RX0 interrupt.
 */
void CAN_RX0_IRQHandler(void) { HAL_CAN_IRQHandler(&hcan); }

/**
 * @brief This function handles CAN RX1 interrupt.
 */
void CAN_RX1_IRQHandler(void) { HAL_CAN_IRQHandler(&hcan); }

/**
 * @brief This function handles TIM2 global interrupt (HAL timebase tick).
 */
void TIM2_IRQHandler(void) { HAL_TIM_IRQHandler(&htim2); }

void USART1_IRQHandler(void) { USART1ErrorInterruptHandler(); }
