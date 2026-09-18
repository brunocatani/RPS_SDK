#include <ROCK/Weapon.h>
#include <ROCK/WeaponParts.h>
#include "WeaponIdentity.h"
#include "ExampleRuntime.h"

#include <algorithm>
#include <array>
#include <cstdio>

namespace
{
    const rock::api::weapon::ApiV1* g_weapon{};
    const rock::api::weaponparts::ApiV1* g_weaponparts{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(1,g_weapon)==rock::api::Status::Ok &&
            client.acquire(1,g_weaponparts)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    std::uint64_t g_lastWeaponGeneration{ ~std::uint64_t{ 0 } };


    bool start(const std::uint64_t) noexcept
    {
        g_lastWeaponGeneration = ~std::uint64_t{ 0 };
        return true;
    }

    void stop(const std::uint64_t) noexcept
    {
        g_lastWeaponGeneration = ~std::uint64_t{ 0 };
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        const auto equipped=rock::sdk::example::weaponIdentity(ownerToken,g_weapon,snapshot);
        if (equipped.weaponGenerationKey == g_lastWeaponGeneration) {
            return;
        }
        g_lastWeaponGeneration = equipped.weaponGenerationKey;

        rock::api::weapon::WeaponClassificationV1 classification{};
        const bool classified =
            g_weapon->queryEquippedWeaponClassificationV1(
                ownerToken,&classification)==rock::api::Status::Ok;

        std::array<rock::api::weaponparts::WeaponEvidenceDetailV1,
            rock::api::weaponparts::kMaxEvidenceDetails> details{};
        std::uint32_t detailCount{};
        (void)g_weaponparts->copyWeaponEvidenceDetailsV1(ownerToken,
            details.data(),
            static_cast<std::uint32_t>(details.size()), &detailCount);

        std::array<rock::api::weapon::WeaponEmitterV1,
            rock::api::weapon::kMaxEmitters> emitters{};
        std::uint32_t emitterCount{};
        (void)g_weapon->copyWeaponEmittersV1(ownerToken,
            emitters.data(),
            static_cast<std::uint32_t>(emitters.size()), &emitterCount);

        char summary[256]{};
        std::snprintf(
            summary,
            sizeof(summary),
            "Weapon=%08X generation=%llu classification=%s sizeClass=%u "
            "evidence=%u emitters=%u",
            equipped.weaponFormId,
            static_cast<unsigned long long>(equipped.weaponGenerationKey),
            classified && classification.valid != 0 ? "valid" : "unavailable",
            static_cast<std::uint32_t>(classification.sizeClass),
            detailCount,
            emitterCount);
        rock::sdk::example::logInfo(summary);

        const auto rowsToLog = (std::min)(detailCount, 4u);
        for (std::uint32_t index = 0; index < rowsToLog; ++index) {
            const auto& detail = details[index];
            char row[288]{};
            std::snprintf(
                row,
                sizeof(row),
                "Part[%u] body=%08X kind=%u action=%u OMOD=%08X attach=%08X "
                "points=%u name=%s",
                index,
                detail.bodyId,
                detail.partKind,
                detail.actionRole,
                detail.omodFormId,
                detail.attachPointFormId,
                detail.pointCount,
                detail.sourceName);
            rock::sdk::example::logInfo(row);
        }
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKWeaponCatalogDumper",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
