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

    std::uint64_t g_catalogSequence{ 0 };

    bool start(const std::uint64_t) noexcept
    {
        g_catalogSequence = 0;
        return true;
    }
    void stop(const std::uint64_t) noexcept {}

    void event(const std::uint64_t ownerToken, const PaperEventV1& data) noexcept
    {
        if (data.kind != PaperEventKindV1::WeaponMotionCatalogChanged &&
            data.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        PaperWeaponMotionCatalogStateV1 catalog{};
        if (PaperApi::inst->getWeaponMotionCatalogStateV1(
                ownerToken, &catalog) != PaperResultV1::Ok ||
            catalog.catalogSequence == 0 ||
            catalog.catalogSequence == g_catalogSequence) {
            return;
        }
        g_catalogSequence = catalog.catalogSequence;
        std::array<PaperWeaponMotionStageV1, 4> stages{};
        std::uint32_t copied{ 0 };
        const auto result = PaperApi::inst->copyWeaponMotionStagesV1(
            ownerToken,
            catalog.catalogSequence,
            0,
            stages.data(),
            static_cast<std::uint32_t>(stages.size()),
            &copied);
        char message[256]{};
        std::snprintf(
            message,
            sizeof(message),
            "catalog=%llu parts=%u stages=%u followers=%u copied=%u result=%u keys=%u",
            static_cast<unsigned long long>(catalog.catalogSequence),
            catalog.partCount,
            catalog.stageCount,
            catalog.followerCount,
            copied,
            static_cast<std::uint32_t>(result),
            copied == 0 ? 0 : stages[0].keyCount);
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKWeaponMotionCatalogReader",
            .requestedCapabilities =
                capability(api::PaperConsumerCapabilityV1::WeaponMotionCatalog) |
                capability(api::PaperConsumerCapabilityV1::FrameCallbacks),
            .minimumTableBytes =
                api::PAPER_PROVIDER_API_V1_WEAPON_MOTION_CATALOG_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onEvent = &event,
        };
        return value;
    }
}
