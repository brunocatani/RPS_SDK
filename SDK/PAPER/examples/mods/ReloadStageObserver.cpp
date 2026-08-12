#include "ExampleRuntime.h"

#include <cstdio>

namespace
{
    using namespace paper::api;

    constexpr auto capability(const PaperConsumerCapabilityV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

    std::uint32_t g_stageFlags{ ~std::uint32_t{ 0 } };

    bool start(const std::uint64_t) noexcept
    {
        g_stageFlags = ~std::uint32_t{ 0 };
        return true;
    }
    void stop(const std::uint64_t) noexcept {}

    void event(const std::uint64_t ownerToken, const PaperEventV1& data) noexcept
    {
        if (data.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        PaperReloadStageStateV1 state{};
        if (PaperApi::inst->getReloadStageStateV1(ownerToken, &state) !=
                PaperResultV1::Ok ||
            state.stageFlags == g_stageFlags) {
            return;
        }
        g_stageFlags = state.stageFlags;
        char message[224]{};
        std::snprintf(
            message,
            sizeof(message),
            "frame=%llu stages=%08X parts=%u displaced=%u fire=%u",
            static_cast<unsigned long long>(state.frameIndex),
            state.stageFlags,
            state.partCount,
            state.displacedPartCount,
            static_cast<std::uint32_t>(state.fireCorrelation));
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKReloadStageObserver",
            .requestedCapabilities =
                capability(
                    api::PaperConsumerCapabilityV1::ReloadStageIdentification) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes =
                api::PAPER_PROVIDER_API_V1_RELOAD_STAGE_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
