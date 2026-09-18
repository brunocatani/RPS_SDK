#include <ROCK/Weapon.h>
#include <ROCK/WeaponParts.h>
#include "WeaponIdentity.h"
#include "ExampleRuntime.h"

#include <array>
#include <cmath>
#include <cstdio>

namespace
{
    const rock::api::weapon::ApiV1* g_weapon{};
    const rock::api::weaponparts::ApiV1* g_weaponparts{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(1,g_weapon)==rock::api::Status::Ok &&
            client.acquire(3,g_weaponparts)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    // Keep false in distributed examples. Set true only after choosing which
    // semantic part your mod owns and replacing the demonstration motion.
    bool kEnableDemoMotion = false;
    constexpr std::uint32_t kInvalidBodyId = 0x7FFF'FFFF;
    constexpr std::uint32_t kGroupId = 1;
    constexpr std::uint32_t kPriority = 100;
    constexpr std::uint32_t kDriveLeaseFrames = 2;

    std::uint64_t g_weaponGeneration{ 0 };
    std::uint32_t g_bodyId{ kInvalidBodyId };
    bool g_targetPublished{ false };
    bool g_drivePublished{ false };


    constexpr std::uint32_t targetFlag(
        const rock::api::weaponparts::WeaponPartTargetFlagV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

    void clearDrive(const std::uint64_t ownerToken) noexcept
    {
        if (g_drivePublished) {
            (void)g_weaponparts->clearWeaponPartDriveTargetsV1(ownerToken);
            g_drivePublished = false;
        }
    }

    void clearAll(const std::uint64_t ownerToken) noexcept
    {
        clearDrive(ownerToken);
        if (g_targetPublished) {
            (void)g_weaponparts->clearWeaponPartTargetsV1(ownerToken);
            g_targetPublished = false;
        }
        g_weaponGeneration = 0;
        g_bodyId = kInvalidBodyId;
    }

    bool start(const std::uint64_t) noexcept
    {
        g_weaponGeneration = 0;
        g_bodyId = kInvalidBodyId;
        g_targetPublished = false;
        g_drivePublished = false;
        if (!kEnableDemoMotion) {
            rock::sdk::example::logInfo(
                "Weapon-part drive demo is opt-in; edit kEnableDemoMotion after selecting a part policy");
        }
        return true;
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        clearAll(ownerToken);
    }

    bool selectAndPublishTarget(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        const auto equipped=rock::sdk::example::weaponIdentity(ownerToken,g_weapon,snapshot);
        std::array<rock::api::weaponparts::WeaponPartPoseV1,
            rock::api::weaponparts::kMaxPoses> poses{};
        std::uint32_t poseCount{ 0 };
        if (g_weaponparts->copyWeaponPartPoseSnapshotV1(
                ownerToken,
                poses.data(),
                static_cast<std::uint32_t>(poses.size()),
                &poseCount) != rock::api::Status::Ok) {
            return false;
        }

        const rock::api::weaponparts::WeaponPartPoseV1* selected{ nullptr };
        for (std::uint32_t index = 0; index < poseCount; ++index) {
            if (poses[index].weaponGenerationKey == equipped.weaponGenerationKey &&
                poses[index].bodyId != kInvalidBodyId) {
                selected = &poses[index];
                break;
            }
        }
        if (!selected) {
            return false;
        }

        rock::api::weaponparts::WeaponPartTargetV1 target{};
        target.flags =
            targetFlag(rock::api::weaponparts::WeaponPartTargetFlagV1::MatchBodyId) |
            targetFlag(rock::api::weaponparts::WeaponPartTargetFlagV1::NonExclusive);
        target.grabMode = rock::api::weaponparts::WeaponPartGrabModeV1::AttachOnly;
        target.weaponGenerationKey = equipped.weaponGenerationKey;
        target.bodyId = selected->bodyId;
        target.groupId = kGroupId;
        target.priority = kPriority;

        if (g_weaponparts->setWeaponPartTargetsV1(
                ownerToken,
                &target,
                1) != rock::api::Status::Ok) {
            return false;
        }

        g_weaponGeneration = equipped.weaponGenerationKey;
        g_bodyId = selected->bodyId;
        g_targetPublished = true;
        char message[192]{};
        std::snprintf(
            message,
            sizeof(message),
            "Selected demonstration part body=%08X generation=%llu",
            g_bodyId,
            static_cast<unsigned long long>(g_weaponGeneration));
        rock::sdk::example::logInfo(message);
        return true;
    }

    bool partIsHeld(std::uint64_t ownerToken) noexcept
    {
        for (const auto hand : { rock::api::Hand::Right, rock::api::Hand::Left }) {
            rock::api::weaponparts::WeaponPartGripStateV1 grip{};
            if (g_weaponparts->getWeaponPartGripStateV1(ownerToken,hand, &grip)==rock::api::Status::Ok &&
                grip.active != 0 && grip.weaponGenerationKey == g_weaponGeneration &&
                grip.bodyId == g_bodyId) {
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
        if (!kEnableDemoMotion ||
            !hasLifecycleFlag(
                snapshot.lifecycleFlags,
                rock::api::core::LifecycleFlag::PhysicsWriteAllowed)) {
            clearDrive(ownerToken);
            return;
        }

        if (equipped.weaponGenerationKey == 0) {
            clearAll(ownerToken);
            return;
        }
        if (equipped.weaponGenerationKey != g_weaponGeneration) {
            clearAll(ownerToken);
            if (!selectAndPublishTarget(ownerToken, snapshot)) {
                return;
            }
        }
        if (!g_targetPublished || !partIsHeld(ownerToken)) {
            clearDrive(ownerToken);
            return;
        }

        std::array<rock::api::weaponparts::WeaponPartPoseV1,
            rock::api::weaponparts::kMaxPoses> poses{};
        std::uint32_t poseCount{ 0 };
        if (g_weaponparts->copyWeaponPartPoseSnapshotV1(
                ownerToken,
                poses.data(),
                static_cast<std::uint32_t>(poses.size()),
                &poseCount) != rock::api::Status::Ok) {
            clearDrive(ownerToken);
            return;
        }

        const rock::api::weaponparts::WeaponPartPoseV1* current{ nullptr };
        for (std::uint32_t index = 0; index < poseCount; ++index) {
            if (poses[index].weaponGenerationKey == g_weaponGeneration &&
                poses[index].bodyId == g_bodyId) {
                current = &poses[index];
                break;
            }
        }
        if (!current) {
            clearDrive(ownerToken);
            return;
        }

        rock::api::weaponparts::WeaponPartDriveTargetV1 drive{};
        drive.flags = targetFlag(rock::api::weaponparts::WeaponPartTargetFlagV1::MatchBodyId);
        drive.driveSpace = rock::api::weaponparts::WeaponPartDriveSpaceV1::WeaponRootLocal;
        drive.weaponGenerationKey = g_weaponGeneration;
        drive.bodyId = g_bodyId;
        drive.groupId = kGroupId;
        drive.priority = kPriority;
        drive.leaseFrames = kDriveLeaseFrames;
        drive.targetTransform = current->weaponRootLocal;
        drive.targetTransform.translate[0] +=
            std::sin(static_cast<float>(snapshot.frameIndex) * 0.03f) * 2.0f;

        g_drivePublished =
            g_weaponparts->setWeaponPartDriveTargetsV1(
                ownerToken,
                &drive,
                1) == rock::api::Status::Ok;

        if (g_drivePublished && snapshot.frameIndex % 120 == 0) {
            std::array<rock::api::weaponparts::WeaponPartDriveApplicationResultV1,
                rock::api::weaponparts::kMaxDrives> results{};
            std::uint32_t resultCount{ 0 };
            if (g_weaponparts->copyWeaponPartDriveApplicationResultsV1(
                    ownerToken,
                    results.data(),
                    static_cast<std::uint32_t>(results.size()),
                    &resultCount) == rock::api::Status::Ok &&
                resultCount != 0) {
                char message[160]{};
                std::snprintf(
                    message,
                    sizeof(message),
                    "Drive body=%08X result=%u",
                    results[0].bodyId,
                    static_cast<std::uint32_t>(results[0].result));
                rock::sdk::example::logInfo(message);
            }
        }
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKWeaponPartDriver",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
