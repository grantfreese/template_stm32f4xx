// helpers.h - Misc helper functions
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes
#include "pindefs.h"

// lib includes

// os headers

// standard headers
#include <machine/endian.h>

#include <cstdint>
#include <type_traits>

// converts strongly-typed enums to underlying datatype so we don't have to cast everytime
template <typename E>
constexpr auto inline toUnder(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

// byte swap functions
#define htons(x) __htons(x)
#define ntohs(x) __ntohs(x)
#define htonl(x) __htonl(x)
#define ntohl(x) __ntohl(x)
