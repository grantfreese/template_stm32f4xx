// time_units.h - HAL abstractions for time units
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes

// lib includes

// os includes

// standard includes
#include <array>
#include <chrono>
#include <ratio>

namespace fw
{
using Hours = std::chrono::duration<float, std::ratio<3600>>;
using Minutes = std::chrono::duration<float, std::ratio<60>>;
using Seconds = std::chrono::duration<float>;
using Milliseconds = std::chrono::duration<float, std::milli>;
using Microseconds = std::chrono::duration<float, std::micro>;
using Nanoseconds = std::chrono::duration<float, std::nano>;

constexpr Hours operator"" _h(unsigned long long h) { return Hours(h); }

constexpr Minutes operator"" _m(unsigned long long m) { return Minutes(m); }

constexpr Seconds operator"" _s(unsigned long long s) { return Seconds(s); }

constexpr Milliseconds operator"" _ms(unsigned long long ms) { return Milliseconds(ms); }

constexpr Microseconds operator"" _us(unsigned long long us) { return Microseconds(us); }

constexpr Nanoseconds operator"" _ns(unsigned long long ns) { return Nanoseconds(ns); }
}  // namespace fw
