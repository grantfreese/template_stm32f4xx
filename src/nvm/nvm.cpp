// nvm.cpp - dual-slot redundant NVM driver
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#include "nvm/nvm.h"

#include <cstddef>
#include <cstring>

#include "stm32f3xx_hal.h"

namespace
{

// Slot addresses occupy the last two 2 KB pages of the 256 KB STM32F373CC flash.
// The linker script reserves these pages by capping the FLASH region at 252 KB; the
// application image cannot grow into them.
constexpr uintptr_t kSlot0Address{0x0803F000};  // page 126
constexpr uintptr_t kSlot1Address{0x0803F800};  // page 127
constexpr std::size_t kSlotSize{2048};

static_assert(sizeof(NonvolatileStorage_t) <= kSlotSize, "NonvolatileStorage_t must fit within one 2 KB flash page");

uintptr_t SlotAddress(int slot)
{
    if (slot == 0)
    {
        return kSlot0Address;
    }
    return kSlot1Address;
}

uint32_t ReadSlotMagic(int slot)
{
    const uintptr_t addr = SlotAddress(slot) + offsetof(NonvolatileStorage_t, magic);
    return *reinterpret_cast<const uint32_t*>(addr);
}

uint32_t ReadSlotSequence(int slot)
{
    const uintptr_t addr = SlotAddress(slot) + offsetof(NonvolatileStorage_t, nvm_sequence);
    return *reinterpret_cast<const uint32_t*>(addr);
}

bool SlotMagicValid(int slot) { return ReadSlotMagic(slot) == kNvmMagic; }

bool ErasePage(uintptr_t addr)
{
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase{};
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = static_cast<uint32_t>(addr);
    erase.NbPages = 1;
    uint32_t page_error{0};
    HAL_StatusTypeDef rc = HAL_FLASHEx_Erase(&erase, &page_error);
    HAL_FLASH_Lock();
    // HAL returns 0xFFFFFFFF in page_error on success.
    return rc == HAL_OK && page_error == 0xFFFFFFFFU;
}

bool ProgramPage(uintptr_t addr, const uint8_t* data, std::size_t size)
{
    HAL_FLASH_Unlock();
    bool ok{true};
    for (std::size_t i = 0; i < size; i += 2)
    {
        uint16_t halfword{0};
        std::memcpy(&halfword, data + i, 2);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + i, halfword) != HAL_OK)
        {
            ok = false;
            break;
        }
    }
    HAL_FLASH_Lock();
    return ok;
}

bool VerifyPage(uintptr_t addr, const uint8_t* data, std::size_t size)
{
    return std::memcmp(reinterpret_cast<const void*>(addr), data, size) == 0;
}

}  // namespace

Nvm::Edit::~Edit() { owner_.Write(); }

void Nvm::ApplyDefaults()
{
    storage_ = NonvolatileStorage_t{};
    storage_.test_value = kTestValueDefault;
    storage_.nvm_sequence = 0;
    storage_.magic = kNvmMagic;
}

void Nvm::Init()
{
    const bool s0_valid = SlotMagicValid(0);
    const bool s1_valid = SlotMagicValid(1);

    if (!s0_valid && !s1_valid)
    {
        // Both slots are invalid (true first boot, chip erase, or kNvmMagic bump).
        // Seed slot 0 with fresh defaults so the next mutation has a baseline to
        // diff against.
        active_slot_ = -1;
        ApplyDefaults();
        Write();
        return;
    }

    int chosen{0};
    if (s0_valid && !s1_valid)
    {
        chosen = 0;
    }
    else if (s1_valid && !s0_valid)
    {
        chosen = 1;
    }
    else
    {
        // Both slots valid. Pick the higher sequence (newer write). Tie -> slot 0.
        if (ReadSlotSequence(0) >= ReadSlotSequence(1))
        {
            chosen = 0;
        }
        else
        {
            chosen = 1;
        }
    }

    std::memcpy(&storage_, reinterpret_cast<const void*>(SlotAddress(chosen)), sizeof(NonvolatileStorage_t));
    active_slot_ = chosen;
}

bool Nvm::Write()
{
    // Target the OTHER slot so the previously-active slot stays intact in case this
    // write is interrupted. First-ever write (active_slot_ == -1) starts at slot 0.
    int target = 0;
    if (active_slot_ == 0)
    {
        target = 1;
    }

    storage_.nvm_sequence += 1;
    storage_.magic = kNvmMagic;

    const auto* bytes = reinterpret_cast<const uint8_t*>(&storage_);

    if (!ErasePage(SlotAddress(target)))
    {
        return false;
    }
    if (!ProgramPage(SlotAddress(target), bytes, sizeof(NonvolatileStorage_t)))
    {
        return false;
    }
    if (!VerifyPage(SlotAddress(target), bytes, sizeof(NonvolatileStorage_t)))
    {
        return false;
    }

    active_slot_ = target;
    return true;
}
