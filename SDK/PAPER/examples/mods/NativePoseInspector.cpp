#include "ExampleRuntime.h"

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
        if (data.kind != PaperEventKindV1::PresentationComplete) {
            return;
        }
        PaperNativePoseFrameStateV1 frame{};
        if (PaperApi::inst->getNativePoseFrameStateV1(ownerToken, &frame) !=
                PaperResultV1::Ok ||
            frame.snapshotSequence == g_snapshotSequence) {
            return;
        }
        g_snapshotSequence = frame.snapshotSequence;
        PaperNativeHandSolutionV1 right{};
        const auto handResult = PaperApi::inst->getNativeHandSolutionV1(
            ownerToken,
            PaperHandV1::Right,
            &right);
        char message[288]{};
        std::snprintf(
            message,
            sizeof(message),
            "snapshot=%llu mode=%u result=%u compatibility=%u hand=%u residual=(%.2f %.2f)",
            static_cast<unsigned long long>(frame.snapshotSequence),
            static_cast<std::uint32_t>(frame.mode),
            static_cast<std::uint32_t>(frame.applicationResult),
            static_cast<std::uint32_t>(frame.compatibilityReason),
            static_cast<std::uint32_t>(handResult),
            right.residualTranslationGameUnits,
            right.residualRotationDegrees);
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKNativePoseInspector",
            .requestedCapabilities =
                capability(api::PaperConsumerCapabilityV1::NativePosePipeline) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes =
                api::PAPER_PROVIDER_API_V1_NATIVE_POSE_PIPELINE_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
