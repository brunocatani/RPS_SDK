#include <ROCK/Animation.h>
#include <ROCK/Hands.h>
#include <ROCK/Weapon.h>
#include "WeaponIdentity.h"
#include "ExampleRuntime.h"

#include <cstdio>

namespace
{
    const rock::api::animation::ApiV1* g_animation{};
    const rock::api::core::ApiV1* g_core{};
    const rock::api::hands::ApiV1* g_hands{};
    const rock::api::weapon::ApiV1* g_weapon{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(1,g_animation)==rock::api::Status::Ok &&
            client.acquire(5,g_core)==rock::api::Status::Ok &&
            client.acquire(1,g_hands)==rock::api::Status::Ok &&
            client.acquire(1,g_weapon)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    std::uint64_t g_owner{};
    std::uint64_t g_phaseCallbackToken{ 0 };
    std::uint64_t g_lastWeaponGeneration{ ~std::uint64_t{ 0 } };


    void ROCK_CALL phaseCallback(
        const rock::api::core::AnimationPhaseContextV1* context,
        void*)
    {
        if (!context || context->phase != rock::api::core::AnimationPhaseV1::Complete ||
            context->frameIndex % 300 != 0) {
            return;
        }
        rock::api::animation::NativeAnimationAuthorityStateV1 authority{};
        if(g_animation->getNativeAnimationAuthorityStateV1(g_owner,&authority)!=rock::api::Status::Ok)return;
        char message[192]{};
        std::snprintf(
            message,
            sizeof(message),
            "Animation complete frame=%llu authority=%08X phaseFlags=%08X",
            static_cast<unsigned long long>(context->frameIndex),
            authority.activeFlags,
            context->flags);
        rock::sdk::example::logInfo(message);
    }

    bool start(const std::uint64_t ownerToken) noexcept
    {
        g_owner=ownerToken;
        g_phaseCallbackToken = 0;
        g_lastWeaponGeneration = ~std::uint64_t{ 0 };
        return g_core->registerAnimationPhaseCallbackV1(
                   ownerToken,
                   &phaseCallback,
                   nullptr,
                   &g_phaseCallbackToken) == rock::api::Status::Ok &&
            g_phaseCallbackToken != 0;
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        if (g_phaseCallbackToken != 0) {
            (void)g_core->unregisterAnimationPhaseCallbackV1(
                ownerToken,
                g_phaseCallbackToken);
            g_phaseCallbackToken = 0;
        }
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

        rock::api::animation::NativeAnimationAuthorityStateV1 authority{};
        const bool authorityAvailable =
            (g_animation->getNativeAnimationAuthorityStateV1(ownerToken, &authority) == rock::api::Status::Ok);
        rock::api::weapon::AuthoredGripPoseV1 authored{};
        const auto authoredResult =
            g_weapon->getSelectedAuthoredGripPoseV1(
                ownerToken,
                &authored);
        rock::api::hands::PresentedHandPoseV1 presented{};
        const auto presentedResult =
            g_hands->getPresentedHandPoseV1(
                ownerToken,
                rock::api::Hand::Right,
                &presented);

        char message[256]{};
        std::snprintf(
            message,
            sizeof(message),
            "Animation observation weaponGeneration=%llu authority=%s flags=%08X "
            "authoredPose=%u presentedPose=%u",
            static_cast<unsigned long long>(equipped.weaponGenerationKey),
            authorityAvailable ? "available" : "unavailable",
            authority.activeFlags,
            static_cast<std::uint32_t>(authoredResult),
            static_cast<std::uint32_t>(presentedResult));
        rock::sdk::example::logInfo(message);
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKAnimationObserver",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
