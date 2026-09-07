// fir_moving_average_filter.h - Fixed-length moving-average (FIR) filter
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <array>
#include <cstddef>

namespace fw
{

//! @brief Fixed-length moving-average filter (FIR with all-ones coefficients / N).
//!
//! Until the buffer is filled, Value() returns the mean of the samples seen so far.
//! Filled() reports whether @p N samples have been pushed and the average reflects
//! the full window.
template <std::size_t N>
class FirMovingAverageFilter
{
public:
    static_assert(N > 0, "FirMovingAverageFilter requires N > 0");

    void Update(float sample)
    {
        if (filled_ < N)
        {
            samples_[filled_] = sample;
            filled_++;
        }
        else
        {
            samples_[head_] = sample;
            head_ = (head_ + 1) % N;
        }
    }

    float Value() const
    {
        if (filled_ == 0)
        {
            return 0.0f;
        }
        float sum = 0.0f;
        for (std::size_t i = 0; i < filled_; ++i)
        {
            sum += samples_[i];
        }
        return sum / static_cast<float>(filled_);
    }

    bool Filled() const { return filled_ == N; }

    void Reset()
    {
        head_ = 0;
        filled_ = 0;
    }

private:
    std::array<float, N> samples_{};
    std::size_t head_{0};
    std::size_t filled_{0};
};

}  // namespace fw
