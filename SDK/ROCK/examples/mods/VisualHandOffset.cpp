#include <ROCK/Animation.h>
#include <ROCK/Hands.h>
#include "ExampleRuntime.h"

namespace
{
    const rock::api::animation::ApiV1* g_animation{};
    const rock::api::hands::ApiV1* g_hands{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(3,g_animation)==rock::api::Status::Ok &&
            client.acquire(1,g_hands)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    // This intentionally changes presentation only. Keep it disabled until a
    // mod has a clear visual-authority activation and blending policy.
    bool kEnableVisualOffset = false;

    bool g_published{ false };


    bool start(const std::uint64_t) noexcept
    {
        g_published = false;
        if (!kEnableVisualOffset) {
            rock::sdk::example::logInfo(
                "Visual-hand authority is opt-in; define activation and blending before enabling");
        }
        return true;
    }

    void clear(const std::uint64_t ownerToken) noexcept
    {
        if (g_published) {
            (void)g_animation->clearHandVisualAuthorityV1(
                ownerToken,
                rock::api::Hand::Right);
            g_published = false;
        }
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        clear(ownerToken);
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        if (!kEnableVisualOffset ||
            !hasLifecycleFlag(
                snapshot.lifecycleFlags,
                rock::api::core::LifecycleFlag::VisualWriteAllowed)) {
            clear(ownerToken);
            return;
        }

        // A control target starts from independent current tracking. Reading
        // our preceding presented output here would accumulate the offset.
        rock::api::hands::HandFrameV1 tracked{};
        if (!(g_hands->getHandFrameV1(ownerToken,
                rock::api::Hand::Right,
                &tracked) == rock::api::Status::Ok) || tracked.frameIndex != snapshot.frameIndex) {
            clear(ownerToken);
            return;
        }

        rock::api::animation::HandVisualAuthorityRequestV1 request{};
        request.hand = rock::api::Hand::Right;
        request.flags = static_cast<std::uint32_t>(
            rock::api::animation::HandVisualAuthorityFlagV1::WorldTransform);
        request.priority = 50;
        request.worldTransform = tracked.transform;
        request.worldTransform.translate[2] += 2.0f;
        request.leaseFrames = 2;
        request.worldGeneration = snapshot.worldGeneration;
        request.skeletonGeneration = snapshot.skeletonGeneration;
        request.providerGeneration = snapshot.providerGeneration;
        g_published = g_animation->setHandVisualAuthorityV1(
                          ownerToken,
                          &request) == rock::api::Status::Ok;
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKVisualHandOffset",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
