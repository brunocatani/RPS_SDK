#pragma once

#include "PAPERApi.h"

#include <cstdint>
#include <string_view>

namespace paper::sdk::example
{
    struct Definition
    {
        const char* pluginName{ nullptr };
        std::uint32_t pluginVersion{ 1 };
        std::uint32_t requestedCapabilities{ 0 };
        std::uint32_t minimumTableBytes{ 0 };
        bool (*onStart)(std::uint64_t ownerToken) noexcept{ nullptr };
        void (*onStop)(std::uint64_t ownerToken) noexcept{ nullptr };
        void (*onEvent)(
            std::uint64_t ownerToken,
            const api::PaperEventV1& eventData) noexcept{ nullptr };
    };

    [[nodiscard]] const Definition& definition() noexcept;
    void logInfo(std::string_view message) noexcept;
    void logWarning(std::string_view message) noexcept;
}
