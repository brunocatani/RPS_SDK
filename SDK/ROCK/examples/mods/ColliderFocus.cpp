#include <ROCK/Diagnostics.h>
#include <ROCK/Weapon.h>
#include <ROCK/WeaponParts.h>
#include "WeaponIdentity.h"
#include "ExampleRuntime.h"

#include <array>
#include <cstdio>

namespace
{
    const rock::api::diagnostics::ApiV1* g_diagnostics{};
    const rock::api::weapon::ApiV1* g_weapon{};
    const rock::api::weaponparts::ApiV1* g_weaponparts{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(3,g_diagnostics)==rock::api::Status::Ok &&
            client.acquire(1,g_weapon)==rock::api::Status::Ok &&
            client.acquire(1,g_weaponparts)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    // Focus replaces ROCK's normal collider overlay while its lease is alive.
    // Keep it opt-in in a general SDK package.
    bool kEnableFocus = false;
    constexpr std::uint32_t kInvalidBodyId = 0x7FFF'FFFF;

    std::uint64_t g_weaponGeneration{ 0 };
    std::uint32_t g_bodyId{ kInvalidBodyId };
    std::uint32_t g_partKind{ 0 };
    bool g_published{ false };


    void clear(const std::uint64_t ownerToken) noexcept
    {
        if (g_published) {
            (void)g_diagnostics->clearColliderVisualizationOverrideV1(
                ownerToken);
            g_published = false;
        }
    }

    bool start(const std::uint64_t) noexcept
    {
        g_weaponGeneration = 0;
        g_bodyId = kInvalidBodyId;
        g_partKind = 0;
        g_published = false;
        if (!kEnableFocus) {
            rock::sdk::example::logInfo(
                "Collider focus is opt-in because it temporarily replaces the normal overlay");
        }
        return true;
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        clear(ownerToken);
    }

    bool selectBody(std::uint64_t ownerToken,const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        const auto equipped=rock::sdk::example::weaponIdentity(ownerToken,g_weapon,snapshot);
        std::array<rock::api::weaponparts::WeaponEvidenceDetailV1,
            rock::api::weaponparts::kMaxEvidenceDetails> details{};
        std::uint32_t count{};
        (void)g_weaponparts->copyWeaponEvidenceDetailsV1(ownerToken,
            details.data(),
            static_cast<std::uint32_t>(details.size()), &count);
        for (std::uint32_t index = 0; index < count; ++index) {
            if (details[index].weaponGenerationKey == equipped.weaponGenerationKey &&
                details[index].bodyId != kInvalidBodyId) {
                g_weaponGeneration = equipped.weaponGenerationKey;
                g_bodyId = details[index].bodyId;
                g_partKind = details[index].partKind;
                return true;
            }
        }
        return false;
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        const auto equipped=rock::sdk::example::weaponIdentity(ownerToken,g_weapon,snapshot);
        if (!kEnableFocus || equipped.weaponGenerationKey == 0 ||
            !hasLifecycleFlag(
                snapshot.lifecycleFlags,
                rock::api::core::LifecycleFlag::VisualWriteAllowed)) {
            clear(ownerToken);
            return;
        }

        if (equipped.weaponGenerationKey != g_weaponGeneration) {
            clear(ownerToken);
            g_bodyId = kInvalidBodyId;
            if (!selectBody(ownerToken,snapshot)) {
                return;
            }
            char message[160]{};
            std::snprintf(
                message,
                sizeof(message),
                "Focused collider body=%08X partKind=%u",
                g_bodyId,
                g_partKind);
            rock::sdk::example::logInfo(message);
        }

        rock::api::diagnostics::ColliderVisualizationRequestV1 request{};
        request.weaponGenerationKey = g_weaponGeneration;
        request.bodyId = g_bodyId;
        request.partKind = g_partKind;
        request.leaseFrames = 2;
        request.worldGeneration = snapshot.worldGeneration;
        request.skeletonGeneration = snapshot.skeletonGeneration;
        request.providerGeneration = snapshot.providerGeneration;
        g_published =
            g_diagnostics->setColliderVisualizationOverrideV1(
                ownerToken,
                &request) == rock::api::Status::Ok;
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKColliderFocus",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
