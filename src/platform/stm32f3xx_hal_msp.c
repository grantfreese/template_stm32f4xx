/**
 ******************************************************************************
 * @file         stm32f3xx_hal_msp.c
 * @brief        This file provides code for the MSP Initialization
 *               and de-Initialization codes.
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
#include "pindefs.h"

/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* System interrupt init*/
    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);
}

/**
 * @brief CAN MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hcan: CAN handle pointer
 */
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hcan->Instance == CAN)
    {
        /* Peripheral clock enable */
        __HAL_RCC_CAN1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**CAN GPIO Configuration
        PB8     ------> CAN_RX
        PB9     ------> CAN_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* CAN interrupt Init */
        HAL_NVIC_SetPriority(CAN_TX_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN_TX_IRQn);
        HAL_NVIC_SetPriority(CAN_RX0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN_RX0_IRQn);
        HAL_NVIC_SetPriority(CAN_RX1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN_RX1_IRQn);
    }
}

/**
 * @brief CAN MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hcan: CAN handle pointer
 */
void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{
    if (hcan->Instance == CAN)
    {
        /* Peripheral clock disable */
        __HAL_RCC_CAN1_CLK_DISABLE();

        /**CAN GPIO Configuration
        PB8     ------> CAN_RX
        PB9     ------> CAN_TX
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);

        /* CAN interrupt DeInit */
        HAL_NVIC_DisableIRQ(CAN_TX_IRQn);
        HAL_NVIC_DisableIRQ(CAN_RX0_IRQn);
        HAL_NVIC_DisableIRQ(CAN_RX1_IRQn);
    }
}

/**
 * @brief TIM_Base MSP Initialization
 * This function configures the hardware resources used in this example
 * @param htim_base: TIM_Base handle pointer
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    if (htim_base->Instance == TIM19)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM19_CLK_ENABLE();
    }

    /* Configure PA0 as TIM19 output */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_TIM19;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief TIM_Base MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param htim_base: TIM_Base handle pointer
 */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
    if (htim_base->Instance == TIM19)
    {
        /* Peripheral clock disable */
        __HAL_RCC_TIM19_CLK_DISABLE();
    }
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 */
// USART1 is configured by fw::UartSTM32F3 (src/platform/stm32f3xx_uart.cpp),
// so HAL_UART_MspInit is no longer invoked for it. USART2 is referenced in a
// few places but no longer initialized; its MSP hooks have been removed with
// USART1's to avoid keeping dead branches.
