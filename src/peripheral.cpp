// peripheral.cpp - Clock tree and MCU peripheral init
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

// local includes
#include "peripheral.h"

// hal device handles
CAN_HandleTypeDef hcan;

// Clock tree (no board-specific crystal assumed -- HSI-driven PLL, valid on any STM32F373):
//   HSI = 8 MHz -> /2 (fixed HSI prediv) -> 4 MHz -> PLLMUL=16 -> SYSCLK = 64 MHz
//   AHB  /1 -> HCLK  = 64 MHz
//   APB1 /2 -> PCLK1 = 32 MHz   (CAN reference, TIM2 timebase)
//   APB2 /1 -> PCLK2 = 64 MHz   (USART1, SPI1)
// Boards with an HSE crystal should switch PLLSource to RCC_PLLSOURCE_HSE and pick an
// HSEPredivValue/PLLMUL pair that lands on the same 64 MHz (e.g. 20 MHz / 5 * 16). HSI is
// ~1 % accurate; that is fine for the core clock and CAN bit timing margins, but
// crystal-referenced boards get tighter CAN timing and accurate USART baud rates.
// FLASH_LATENCY_2: 2 wait states for HCLK > 48 MHz (RM0313).
void SystemClock_Config(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();

    RCC_OscInitTypeDef RCC_OscInitStruct{};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitTypeDef RCC_ClkInitStruct{};
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }

    // Kernel clock selections for the peripherals the template drives:
    // USART1 from PCLK2; I2C2 from HSI (the I2C timing constants in the i2c2
    // driver assume the 8 MHz HSI kernel clock); ADC1 from PCLK2/8 = 8 MHz
    // (datasheet max 14 MHz -- the reset default PCLK2/2 = 32 MHz is out of spec).
    RCC_PeriphCLKInitTypeDef PeriphClkInit{};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_I2C2 | RCC_PERIPHCLK_ADC1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_HSI;
    PeriphClkInit.Adc1ClockSelection = RCC_ADC1PCLK2_DIV8;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

// CAN bit timing.
// 16 TQ/bit, sample point 75%.
// bitrate = PCLK1 / (kCanPrescaler * (1 + BS1 + BS2)) = 32e6 / (2 * 16) = 1.000 Mbit/s.
constexpr uint32_t kCanPrescaler{2};
constexpr uint32_t kCanSjw{CAN_SJW_3TQ};
constexpr uint32_t kCanBs1{CAN_BS1_11TQ};
constexpr uint32_t kCanBs2{CAN_BS2_4TQ};

void MX_CAN_Init(void)
{
    hcan.Instance = CAN;
    hcan.Init.Prescaler = kCanPrescaler;
    hcan.Init.Mode = CAN_MODE_NORMAL;
    hcan.Init.SyncJumpWidth = kCanSjw;
    hcan.Init.TimeSeg1 = kCanBs1;
    hcan.Init.TimeSeg2 = kCanBs2;
    hcan.Init.TimeTriggeredMode = DISABLE;
    // ABOM=1: hardware auto-recovers from BUS-OFF after 128 occurrences of 11
    // consecutive recessive bits. Without it, a single TEC-saturating event (e.g.
    // bus-rate mismatch) wedges the peripheral permanently.
    hcan.Init.AutoBusOff = ENABLE;
    hcan.Init.AutoWakeUp = DISABLE;
    hcan.Init.AutoRetransmission = DISABLE;
    hcan.Init.ReceiveFifoLocked = DISABLE;
    hcan.Init.TransmitFifoPriority = ENABLE;

    if (HAL_CAN_Init(&hcan) != HAL_OK)
    {
        Error_Handler();
    }

    // Accept every frame (standard and extended) into RX FIFO0. The template has no RX
    // protocol; received frames are only surfaced through the CAN debug streamer.
    CAN_FilterTypeDef accept_all{};
    accept_all.FilterBank = 0;
    accept_all.FilterMode = CAN_FILTERMODE_IDMASK;
    accept_all.FilterScale = CAN_FILTERSCALE_32BIT;
    accept_all.FilterIdHigh = 0x0000;
    accept_all.FilterIdLow = 0x0000;
    accept_all.FilterMaskIdHigh = 0x0000;
    accept_all.FilterMaskIdLow = 0x0000;
    accept_all.FilterFIFOAssignment = CAN_RX_FIFO0;
    accept_all.FilterActivation = ENABLE;
    accept_all.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan, &accept_all) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY);

    // The peripheral stays parked in init mode here -- deliberately no HAL_CAN_Start,
    // whose unbounded INAK-clear wait can hang the boot if the bus is stuck dominant.
    // The CAN task performs the bounded init-mode exit via SetCanMode(kRunning) and
    // retries it every task tick until the bus is free.
}
