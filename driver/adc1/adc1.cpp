#include "adc1/adc1.h"

#include <cstdio>

#include "stm32f3xx_hal.h"

namespace fw
{

// Factory temperature-sensor calibration: raw ADC readings of channel 16 taken by ST at
// 30 C and 110 C with VDDA = 3.3 V (STM32F37x datasheet, "Temperature sensor calibration").
static const volatile uint16_t* const kTsCal1Addr{reinterpret_cast<const volatile uint16_t*>(0x1FFFF7B8U)};
static const volatile uint16_t* const kTsCal2Addr{reinterpret_cast<const volatile uint16_t*>(0x1FFFF7C2U)};
constexpr float kTsCal1TempC{30.0f};
constexpr float kTsCal2TempC{110.0f};

void Adc1::Initialize()
{
    // Enable GPIOA clock and configure PA1 (ADC1_IN1) as analog input.
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio_init{
        .Pin = GPIO_PIN_1,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NOPULL,
    };
    HAL_GPIO_Init(GPIOA, &gpio_init);

    // Enable ADC1 peripheral clock
    RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_ADC1EN;

    // Power on ADC1
    ADC1->CR2 = ADC1->CR2 | ADC_CR2_ADON;

    // Brief stabilization delay (~1 us at 72 MHz)
    for (uint32_t i = 0; i < 72; ++i)
    {
        __NOP();
    }

    // Reset calibration
    ADC1->CR2 = ADC1->CR2 | ADC_CR2_RSTCAL;
    while ((ADC1->CR2 & ADC_CR2_RSTCAL) != 0U)
    {
    }

    // Run calibration
    ADC1->CR2 = ADC1->CR2 | ADC_CR2_CAL;
    while ((ADC1->CR2 & ADC_CR2_CAL) != 0U)
    {
    }

    // Sample times: 239.5 cycles (longest) for channel 1 (SMPR2) and channel 16 (SMPR1).
    // At the 8 MHz ADC kernel clock that is ~30 us, above the temperature sensor's
    // 17.1 us minimum sampling time.
    ADC1->SMPR2 = ADC1->SMPR2 | ADC_SMPR2_SMP1;
    ADC1->SMPR1 = ADC1->SMPR1 | ADC_SMPR1_SMP16;

    // Single regular conversion; ConvertChannel() writes SQ1 per call.
    // SQR1 L[3:0] = 0000 (1 conversion)
    ADC1->SQR1 = 0;
    ADC1->SQR3 = 1U;

    // Wake the internal temperature sensor / VREFINT (channel 16 reads 0 without it)
    ADC1->CR2 = ADC1->CR2 | ADC_CR2_TSVREFE;

    // Configure for software trigger: EXTSEL[2:0] = 111 (SWSTART), EXTTRIG = 1
    ADC1->CR2 = ADC1->CR2 | ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;
}

uint16_t Adc1::ConvertChannel(uint32_t channel)
{
    // Select the channel as the single regular conversion: SQR3 SQ1[4:0]
    ADC1->SQR3 = channel & 0x1FU;

    // Clear EOC flag
    ADC1->SR = ADC1->SR & ~ADC_SR_EOC;

    // Start conversion via SWSTART
    ADC1->CR2 = ADC1->CR2 | ADC_CR2_SWSTART;

    // Poll for end-of-conversion with bounded timeout (~100K iterations, ~14 ms at 64 MHz).
    // A single 12-bit conversion at 239.5 + 12.5 = 252 cycles takes ~32 us at 8 MHz ADC clock.
    for (uint32_t timeout = 100'000U; timeout != 0U; --timeout)
    {
        if ((ADC1->SR & ADC_SR_EOC) != 0U)
        {
            return static_cast<uint16_t>(ADC1->DR & 0x0FFFU);
        }
    }
    // Conversion did not complete; return 0 as sentinel.
    return 0;
}

uint16_t Adc1::ReadRaw() { return ConvertChannel(1U); }

uint16_t Adc1::ReadDieTemperatureRaw() { return ConvertChannel(16U); }

float Adc1::ReadDieTemperatureCelsius() { return DieTemperatureCelsiusFromRaw(ReadDieTemperatureRaw()); }

float Adc1::DieTemperatureCelsiusFromRaw(uint16_t raw)
{
    float cal1 = static_cast<float>(*kTsCal1Addr);
    float cal2 = static_cast<float>(*kTsCal2Addr);
    return kTsCal1TempC + (static_cast<float>(raw) - cal1) * (kTsCal2TempC - kTsCal1TempC) / (cal2 - cal1);
}

float Adc1::ReadVoltageVolts()
{
    uint16_t raw = ReadRaw();
    return static_cast<float>(raw) / 4096.0f * kVref;
}

void Adc1::PrintDiagnostics()
{
    // Trigger a fresh conversion
    ADC1->SR = ADC1->SR & ~ADC_SR_EOC;
    ADC1->CR2 = ADC1->CR2 | ADC_CR2_SWSTART;
    while ((ADC1->SR & ADC_SR_EOC) == 0U)
    {
    }
    uint32_t dr = ADC1->DR;

    printf("--- ADC1 diag ---\r\n");
    printf("  CR1   = 0x%08lX\r\n", static_cast<unsigned long>(ADC1->CR1));
    printf("  CR2   = 0x%08lX\r\n", static_cast<unsigned long>(ADC1->CR2));
    printf("  SR    = 0x%08lX\r\n", static_cast<unsigned long>(ADC1->SR));
    printf("  SQR1  = 0x%08lX\r\n", static_cast<unsigned long>(ADC1->SQR1));
    printf("  SQR3  = 0x%08lX  (SQ1=%lu)\r\n",
           static_cast<unsigned long>(ADC1->SQR3),
           static_cast<unsigned long>(ADC1->SQR3 & 0x1FU));
    printf("  SMPR2 = 0x%08lX  (SMP1=%lu)\r\n",
           static_cast<unsigned long>(ADC1->SMPR2),
           static_cast<unsigned long>((ADC1->SMPR2 >> ADC_SMPR2_SMP1_Pos) & 0x7U));
    printf(
        "  DR    = 0x%08lX  (raw=%lu)\r\n", static_cast<unsigned long>(dr), static_cast<unsigned long>(dr & 0x0FFFU));
    printf("-----------------\r\n");
}

}  // namespace fw
