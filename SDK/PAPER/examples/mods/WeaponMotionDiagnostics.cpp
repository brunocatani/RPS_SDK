#include "ExampleRuntime.h"

#include <cstdio>

namespace
{
    using namespace paper::api;

    constexpr auto capability(const PaperConsumerCapabilityV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

    std::uint64_t g_lastLearningRevision{ 0 };

    bool start(const std::uint64_t) noexcept
    {
        g_lastLearningRevision = 0;
        return true;
    }
    void stop(const std::uint64_t) noexcept {}

    void event(const std::uint64_t ownerToken, const PaperEventV1& data) noexcept
    {
        if (data.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        PaperWeaponMotionLearningStateV1 learning{};
        PaperWeaponMotionStoreStateV1 store{};
        if (PaperApi::inst->getWeaponMotionLearningStateV1(
                ownerToken, &learning) != PaperResultV1::Ok ||
            PaperApi::inst->getWeaponMotionStoreStateV1(
                ownerToken, &store) != PaperResultV1::Ok ||
            learning.learningRevision == g_lastLearningRevision) {
            return;
        }
        g_lastLearningRevision = learning.learningRevision;
        char message[288]{};
        std::snprintf(
            message,
            sizeof(message),
            "learning=%llu active=%u accepted=%llu rejected=%llu storeFlags=%08X persistent=%u pending=%u",
            static_cast<unsigned long long>(learning.learningRevision),
            learning.activeRecorderCount,
            static_cast<unsigned long long>(learning.acceptedStrokeCount),
            static_cast<unsigned long long>(learning.rejectedStrokeCount),
            store.flags,
            store.persistentRecordCount,
            store.pendingWriteCount);
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKWeaponMotionDiagnostics",
            .requestedCapabilities =
                capability(
                    api::PaperConsumerCapabilityV1::WeaponMotionDiagnostics) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes =
                api::PAPER_PROVIDER_API_V1_WEAPON_MOTION_DIAGNOSTICS_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
