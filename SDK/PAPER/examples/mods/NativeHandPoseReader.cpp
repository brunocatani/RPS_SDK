#include "ExampleRuntime.h"

#include <bit>
#include <cstdio>

namespace
{
    using namespace paper::api;

    constexpr auto capability(const PaperConsumerCapabilityV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

    std::uint64_t g_lastCaptureSequence{ 0 };

    bool start(const std::uint64_t) noexcept
    {
        g_lastCaptureSequence = 0;
        return true;
    }
    void stop(const std::uint64_t) noexcept {}

    void event(const std::uint64_t ownerToken, const PaperEventV1& data) noexcept
    {
        if (data.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        PaperNativeHandPoseV1 right{};
        PaperNativeHandPoseV1 left{};
        if (!PaperApi::inst->getNativeHandPoseV1(
                ownerToken, PaperHandV1::Right, &right) ||
            !PaperApi::inst->getNativeHandPoseV1(
                ownerToken, PaperHandV1::Left, &left) ||
            right.captureSequence == g_lastCaptureSequence) {
            return;
        }
        g_lastCaptureSequence = right.captureSequence;
        char message[224]{};
        std::snprintf(
            message,
            sizeof(message),
            "capture=%llu rightFingers=%u leftFingers=%u right=(%.2f %.2f %.2f)",
            static_cast<unsigned long long>(right.captureSequence),
            std::popcount(right.fingerLocalTransformMask),
            std::popcount(left.fingerLocalTransformMask),
            right.handInWeapon.translate[0],
            right.handInWeapon.translate[1],
            right.handInWeapon.translate[2]);
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKNativeHandPoseReader",
            .requestedCapabilities =
                capability(api::PaperConsumerCapabilityV1::NativeHandPose) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes = api::PAPER_PROVIDER_API_V1_BASE_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
