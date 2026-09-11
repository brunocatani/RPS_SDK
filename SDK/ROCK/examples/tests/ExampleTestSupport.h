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
    inline rock::provider::RockProviderResultV1 cleared(std::uint64_t owner)
    {
        ++clearCalls;
        clearArgumentsValid &= owner == testOwner;
        return rock::provider::RockProviderResultV1::Ok;
    }
}
