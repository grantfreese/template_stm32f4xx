// stm32f3xx_uart.cpp - USART class implementation
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "stm32f3xx_uart.h"

#include <cstddef>
#include <cstdint>
#include <span>

#include "framework/error.h"
#include "stm32f3xx_hal.h"

namespace fw
{
UartSTM32F3* UartSTM32F3::instance_usart1_{nullptr};

extern "C" void USART1ErrorInterruptHandler()
{
    if (UartSTM32F3::instance_usart1_ != nullptr)
    {
        UartSTM32F3::instance_usart1_->HandleErrorIsr();
    }
}

UartSTM32F3::UartSTM32F3(USART_TypeDef* uart, uint32_t baud_rate) : uart_{uart}, baud_rate_{baud_rate} {}

void UartSTM32F3::Initialize()
{
    ConfigureRcc();
    ConfigureGpio();
    ConfigureUart();
    ConfigureRxDma();
    RegisterIsr();
}

void UartSTM32F3::ConfigureRcc()
{
    if (uart_ == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
    }
    else
    {
        runtime_assert(false, "UartSTM32F3: unsupported USART instance");
    }
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
}

void UartSTM32F3::ConfigureGpio()
{
    if (uart_ == USART1)
    {
        // USART1 TX = PA9, RX = PA10, AF7.
        GPIO_InitTypeDef init{};
        init.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        init.Mode = GPIO_MODE_AF_PP;
        init.Pull = GPIO_NOPULL;
        init.Speed = GPIO_SPEED_FREQ_HIGH;
        init.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &init);
    }
}

void UartSTM32F3::ConfigureUart()
{
    // BRR at OVER8=0: USARTDIV = fck / baud. USART1 lives on APB2 (PCLK2).
    const uint32_t pclk = (uart_ == USART1) ? HAL_RCC_GetPCLK2Freq() : HAL_RCC_GetPCLK1Freq();
    const uint32_t usartdiv = (pclk + baud_rate_ / 2) / baud_rate_;

    // CR1.UE must be 0 before BRR / data-width / parity / oversampling are written.
    uart_->CR1 = 0;
    uart_->CR2 = 0;
    uart_->CR3 = 0;

    uart_->BRR = usartdiv;

    // 8N1, TE + RE. OVER8=0, PCE=0, M=0 are the implicit defaults.
    uart_->CR1 = USART_CR1_TE | USART_CR1_RE;

    // DMAR routes RX to DMA; EIE pops the error IRQ so latched ORE / FE / NE /
    // PE can be cleared. Overrun in DMA mode still gates further writes to
    // RDR via the shadow register until ORE is acknowledged.
    uart_->CR3 = USART_CR3_DMAR | USART_CR3_EIE;

    uart_->CR1 = uart_->CR1 | USART_CR1_UE;
}

void UartSTM32F3::ConfigureRxDma()
{
    if (uart_ != USART1)
    {
        return;
    }
    // F373 maps USART1_RX to DMA1 channel 5 (rm0316 13.3.7 Table 39).
    rx_dma_channel_ = DMA1_Channel5;

    // GIF / TCIF / HTIF / TEIF for channel 5.
    rx_dma_clear_flags_ = (DMA_IFCR_CGIF5 | DMA_IFCR_CTCIF5 | DMA_IFCR_CHTIF5 | DMA_IFCR_CTEIF5);

    // CMAR / CPAR / CNDTR are read-only while CCR.EN=1.
    rx_dma_channel_->CCR = 0;
    DMA1->IFCR = rx_dma_clear_flags_;

    rx_dma_channel_->CPAR = reinterpret_cast<uint32_t>(&uart_->RDR);
    rx_dma_channel_->CMAR = reinterpret_cast<uint32_t>(rx_ring_);
    rx_dma_channel_->CNDTR = kRxRingSize;

    // Peripheral->memory, byte transfers, MINC, CIRC, PL=medium (leaves the
    // higher priority levels for other DMA users on the controller).
    rx_dma_channel_->CCR = DMA_CCR_MINC | DMA_CCR_CIRC | (0b01 << DMA_CCR_PL_Pos);

    // Enable last; the engine starts copying RDR -> ring on the next RXNE.
    rx_dma_channel_->CCR = rx_dma_channel_->CCR | DMA_CCR_EN;

    rx_read_index_ = 0;
}

void UartSTM32F3::RegisterIsr()
{
    if (uart_ == USART1)
    {
        instance_usart1_ = this;
        HAL_NVIC_SetPriority(USART1_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

size_t UartSTM32F3::DrainRx(RxByteHandler handler, void* user_data)
{
    if (rx_dma_channel_ == nullptr)
    {
        return 0;
    }
    // (kRxRingSize - CNDTR) is the index past the most recently written byte.
    // CNDTR is a single AHB-atomic word; no critical section needed.
    const size_t write_index = kRxRingSize - rx_dma_channel_->CNDTR;
    const size_t drained = DrainRingTo(rx_ring_, kRxRingSize, rx_read_index_, write_index, handler, user_data);

    // Re-arm RX in the unlikely event ORE latched (DMA should keep this clean,
    // but the HW shadow register still gates writes until cleared).
    if (uart_->ISR & USART_ISR_ORE)
    {
        uart_->ICR = USART_ICR_ORECF;
    }
    return drained;
}

void UartSTM32F3::WriteByte(uint8_t byte)
{
    // TXE=1 when TDR has moved into the shift register and is ready for the next byte.
    while ((uart_->ISR & USART_ISR_TXE) == 0)
    {
    }
    uart_->TDR = byte;
}

void UartSTM32F3::Write(std::span<const uint8_t> data)
{
    for (uint8_t b : data)
    {
        WriteByte(b);
    }
    // Wait for transmission complete so callers can assume the line is idle
    // on return -- matters where the next operation is a peripheral reset.
    while ((uart_->ISR & USART_ISR_TC) == 0)
    {
    }
}

void UartSTM32F3::HandleErrorIsr()
{
    // Framing / noise / parity would imply a bench wiring problem; clear
    // them anyway so they don't latch and block subsequent reception.
    if (uart_->ISR & USART_ISR_ORE)
    {
        uart_->ICR = USART_ICR_ORECF;
        overrun_count_ = overrun_count_ + 1;
    }
    if (uart_->ISR & USART_ISR_FE)
    {
        uart_->ICR = USART_ICR_FECF;
    }
    if (uart_->ISR & USART_ISR_NE)
    {
        uart_->ICR = USART_ICR_NCF;
    }
    if (uart_->ISR & USART_ISR_PE)
    {
        uart_->ICR = USART_ICR_PECF;
    }
}

}  // namespace fw
