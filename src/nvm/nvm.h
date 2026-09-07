// nvm.h - dual-slot redundant NVM driver
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <cstdint>

#include "main.h"

//! @brief Dual-slot redundant NVM driver.
//!
//! Owns the NonvolatileStorage_t record and persists it to two redundant 2 KB
//! flash pages at the top of flash (0x0803F000 and 0x0803F800). Every write
//! targets the inactive slot first, programs and verifies it, and only then
//! promotes it to active. Power loss at any point during the write leaves the
//! OTHER slot intact, so stored settings can never be lost to a partial-write
//! event.
//!
//! Read access goes through Get() and returns a const reference. The struct
//! cannot be mutated through that path; mutation must go through the RAII
//! Edit handle returned by BeginEdit(), which persists on scope exit.
//!
//! Slot selection on boot: scan both slots, pick the one whose magic field
//! matches kNvmMagic AND whose nvm_sequence is highest. If both slots have
//! invalid magic (true first boot, chip erase, or kNvmMagic bump after a
//! struct layout change), Init() applies first-boot defaults via
//! ApplyDefaults() and seeds slot 0.
class Nvm
{
public:
    //! @brief RAII edit handle. Modify the struct via operator->; the change
    //! is persisted to flash when the handle goes out of scope.
    class Edit
    {
    public:
        ~Edit();
        Edit(const Edit&) = delete;
        Edit& operator=(const Edit&) = delete;
        Edit(Edit&&) = delete;
        Edit& operator=(Edit&&) = delete;

        NonvolatileStorage_t* operator->() { return &owner_.storage_; }
        NonvolatileStorage_t& operator*() { return owner_.storage_; }

    private:
        friend class Nvm;
        explicit Edit(Nvm& owner) : owner_(owner) {}
        Nvm& owner_;
    };

    Nvm() = default;
    Nvm(const Nvm&) = delete;
    Nvm& operator=(const Nvm&) = delete;

    //! @brief Boot-time read. Scans both slots and copies the chosen one into
    //! the internal storage. If neither slot has matching magic, applies
    //! first-boot defaults and seeds slot 0 so the record is always live
    //! after Init() returns.
    void Init();

    //! @brief Read-only access to the live record. Returns by const reference
    //! so the struct cannot be mutated through this path.
    const NonvolatileStorage_t& Get() const { return storage_; }

    //! @brief Open an RAII edit window. The returned handle exposes the
    //! mutable struct via operator->. On destruction the handle persists the
    //! current contents to flash atomically.
    Edit BeginEdit() { return Edit(*this); }

    //! @brief Writable accessor for compound multi-call transactions that
    //! cannot fit inside one Edit scope. Prefer BeginEdit() for everything
    //! else -- it auto-persists and cannot be confused with a read-only Get().
    //!
    //! Callers of Mutate() MUST call Write() once the multi-step transaction
    //! is complete.
    NonvolatileStorage_t& Mutate() { return storage_; }

    //! @brief Persist the in-RAM record to flash. Public for callers that
    //! used Mutate() to assemble a multi-call write transaction.
    bool Write();

private:
    void ApplyDefaults();

    NonvolatileStorage_t storage_{};
    int active_slot_{-1};
};

extern Nvm* nvm_h;
