#include <ROCK/Hands.h>
#include <ROCK/Input.h>
#include "ExampleRuntime.h"

#include <cstdio>

namespace
{
    const rock::api::hands::ApiV1* g_hands{};
    const rock::api::input::ApiV1* g_input{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(1,g_hands)==rock::api::Status::Ok &&
            client.acquire(3,g_input)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    constexpr std::uint32_t kTriggerButtonId = 33;
    constexpr std::uint32_t kFaceButtonId = 7;
    constexpr std::uint32_t kLeaseFrames = 3;

    bool g_leasePublished{ false };
    std::uint32_t g_lastEffectiveFlags{ ~std::uint32_t{ 0 } };


    bool start(const std::uint64_t) noexcept
    {
        g_leasePublished = false;
        g_lastEffectiveFlags = ~std::uint32_t{ 0 };
        return true;
    }

    void clear(const std::uint64_t ownerToken) noexcept
    {
        if (!g_input) {
            return;
        }
        (void)g_input->clearHandInputSuppressionV1(
            ownerToken,
            rock::api::Hand::Right);
        (void)g_input->clearHandInputSuppressionV1(
            ownerToken,
            rock::api::Hand::Left);
        g_leasePublished = false;
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        clear(ownerToken);
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        rock::api::hands::RolesV1 roles{};
        const auto hand=g_hands->getRoles(ownerToken,&roles)==rock::api::Status::Ok &&
            roles.sample.frameIndex==snapshot.frameIndex && roles.sample.worldGeneration==snapshot.worldGeneration &&
            roles.sample.skeletonGeneration==snapshot.skeletonGeneration && roles.sample.providerGeneration==snapshot.providerGeneration ? roles.offhand : rock::api::Hand::None;
        rock::api::input::RawWandButtonStateV1 trigger{};
        rock::api::input::RawWandButtonStateV1 face{};
        const bool sampled = hand != rock::api::Hand::None &&
            (g_input->getRawWandButtonStateV1(ownerToken,
                hand,
                kTriggerButtonId,
                &trigger) == rock::api::Status::Ok) &&
            (g_input->getRawWandButtonStateV1(ownerToken,
                hand,
                kFaceButtonId,
                &face) == rock::api::Status::Ok) &&
            trigger.available != 0 && face.available != 0;
        const bool chordHeld = sampled && trigger.held != 0 && face.held != 0;

        if (chordHeld) {
            rock::api::input::HandInputSuppressionRequestV1 request{};
            request.hand = hand;
            request.flags = static_cast<std::uint32_t>(
                rock::api::input::HandInputSuppressionFlagV1::SuppressConfigModeChord);
            request.leaseFrames = kLeaseFrames;
            request.worldGeneration = snapshot.worldGeneration;
            request.skeletonGeneration = snapshot.skeletonGeneration;
            request.providerGeneration = snapshot.providerGeneration;
            g_leasePublished =
                g_input->setHandInputSuppressionV1(
                    ownerToken,
                    &request) == rock::api::Status::Ok;
        } else if (g_leasePublished) {
            clear(ownerToken);
        }

        rock::api::input::HandInputSuppressionStateV1 state{};
        if (hand != rock::api::Hand::None &&
            g_input->getHandInputSuppressionStateV1(
                ownerToken,
                hand,
                &state) == rock::api::Status::Ok &&
            state.effectiveFlags != g_lastEffectiveFlags) {
            g_lastEffectiveFlags = state.effectiveFlags;
            char message[192]{};
            std::snprintf(
                message,
                sizeof(message),
                "Input lease hand=%u caller=%08X effective=%08X remaining=%u",
                static_cast<std::uint32_t>(hand),
                state.callerFlags,
                state.effectiveFlags,
                state.callerRemainingFrames);
            rock::sdk::example::logInfo(message);
        }
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKInputChordLease",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
