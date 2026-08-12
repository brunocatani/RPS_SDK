#include "ExampleRuntime.h"

#include <cstdio>

namespace
{
    using namespace paper::api;

    constexpr auto capability(const PaperConsumerCapabilityV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

    std::uint64_t g_lastConfigRevision{ ~std::uint64_t{ 0 } };

    bool start(const std::uint64_t) noexcept
    {
        g_lastConfigRevision = ~std::uint64_t{ 0 };
        return true;
    }

    void stop(const std::uint64_t) noexcept {}

    void event(
        const std::uint64_t ownerToken,
        const PaperEventV1& eventData) noexcept
    {
        if (eventData.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        PaperConfigStateV1 config{};
        if (!PaperApi::inst->getConfigStateV1(ownerToken, &config) ||
            config.revision == g_lastConfigRevision) {
            return;
        }
        g_lastConfigRevision = config.revision;
        char message[256]{};
        std::snprintf(
            message,
            sizeof(message),
            "config=%llu enabled=%u status=%08X authority=%08X captureMode=%u cache=%u",
            static_cast<unsigned long long>(config.revision),
            config.enabled,
            eventData.runtime.statusFlags,
            eventData.runtime.activeAuthorityFlags,
            static_cast<std::uint32_t>(config.developmentCaptureMode),
            static_cast<std::uint32_t>(config.weaponMotionCacheAccess));
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKRuntimeMonitor",
            .requestedCapabilities =
                capability(api::PaperConsumerCapabilityV1::RuntimeState) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes = api::PAPER_PROVIDER_API_V1_BASE_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
