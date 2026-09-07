// signals.h - Signals HAL
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes

// lib includes
#include "framework/error.h"

// os includes

// standard includes

namespace fw
{
class IResistance
{
public:
    virtual float GetResistance() const = 0;
};

class ITemperature
{
public:
    virtual float GetTemperature() const = 0;
};

class ICurrent
{
public:
    virtual float GetCurrent() const = 0;
};

class IVoltage
{
public:
    virtual float GetVoltage() const = 0;  // voltage mcu pin
};

class IADCChannel : public IVoltage
{
public:
    virtual int GetRawCount() const = 0;
};

class IRatio
{
public:
    /// @brief Get ratio of signal. (Either 0.0 to 1.0 or -1.0 to +1.0)
    /// @return float
    virtual float GetRatio() const = 0;
};

class SimpleVoltage : public IVoltage
{
public:
    float GetVoltage() const override { return voltage_; }
    void SetVoltage(float voltage) { voltage_ = voltage; }

private:
    float voltage_;
};

class SimpleCurrent : public ICurrent
{
public:
    float GetCurrent() const override { return current_; }
    void SetCurrent(float current) { current_ = current; }

private:
    float current_;
};

class SimpleTemperature : public ITemperature
{
public:
    float GetTemperature() const override { return temperature_; }
    void SetTemperature(float temperature) { temperature_ = temperature; }

private:
    float temperature_;
};

}  // namespace fw
