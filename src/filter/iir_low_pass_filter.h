// iir_low_pass_filter.h - Single-pole IIR low-pass filter
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

namespace fw
{

//! @brief Single-pole IIR low-pass: y_new = a * x + (1 - a) * y_old.
//!
//! State is one float; one Update call is one multiply-add per axis.
//! @p alpha is the per-sample mixing coefficient in (0, 1]. For a time
//! constant of @c tau seconds at a sample period of @c Ts seconds, use
//! @c alpha = Ts / tau (small alpha = heavy smoothing). Reset() seeds the
//! filter on the first sample after reset so the initial Value() is the
//! first sample rather than a zero-vs-input transient.
class IirLowPassFilter
{
public:
    explicit IirLowPassFilter(float alpha) : alpha_{alpha} {}

    void Update(float sample)
    {
        if (seeded_)
        {
            y_ = alpha_ * sample + (1.0f - alpha_) * y_;
        }
        else
        {
            y_ = sample;
            seeded_ = true;
        }
    }

    float Value() const { return y_; }

    void Reset()
    {
        y_ = 0.0f;
        seeded_ = false;
    }

private:
    float alpha_;
    float y_{0.0f};
    bool seeded_{false};
};

}  // namespace fw
