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

    std::uint64_t g_snapshotSequence{ 0 };

    bool start(const std::uint64_t) noexcept
    {
        g_snapshotSequence = 0;
        return true;
    }
    void stop(const std::uint64_t) noexcept {}

    void event(const std::uint64_t ownerToken, const PaperEventV1& data) noexcept
    {
        if (data.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        PaperReloadFrameStateV1 state{};
        if (PaperApi::inst->getReloadFrameStateV1(ownerToken, &state) !=
                PaperResultV1::Ok ||
            state.snapshotSequence == 0 ||
            state.snapshotSequence == g_snapshotSequence) {
            return;
        }
        g_snapshotSequence = state.snapshotSequence;
        if (state.frameIndex % 120 != 0) {
            return;
        }
        std::array<PaperReloadNodeObservationV1, 16> observations{};
        std::uint32_t copied{ 0 };
        const auto result = PaperApi::inst->copyReloadNodeObservationsV1(
            ownerToken,
            state.snapshotSequence,
            0,
            observations.data(),
            static_cast<std::uint32_t>(observations.size()),
            &copied);
        char message[224]{};
        std::snprintf(
            message,
            sizeof(message),
            "snapshot=%llu native=%u postRock=%u copied=%u result=%u",
            static_cast<unsigned long long>(state.snapshotSequence),
            state.nativeGraphOutputCount,
            state.postRockCount,
            copied,
            static_cast<std::uint32_t>(result));
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKReloadFrameObserver",
            .requestedCapabilities =
                capability(api::PaperConsumerCapabilityV1::ReloadObservations) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes =
                api::PAPER_PROVIDER_API_V1_RELOAD_OBSERVATION_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
