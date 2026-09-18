#include <ROCK/Grab.h>
#include "ExampleRuntime.h"

#include <RE/Fallout.h>

#include <cstdio>

namespace
{
    const rock::api::grab::ApiV1* g_grab{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(3,g_grab)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    // Set this to a world reference owned/resolved by your mod. The helper
    // borrows its pointer only to read formID; ROCK resolves that value later.
    std::uint32_t kTargetFormId = 0;
    constexpr std::uint64_t kCommandTimeoutFrames = 120;

    std::uint64_t g_commandId{ 0 };
    std::uint64_t g_queuedFrame{ 0 };
    std::uint32_t g_worldGeneration{ 0 };
    bool g_attempted{ false };


    bool start(const std::uint64_t) noexcept
    {
        g_commandId = 0;
        g_queuedFrame = 0;
        g_worldGeneration = 0;
        g_attempted = false;
        if (kTargetFormId == 0) {
            rock::sdk::example::logInfo(
                "Force-grab command is inert until kTargetFormId is supplied by your mod");
        }
        return true;
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        if (g_commandId != 0) {
            (void)g_grab->cancelInteractionCommandV1(
                ownerToken,
                g_commandId);
        }
        g_commandId = 0;
        g_attempted = false;
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        if (snapshot.worldGeneration != g_worldGeneration) {
            if (g_commandId != 0) {
                (void)g_grab->cancelInteractionCommandV1(
                    ownerToken,
                    g_commandId);
            }
            g_commandId = 0;
            g_attempted = false;
            g_worldGeneration = snapshot.worldGeneration;
        }

        if (kTargetFormId == 0 ||
            !hasLifecycleFlag(
                snapshot.lifecycleFlags,
                rock::api::core::LifecycleFlag::PhysicsWriteAllowed)) {
            return;
        }

        if (g_commandId == 0 && !g_attempted) {
            g_attempted = true;
            rock::api::grab::ForceGrabRequestV1 request{};
            request.hand=rock::api::Hand::Right;
            request.targetFormId=kTargetFormId;
            request.maxDistanceGame=150.0f;
            request.worldGeneration=snapshot.worldGeneration;
            request.skeletonGeneration=snapshot.skeletonGeneration;
            request.providerGeneration=snapshot.providerGeneration;
            const auto admitted=g_grab->requestForceGrabV1(ownerToken,&request,&g_commandId);
            if (admitted == rock::api::Status::RequestQueued) {
                g_queuedFrame = snapshot.frameIndex;
            } else {
                char message[96]{};
                std::snprintf(message, sizeof(message),
                    "Force-grab admission failed: result=%u", static_cast<std::uint32_t>(admitted));
                rock::sdk::example::logWarning(message);
            }
        }

        if (g_commandId == 0) {
            return;
        }

        rock::api::grab::InteractionCommandResultV1 command{};
        if (g_grab->getInteractionCommandResultV1(
                ownerToken,
                g_commandId,
                &command) != rock::api::Status::Ok) {
            return;
        }

        const bool terminal =
            command.state == rock::api::grab::InteractionCommandStateV1::Succeeded ||
            command.state == rock::api::grab::InteractionCommandStateV1::Rejected ||
            command.state == rock::api::grab::InteractionCommandStateV1::Cancelled;
        if (!terminal && snapshot.frameIndex - g_queuedFrame > kCommandTimeoutFrames) {
            (void)g_grab->cancelInteractionCommandV1(
                ownerToken,
                g_commandId);
            return;
        }
        if (!terminal) {
            return;
        }

        char message[208]{};
        std::snprintf(
            message,
            sizeof(message),
            "Command=%llu state=%u stage=%u failure=%u accepted=%llu applied=%llu",
            static_cast<unsigned long long>(command.commandId),
            static_cast<std::uint32_t>(command.state),
            static_cast<std::uint32_t>(command.stage),
            static_cast<std::uint32_t>(command.failure),
            static_cast<unsigned long long>(command.acceptedFrame),
            static_cast<unsigned long long>(command.appliedFrame));
        rock::sdk::example::logInfo(message);
        g_commandId = 0;
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKForceGrabCommand",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
