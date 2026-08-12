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
        if (data.kind != PaperEventKindV1::FrameComplete) {
            return;
        }
        PaperReloadCatalogStateV1 state{};
        if (PaperApi::inst->getReloadCatalogStateV1(ownerToken, &state) !=
                PaperResultV1::Ok ||
            state.catalogSequence == 0 ||
            state.catalogSequence == g_catalogSequence) {
            return;
        }
        g_catalogSequence = state.catalogSequence;

        std::array<PaperReloadEvidenceV1, 8> evidence{};
        std::uint32_t copied{ 0 };
        const auto result = PaperApi::inst->copyReloadEvidenceV1(
            ownerToken,
            state.catalogSequence,
            0,
            evidence.data(),
            static_cast<std::uint32_t>(evidence.size()),
            &copied);
        char message[320]{};
        std::snprintf(
            message,
            sizeof(message),
            "catalog=%llu weapon=%08X nodes=%u evidence=%u copied=%u result=%u family=%u flags=%llX",
            static_cast<unsigned long long>(state.catalogSequence),
            state.weaponFormId,
            state.nodeCount,
            state.evidenceCount,
            copied,
            static_cast<std::uint32_t>(result),
            static_cast<std::uint32_t>(state.classification.primaryFamily),
            static_cast<unsigned long long>(state.classification.familyFlags));
        paper::sdk::example::logInfo(message);
    }
}

namespace paper::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "PAPERSDKReloadCatalogDumper",
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
