// error.h - Error handling framework
// copyright Grant Freese 2026
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

#pragma once

// local includes

// lib includes

// os includes

// standard includes
#include <stdexcept>
#include <string>
#include <string_view>

namespace fw
{
#ifdef __cpp_exceptions

class NotImplementedException : public std::runtime_error
{
public:
    NotImplementedException(const std::string message) : std::runtime_error(message) {}
    NotImplementedException() : std::runtime_error("") {}
};

class NotSupportedException : public std::runtime_error
{
public:
    NotSupportedException(const std::string message) : std::runtime_error(message) {}
    NotSupportedException() : std::runtime_error("") {}
};

template <class T>
[[noreturn]] inline void try_throw(const T& exception)
{
    throw exception;
}

constexpr void runtime_assert(bool condition, std::string_view description)
{
    if (condition)
    {
        return;
    }

    throw std::runtime_error{std::string{"ASSERTION FAILURE: "} + std::string{description}};
}

template <typename T>
constexpr void runtime_assert(const T& expected, const T& actual, std::string_view description)
{
    if (expected == actual)
    {
        return;
    }

    throw std::runtime_error{std::string{"ASSERTION FAILURE: "} + std::string{description}
                             + " expected: " + std::to_string(expected) + ", actual: " + std::to_string(actual)};
}

#else

[[noreturn]] inline void HandleError() { while (1); }

class NotImplementedException
{
public:
    NotImplementedException(const std::string message) { (void)message; }
    NotImplementedException() {}
};

class NotSupportedException
{
public:
    NotSupportedException(const std::string message) { (void)message; }
    NotSupportedException() {}
};

template <class T>
[[noreturn]] inline void try_throw(const T&)
{
    HandleError();
}

inline void runtime_assert(bool condition, std::string_view)
{
    if (condition)
    {
        return;
    }

    HandleError();
}
template <typename T>
inline void runtime_assert(const T& expected, const T& actual, std::string_view)
{
    if (expected == actual)
    {
        return;
    }

    HandleError();
}
#endif

}  // namespace fw
