#include <ROCK/Grab.h>
#include <ROCK/References.h>
#include "ExampleRuntime.h"
#include <cstdio>

namespace
{
    const rock::api::grab::ApiV1* g_grab{};
    const rock::api::references::ApiV1* g_references{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(3,g_grab)==rock::api::Status::Ok &&
            client.acquire(1,g_references)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;
    // Inert defaults. Replace with your mod's selected world reference and
    // activity predicate. All queries below run in ROCK's owner frame callback.
    std::uint32_t g_selectedReference = 0;
    bool g_activityWantsGrip = false;
    bool g_attempted = false;
    bool g_held = false;
    bool g_releasing = false;
    bool g_cancelRequested = false;
    std::uint32_t g_heldFrame = 0;
    std::uint32_t g_worldGeneration = 0;
    std::uint32_t g_skeletonGeneration = 0;
    std::uint32_t g_providerGeneration = 0;
    std::uint64_t g_command = 0;

    bool start(std::uint64_t) noexcept
    {
        g_attempted = g_held = g_releasing = g_cancelRequested = false;
        g_command = 0; g_heldFrame = g_worldGeneration = 0;
        g_skeletonGeneration = g_providerGeneration = 0;
        return true;
    }

    void stop(std::uint64_t owner) noexcept
    {
        if (g_command) (void)g_grab->cancelInteractionCommandV1(owner, g_command);
        // The runtime unregisters this owner after stop; ROCK releases any
        // successful PA attachment owned by it. Manual grips remain independent.
        (void)start(owner);
    }

    void frame(std::uint64_t owner, const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        if (g_worldGeneration != snapshot.worldGeneration ||
            g_skeletonGeneration != snapshot.skeletonGeneration ||
            g_providerGeneration != snapshot.providerGeneration) {
            stop(owner);
            g_worldGeneration = snapshot.worldGeneration;
            g_skeletonGeneration = snapshot.skeletonGeneration;
            g_providerGeneration = snapshot.providerGeneration;
        }
        const bool canWrite = hasLifecycleFlag(snapshot.lifecycleFlags, rock::api::core::LifecycleFlag::PhysicsWriteAllowed);
        if (g_command) {
            rock::api::grab::InteractionCommandResultV1 result{};
            if (g_grab->getInteractionCommandResultV1(owner, g_command, &result) != rock::api::Status::Ok) return;
            if (result.state == rock::api::grab::InteractionCommandStateV1::Queued) {
                if (!g_releasing && !g_cancelRequested && (!g_activityWantsGrip || !canWrite)) {
                    const auto cancelled = g_grab->cancelInteractionCommandV1(owner, g_command);
                    g_cancelRequested = true;
                    if (cancelled != rock::api::Status::Ok)
                        rock::sdk::example::logWarning("PA cancellation not accepted; continue polling the command");
                }
                return;
            }
            if (result.state == rock::api::grab::InteractionCommandStateV1::Succeeded) {
                g_held = !g_releasing;
                g_heldFrame = g_held ? result.targetFormId : 0;
            } else if (g_releasing && (result.failure == rock::api::grab::InteractionFailureV1::HandNotHolding ||
                       result.failure == rock::api::grab::InteractionFailureV1::HeldObjectMismatch)) {
                g_held = false;
            }
            char text[128]{};
            std::snprintf(text, sizeof(text), "PA command state=%u failure=%u frame=%08X",
                static_cast<unsigned>(result.state), static_cast<unsigned>(result.failure), result.targetFormId);
            rock::sdk::example::logInfo(text);
            g_command = 0;
            g_cancelRequested = false;
        }

        // A successful command is historical evidence. Native grip release or
        // target loss can end the attachment before this activity ends.
        if (g_held) {
            rock::api::grab::HandTargetDetailsV1 details{};
            if (g_grab->getHandTargetDetailsV1(owner, rock::api::Hand::Right, &details) != rock::api::Status::Ok) return;
            if (details.handState.phase != rock::api::grab::HandInteractionPhaseV1::Holding ||
                details.handState.targetFormId != g_heldFrame ||
                details.powerArmorPoint != rock::api::PowerArmorPointV1::LeftArmorHand ||
                details.handState.surfaceGripMode != rock::api::grab::SurfaceGripModeV1::AnimatedArmorBone) {
                g_held = false;
                g_heldFrame = 0;
            }
        }

        if (!canWrite) return;
        if (!g_activityWantsGrip) {
            g_attempted = false;
            if (g_held) {
                rock::api::grab::ForceReleaseRequestV1 release{};
                release.hand = rock::api::Hand::Right;
                release.targetFormId = g_heldFrame;
                release.flags = static_cast<std::uint32_t>(rock::api::grab::ForceReleaseFlagV1::RequireMatchingTarget);
                release.worldGeneration = snapshot.worldGeneration;
                release.skeletonGeneration = snapshot.skeletonGeneration;
                release.providerGeneration = snapshot.providerGeneration;
                g_releasing = true;
                (void)g_grab->requestForceReleaseV1(owner, &release, &g_command);
            }
            return;
        }
        if (!g_selectedReference || g_attempted || g_held) return;
        g_attempted = true;

        rock::api::references::ReferenceQueryV1 query{};
        query.referenceFormId = g_selectedReference;
        query.worldGeneration = snapshot.worldGeneration;
        query.skeletonGeneration = snapshot.skeletonGeneration;
        query.providerGeneration = snapshot.providerGeneration;
        rock::api::references::PowerArmorTargetV1 target{};
        constexpr auto requiredFlags =
            static_cast<std::uint32_t>(rock::api::references::TargetDetailFlagV1::PowerArmorClassification) |
            static_cast<std::uint32_t>(rock::api::references::TargetDetailFlagV1::PowerArmorFrame);
        if (g_references->queryPowerArmorTargetV1(owner, &query, &target) != rock::api::Status::Ok ||
            (target.flags & requiredFlags) != requiredFlags || !target.frameReference.referenceFormId) return;
        bool pointValid = false;
        for (const auto& pose : target.points)
            if (pose.point == rock::api::PowerArmorPointV1::LeftArmorHand && pose.valid) pointValid = true;
        if (!pointValid) return;

        // The right player hand requests the PA frame's left armor-hand bone.
        // Query both hands/points independently; side names describe the armor.
        rock::api::grab::PowerArmorGrabRequestV1 grab{};
        grab.target.worldGeneration=query.worldGeneration;
        grab.target.skeletonGeneration=query.skeletonGeneration;
        grab.target.providerGeneration=query.providerGeneration;
        grab.target.referenceFormId = target.frameReference.referenceFormId;
        grab.target.referenceNativeHandle = target.frameReference.referenceNativeHandle;
        grab.hand = rock::api::Hand::Right;
        grab.point = rock::api::PowerArmorPointV1::LeftArmorHand;
        g_releasing = false;
        (void)g_grab->requestPowerArmorGrabV1(owner, &grab, &g_command);

    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKPowerArmorInteraction",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start, .onStop = &stop, .onFrame = &frame,
        };
        return value;
    }
}
