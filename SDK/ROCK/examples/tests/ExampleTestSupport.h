#pragma once
#include "ExampleRuntime.h"
#include <cstdio>

inline bool checkExample(bool condition, const char* example, const char* check)
{
    if (!condition) std::printf("%s: %s\n", example, check);
    return condition;
}

// Per-translation-unit callback recorder; examples keep their real private
// state and callbacks, while only the external provider table is replaced.
namespace {
    inline unsigned clearCalls = 0;
    inline bool clearArgumentsValid = true;
    constexpr std::uint64_t testOwner = 42;
    inline rock::api::Status cleared(std::uint64_t owner) noexcept {
        ++clearCalls;
        clearArgumentsValid &= owner == testOwner;
        return rock::api::Status::Ok;
    }
}
