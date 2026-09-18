#include <ROCK/Collision.h>
#include <ROCK/Diagnostics.h>
#include <ROCK/Weapon.h>
#include "ExampleRuntime.h"

#include <array>

namespace
{
    const rock::api::collision::ApiV1* g_collision{};
    const rock::api::diagnostics::ApiV1* g_diagnostics{};
    const rock::api::weapon::ApiV1* g_weapon{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(1,g_collision)==rock::api::Status::Ok &&
            client.acquire(3,g_diagnostics)==rock::api::Status::Ok &&
            client.acquire(1,g_weapon)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    constexpr float kMaximumDistanceGame = 4096.0f;


    bool start(const std::uint64_t) noexcept
    {
        return true;
    }

    void clear(const std::uint64_t ownerToken) noexcept
    {
        (void)g_diagnostics->clearDebugOverlayV1(ownerToken);
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        clear(ownerToken);
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        if (!hasLifecycleFlag(
                snapshot.lifecycleFlags,
                rock::api::core::LifecycleFlag::WorldAvailable) ||
            !hasLifecycleFlag(
                snapshot.lifecycleFlags,
                rock::api::core::LifecycleFlag::VisualWriteAllowed)) {
            clear(ownerToken);
            return;
        }

        rock::api::weapon::EquippedWeaponGripStateV1 weapon{};
        if (!(g_weapon->getEquippedWeaponGripStateV1(
                ownerToken,
                &weapon) == rock::api::Status::Ok) ||
            weapon.weaponGenerationKey == 0) {
            clear(ownerToken);
            return;
        }

        rock::api::collision::WorldRaycastRequestV1 request{};
        request.startGame = weapon.muzzleOriginGame;
        request.directionGame = weapon.muzzleDirectionGame;
        request.maxDistanceGame = kMaximumDistanceGame;
        request.worldGeneration = snapshot.worldGeneration;
        request.skeletonGeneration = snapshot.skeletonGeneration;
        request.providerGeneration = snapshot.providerGeneration;

        rock::api::collision::WorldRaycastResultV1 result{};
        if (g_collision->queryWorldRaycastV1(
                ownerToken,
                &request,
                &result) != rock::api::Status::Ok) {
            clear(ownerToken);
            return;
        }

        std::array<rock::api::diagnostics::DebugOverlayLineV1, 2> lines{};
        auto& ray = lines[0];
        ray.startGame[0] = request.startGame.x;
        ray.startGame[1] = request.startGame.y;
        ray.startGame[2] = request.startGame.z;
        ray.endGame[0] = result.hitPointGame.x;
        ray.endGame[1] = result.hitPointGame.y;
        ray.endGame[2] = result.hitPointGame.z;
        ray.color[0] = result.hit != 0 ? 1.0f : 0.2f;
        ray.color[1] = result.hit != 0 ? 0.2f : 0.8f;
        ray.color[2] = 0.1f;
        ray.color[3] = 1.0f;

        std::uint32_t lineCount = 1;
        if (result.hit != 0) {
            auto& normal = lines[lineCount++];
            normal.startGame[0] = result.hitPointGame.x;
            normal.startGame[1] = result.hitPointGame.y;
            normal.startGame[2] = result.hitPointGame.z;
            normal.endGame[0] = result.hitPointGame.x + result.hitNormalGame.x * 6.0f;
            normal.endGame[1] = result.hitPointGame.y + result.hitNormalGame.y * 6.0f;
            normal.endGame[2] = result.hitPointGame.z + result.hitNormalGame.z * 6.0f;
            normal.color[0] = 0.2f;
            normal.color[1] = 0.7f;
            normal.color[2] = 1.0f;
            normal.color[3] = 1.0f;
        }

        rock::api::diagnostics::DebugOverlayPublicationV1 publication{};
        publication.lineCount = lineCount;
        publication.lines = lines.data();
        publication.worldGeneration = snapshot.worldGeneration;
        publication.skeletonGeneration = snapshot.skeletonGeneration;
        publication.providerGeneration = snapshot.providerGeneration;
        publication.leaseFrames = 2;
        (void)g_diagnostics->publishDebugOverlayV1(
            ownerToken,
            &publication);
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKMuzzleRayVisualizer",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
