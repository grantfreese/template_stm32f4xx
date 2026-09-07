// task_stats.h - Running statistics for a FreeRTOS task loop
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <cstdint>
#include <limits>

namespace fw
{

//! @brief Tracks per-iteration duration of a periodic task for diagnostic reporting.
//!
//! Accumulates the minimum, maximum, and mean duration of each loop iteration via an
//! exponential moving average, plus a count of deadline-missed iterations.
class TaskStats
{
public:
    //! @brief Record the duration of a single loop iteration.
    //!
    //! @param duration_ms  Measured elapsed time (in milliseconds) for one loop iteration.
    //! @param target_ms    Target iteration period (in milliseconds). Iterations exceeding
    //!                     this contribute to GetDeadlineMissCount().
    void RecordTick(uint32_t duration_ms, uint32_t target_ms)
    {
        if (duration_ms < min_ms_)
        {
            min_ms_ = duration_ms;
        }
        if (duration_ms > max_ms_)
        {
            max_ms_ = duration_ms;
        }
        if (duration_ms > target_ms)
        {
            ++deadline_miss_count_;
        }

        // Exponential moving average with alpha = 1/16. Lightweight and self-smoothing;
        // no unbounded accumulator that could overflow over long uptimes.
        if (!avg_initialized_)
        {
            avg_ms_x16_ = duration_ms * 16;
            avg_initialized_ = true;
        }
        else
        {
            avg_ms_x16_ = avg_ms_x16_ - (avg_ms_x16_ / 16) + duration_ms;
        }
        ++count_;
    }

    //! @brief Mean loop duration rounded to the nearest millisecond.
    uint32_t GetAvgMs() const
    {
        if (avg_initialized_)
        {
            return (avg_ms_x16_ + 8) / 16;
        }
        return 0;
    }

    //! @brief Longest loop duration observed since the last Reset().
    uint32_t GetMaxMs() const { return max_ms_; }

    //! @brief Shortest loop duration observed since the last Reset().
    uint32_t GetMinMs() const
    {
        if (count_ == 0)
        {
            return 0;
        }
        return min_ms_;
    }

    //! @brief Total number of iterations recorded since Reset().
    uint32_t GetCount() const { return count_; }

    //! @brief Number of iterations that exceeded the target period.
    uint32_t GetDeadlineMissCount() const { return deadline_miss_count_; }

    //! @brief Clear all accumulated statistics.
    void Reset()
    {
        count_ = 0;
        min_ms_ = std::numeric_limits<uint32_t>::max();
        max_ms_ = 0;
        avg_ms_x16_ = 0;
        avg_initialized_ = false;
        deadline_miss_count_ = 0;
    }

private:
    uint32_t count_{0};
    uint32_t min_ms_{std::numeric_limits<uint32_t>::max()};
    uint32_t max_ms_{0};
    uint32_t avg_ms_x16_{0};  // moving average scaled × 16 for fixed-point math
    bool avg_initialized_{false};
    uint32_t deadline_miss_count_{0};
};

}  // namespace fw
