#include "ExampleRuntime.h"

namespace
{
    using namespace paper::api;

    // Keep false in distributed examples. Authority is coordination only; it
    // does not submit a hand pose or an IK solution.
    bool kEnableAnimationAuthority = false;
    constexpr std::uint32_t kLeaseFrames = 2;

    constexpr auto capability(const PaperConsumerCapabilityV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

    PaperResultV1 renew(const std::uint64_t ownerToken) noexcept
    {
        PaperAuthorityRequestV1 request{};
        request.flags = static_cast<std::uint32_t>(
            PaperNativeAnimationAuthorityFlagV1::ReloadPose);
        request.leaseFrames = kLeaseFrames;
        return PaperApi::inst->setAnimationAuthorityV1(ownerToken, &request);
    }

    bool start(const std::uint64_t ownerToken) noexcept
    {
        if (!kEnableAnimationAuthority) {
            paper::sdk::example::logInfo(
                "Animation authority example is inert; enable it only around an owned animation pipeline");
            return true;
        }
        return renew(ownerToken) == PaperResultV1::Ok;
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        (void)PaperApi::inst->clearAnimationAuthorityV1(ownerToken);
    }

    void event(const std::uint64_t ownerToken, const PaperEventV1& data) noexcept
    {
        if (kEnableAnimationAuthority &&
            data.kind == PaperEventKindV1::FrameComplete) {
            (void)renew(ownerToken);
        }
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKAnimationAuthorityLease",
            .requestedCapabilities =
                capability(api::PaperConsumerCapabilityV1::AnimationAuthority) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes = api::PAPER_PROVIDER_API_V1_BASE_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
