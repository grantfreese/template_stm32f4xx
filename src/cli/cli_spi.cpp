// cli_spi.cpp - Top-level 'spi' CLI command for SPI peripheral diagnostics
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
//
// 'spi info' prints PCLK1 and PCLK2, then for each enabled SPI peripheral
// the BR field from CR1 and the resulting SCK frequency. Provides a
// hardware-free way to verify SCK against a target rate (e.g. the
// LTC6813's 1 MHz ISOSPI hard limit) without scoping the line.

#include "cli/cli_setup.h"
#include "stm32f3xx_hal.h"

namespace
{

struct SpiEntry
{
    const char* name;
    SPI_TypeDef* regs;
    uint32_t pclk;
    bool enabled;
};

void PrintEntry(const SpiEntry& entry)
{
    if (!entry.enabled)
    {
        cli_printf("  %s: disabled (peripheral clock not enabled)", entry.name);
        return;
    }
    const uint32_t br = (entry.regs->CR1 >> 3) & 0x7;
    const uint32_t sck = entry.pclk >> (br + 1);
    cli_printf("  %s: BR=%lu  /%lu  SCK=%lu Hz", entry.name, br, 1UL << (br + 1), sck);
}

void OnSpi(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)args;
    (void)context;

    const uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    const uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();

    cli_printf("PCLK1 (APB1) = %lu Hz", pclk1);
    cli_printf("PCLK2 (APB2) = %lu Hz", pclk2);

    const SpiEntry entries[] = {
        {"SPI1", SPI1, pclk2, (RCC->APB2ENR & RCC_APB2ENR_SPI1EN) != 0},
        {"SPI2", SPI2, pclk1, (RCC->APB1ENR & RCC_APB1ENR_SPI2EN) != 0},
        {"SPI3", SPI3, pclk1, (RCC->APB1ENR & RCC_APB1ENR_SPI3EN) != 0},
    };
    for (const auto& entry : entries)
    {
        PrintEntry(entry);
    }
}

}  // namespace

void RegisterSpiCliCommands(EmbeddedCli* cli)
{
    static constexpr CliCommandBinding kCommandSpi = {
        .name = "spi",
        .help = "spi info -- PCLK1/PCLK2 and per-peripheral BR + computed SCK",
        .tokenizeArgs = true,
        .context = NULL,
        .binding = OnSpi,
    };
    embeddedCliAddBinding(cli, &kCommandSpi);
}
