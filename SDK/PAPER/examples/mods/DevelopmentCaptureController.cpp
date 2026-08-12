#include "ExampleRuntime.h"

#include <cstdio>

namespace
{
    using namespace paper::api;

    // Keep false in distributed examples. The user's PAPER.ini remains the
    // policy ceiling and must separately permit API activation.
    bool kEnableDevelopmentCapture = false;
    constexpr std::uint32_t kLeaseFrames = 120;
    constexpr std::uint32_t kRequestedScopes =
        static_cast<std::uint32_t>(
            PaperDevelopmentCaptureScopeV1::ExactAnimationHarvest) |
        static_cast<std::uint32_t>(
            PaperDevelopmentCaptureScopeV1::WeaponMotionCompilation) |
        static_cast<std::uint32_t>(
            PaperDevelopmentCaptureScopeV1::CompiledCacheRead) |
        static_cast<std::uint32_t>(
            PaperDevelopmentCaptureScopeV1::CompiledCacheWrite);

    constexpr auto capability(const PaperConsumerCapabilityV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

    std::uint64_t g_lastRenewalFrame{ 0 };
    bool g_denialReported{ false };

    PaperResultV1 renew(const std::uint64_t ownerToken) noexcept
    {
        PaperDevelopmentCaptureRequestV1 request{};
        request.scopes = kRequestedScopes;
        request.leaseFrames = kLeaseFrames;
        return PaperApi::inst->setDevelopmentCaptureV1(ownerToken, &request);
    }

    bool start(const std::uint64_t ownerToken) noexcept
    {
        g_lastRenewalFrame = 0;
        g_denialReported = false;
        if (!kEnableDevelopmentCapture) {
            paper::sdk::example::logInfo(
                "Development capture example is inert; set its explicit demo switch to opt in");
            return true;
        }
        const auto result = renew(ownerToken);
        if (result != PaperResultV1::Ok) {
            g_denialReported = true;
            char message[128]{};
            std::snprintf(
                message,
                sizeof(message),
                "Initial development capture request returned %u; policy state remains inspectable",
                static_cast<std::uint32_t>(result));
            paper::sdk::example::logWarning(message);
        }
        return true;
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        (void)PaperApi::inst->clearDevelopmentCaptureV1(ownerToken);
        g_lastRenewalFrame = 0;
    }

    void event(const std::uint64_t ownerToken, const PaperEventV1& data) noexcept
    {
        if (!kEnableDevelopmentCapture ||
            data.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        if (data.runtime.frameIndex - g_lastRenewalFrame >= kLeaseFrames / 2) {
            g_lastRenewalFrame = data.runtime.frameIndex;
            const auto result = renew(ownerToken);
            if (result != PaperResultV1::Ok && !g_denialReported) {
                g_denialReported = true;
                char message[128]{};
                std::snprintf(
                    message,
                    sizeof(message),
                    "Development capture request rejected with result %u",
                    static_cast<std::uint32_t>(result));
                paper::sdk::example::logWarning(message);
            }
        }
        if (data.runtime.frameIndex % 120 != 0) {
            return;
        }
        PaperDevelopmentCaptureStateV1 state{};
        if (PaperApi::inst->getDevelopmentCaptureStateV1(ownerToken, &state) !=
                PaperResultV1::Ok) {
            return;
        }
        char message[256]{};
        std::snprintf(
            message,
            sizeof(message),
            "active=%08X denied=%08X lease=%u clips=%u/%u motion=%u/%u pendingWrites=%u",
            state.activeScopes,
            state.deniedScopes,
            state.remainingLeaseFrames,
            state.exactClipsSampled,
            state.exactAnimationClipCount,
            state.motionPartCount,
            state.motionStageCount,
            state.pendingWriteCount);
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKDevelopmentCaptureController",
            .requestedCapabilities =
                capability(
                    api::PaperConsumerCapabilityV1::DevelopmentCaptureControl) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes =
                api::PAPER_PROVIDER_API_V1_DEVELOPMENT_CAPTURE_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
