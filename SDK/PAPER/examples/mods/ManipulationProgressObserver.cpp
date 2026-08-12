#include "ExampleRuntime.h"

#include <cstdio>

namespace
{
    using namespace paper::api;

    constexpr auto capability(const PaperConsumerCapabilityV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

    std::uint64_t g_eventSequence{ 0 };

    bool start(const std::uint64_t) noexcept
    {
        g_eventSequence = 0;
        return true;
    }
    void stop(const std::uint64_t) noexcept {}

    void event(const std::uint64_t ownerToken, const PaperEventV1& data) noexcept
    {
        if (data.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        PaperWeaponManipulationFrameStateV1 frame{};
        if (PaperApi::inst->getWeaponManipulationFrameStateV1(
                ownerToken, &frame) != PaperResultV1::Ok ||
            frame.eventSequence == g_eventSequence) {
            return;
        }
        g_eventSequence = frame.eventSequence;
        PaperWeaponManipulationHandStateV1 right{};
        PaperWeaponManipulationHandStateV1 left{};
        const auto rightResult = PaperApi::inst->getWeaponManipulationHandStateV1(
            ownerToken, PaperHandV1::Right, &right);
        const auto leftResult = PaperApi::inst->getWeaponManipulationHandStateV1(
            ownerToken, PaperHandV1::Left, &left);
        char message[288]{};
        std::snprintf(
            message,
            sizeof(message),
            "event=%llu hands=%u mapped=%u right=%u/%.3f left=%u/%.3f",
            static_cast<unsigned long long>(frame.eventSequence),
            frame.activeHandCount,
            frame.mappedHandCount,
            static_cast<std::uint32_t>(rightResult),
            right.normalizedProgress,
            static_cast<std::uint32_t>(leftResult),
            left.normalizedProgress);
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKManipulationProgressObserver",
            .requestedCapabilities =
                capability(
                    api::PaperConsumerCapabilityV1::WeaponManipulationTelemetry) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes =
                api::PAPER_PROVIDER_API_V1_WEAPON_MANIPULATION_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
