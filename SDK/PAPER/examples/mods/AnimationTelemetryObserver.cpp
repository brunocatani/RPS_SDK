#include "ExampleRuntime.h"

#include <array>
#include <cstdio>

namespace
{
    using namespace paper::api;

    constexpr auto capability(const PaperConsumerCapabilityV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

    std::uint64_t g_catalogRevision{ 0 };

    bool start(const std::uint64_t) noexcept
    {
        g_catalogRevision = 0;
        return true;
    }
    void stop(const std::uint64_t) noexcept {}

    void event(const std::uint64_t ownerToken, const PaperEventV1& data) noexcept
    {
        if (data.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        PaperReloadAnimationCatalogStateV1 catalog{};
        PaperReloadAnimationLiveStateV1 live{};
        if (PaperApi::inst->getReloadAnimationCatalogStateV1(
                ownerToken, &catalog) != PaperResultV1::Ok ||
            PaperApi::inst->getReloadAnimationLiveStateV1(
                ownerToken, &live) != PaperResultV1::Ok ||
            catalog.catalogRevision == g_catalogRevision) {
            return;
        }
        g_catalogRevision = catalog.catalogRevision;
        std::array<PaperReloadAnimationClipV1, 4> clips{};
        std::uint32_t copied{ 0 };
        const auto result = PaperApi::inst->copyReloadAnimationClipsV1(
            ownerToken,
            catalog.catalogSequence,
            0,
            clips.data(),
            static_cast<std::uint32_t>(clips.size()),
            &copied);
        char message[320]{};
        std::snprintf(
            message,
            sizeof(message),
            "catalog=%llu revision=%llu passive=%u exact=%u copied=%u result=%u live=%s fraction=%.3f",
            static_cast<unsigned long long>(catalog.catalogSequence),
            static_cast<unsigned long long>(catalog.catalogRevision),
            catalog.liveClipCount,
            catalog.exactClipCount,
            copied,
            static_cast<std::uint32_t>(result),
            live.animationName,
            live.fraction);
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKAnimationTelemetryObserver",
            .requestedCapabilities =
                capability(
                    api::PaperConsumerCapabilityV1::ReloadAnimationTelemetry) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes =
                api::PAPER_PROVIDER_API_V1_RELOAD_ANIMATION_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
