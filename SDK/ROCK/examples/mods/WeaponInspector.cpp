#include <ROCK/Collision.h>
#include <ROCK/Weapon.h>
#include <ROCK/WeaponParts.h>
#include "WeaponIdentity.h"
#include "ExampleRuntime.h"

#include <array>
#include <cstdio>

namespace
{
    const rock::api::collision::ApiV1* g_collision{};
    const rock::api::weapon::ApiV1* g_weapon{};
    const rock::api::weaponparts::ApiV1* g_weaponparts{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(1,g_collision)==rock::api::Status::Ok &&
            client.acquire(1,g_weapon)==rock::api::Status::Ok &&
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

        if (equipped.weaponGenerationKey == 0 || equipped.weaponFormId == 0) {
            rock::sdk::example::logInfo("No equipped weapon is published");
            return;
        }

        rock::api::weapon::WeaponCompositionStateV1 composition{};
        std::array<rock::api::weapon::WeaponCompositionEntryV1, 64> entries{};
        std::uint32_t entryCount{ 0 };
        const auto compositionResult =
            g_weapon->getWeaponCompositionStateV1(
                ownerToken,
                &composition);
        if (compositionResult == rock::api::Status::Ok) {
            (void)g_weapon->copyWeaponCompositionEntriesV1(
                ownerToken,
                entries.data(),
                static_cast<std::uint32_t>(entries.size()),
                &entryCount);
        }

        std::array<rock::api::weaponparts::WeaponPartPoseV1, 128> partPoses{};
        std::uint32_t partPoseCount{ 0 };
        (void)g_weaponparts->copyWeaponPartPoseSnapshotV1(
            ownerToken,
            partPoses.data(),
            static_cast<std::uint32_t>(partPoses.size()),
            &partPoseCount);

        rock::api::weapon::ScopeSightStateV1 scope{};
        const auto scopeResult = g_weapon->getScopeSightStateV1(
            ownerToken,
            &scope);

        rock::api::collision::EnvironmentV1 environment{};
        (void)g_collision->getEnvironment(ownerToken,&environment);
        char message[256]{};
        std::snprintf(
            message,
            sizeof(message),
            "Weapon=%08X generation=%llu bodies=%u compositionEntries=%u partPoses=%u scope=%s",
            equipped.weaponFormId,
            static_cast<unsigned long long>(equipped.weaponGenerationKey),
            environment.weaponBodyCount,
            entryCount,
            partPoseCount,
            scopeResult == rock::api::Status::Ok ? "published" : "unavailable");
        rock::sdk::example::logInfo(message);
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKWeaponInspector",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
