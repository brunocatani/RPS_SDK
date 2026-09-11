#include "ExampleRuntime.h"
#include <cstdio>

namespace
{
    using namespace rock::provider;
    // Inert defaults. Replace with your mod's selected world reference and
    // activity predicate. All queries below run in ROCK's owner frame callback.
    std::uint32_t g_selectedReference = 0;
    bool g_activityWantsGrip = false;
    bool g_attempted = false;
    bool g_held = false;
    bool g_releasing = false;
    std::uint32_t g_heldFrame = 0;
    std::uint32_t g_worldGeneration = 0;
    std::uint64_t g_command = 0;

    bool start(std::uint64_t) noexcept
    {
        g_attempted = g_held = g_releasing = false;
        g_command = 0; g_heldFrame = g_worldGeneration = 0;
        return true;
    }

    void stop(std::uint64_t owner) noexcept
    {
        if (g_command) (void)RockProviderApi::inst->cancelInteractionCommandV1(owner, g_command);
        // The runtime unregisters this owner after stop; ROCK releases any
        // successful PA attachment owned by it. Manual grips remain independent.
        (void)start(owner);
    }

    void frame(std::uint64_t owner, const RockProviderFrameSnapshot& snapshot) noexcept
    {
        if (g_worldGeneration != snapshot.worldGeneration) {
            stop(owner);
            g_worldGeneration = snapshot.worldGeneration;
        }
        if (g_command) {
            RockProviderInteractionCommandResultV1 result{};
            if (RockProviderApi::inst->getInteractionCommandResultV1(owner, g_command, &result) != RockProviderResultV1::Ok) return;
            if (result.state == RockProviderInteractionCommandStateV1::Queued) return;
            if (result.state == RockProviderInteractionCommandStateV1::Succeeded) {
                g_held = !g_releasing;
                g_heldFrame = g_held ? result.targetFormId : 0;
            } else if (g_releasing && (result.failure == RockProviderInteractionFailureV1::HandNotHolding ||
                       result.failure == RockProviderInteractionFailureV1::HeldObjectMismatch)) {
                g_held = false;
            }
            char text[128]{};
            std::snprintf(text, sizeof(text), "PA command state=%u failure=%u frame=%08X",
                static_cast<unsigned>(result.state), static_cast<unsigned>(result.failure), result.targetFormId);
            rock::sdk::example::logInfo(text);
            g_command = 0;
        }

        if (!g_activityWantsGrip) {
            g_attempted = false;
            if (g_held) {
                RockProviderForceReleaseRequestV1 release{};
                release.hand = RockProviderHand::Right;
                release.targetFormId = g_heldFrame;
                release.flags = static_cast<std::uint32_t>(RockProviderForceReleaseFlagV1::RequireMatchingTarget);
                release.worldGeneration = snapshot.worldGeneration;
                release.skeletonGeneration = snapshot.skeletonGeneration;
                release.providerGeneration = snapshot.providerGeneration;
                g_releasing = true;
                (void)RockProviderApi::inst->requestForceReleaseV1(owner, &release, &g_command);
            }
            return;
        }
        if (!g_selectedReference || g_attempted || g_held) return;
        g_attempted = true;

        RockProviderReferenceQueryV1 query{};
        query.referenceFormId = g_selectedReference;
        query.worldGeneration = snapshot.worldGeneration;
        query.skeletonGeneration = snapshot.skeletonGeneration;
        query.providerGeneration = snapshot.providerGeneration;
        RockProviderPowerArmorTargetV1 target{};
        if (RockProviderApi::inst->queryPowerArmorTargetV1(owner, &query, &target) != RockProviderResultV1::Ok ||
            !target.frameReference.referenceFormId || !target.points[0].valid) return;

        // The right player hand requests the PA frame's left armor-hand bone.
        // Query both hands/points independently; side names describe the armor.
        RockProviderPowerArmorGrabRequestV1 grab{};
        grab.target = query;
        grab.target.referenceFormId = target.frameReference.referenceFormId;
        grab.target.referenceNativeHandle = target.frameReference.referenceNativeHandle;
        grab.hand = RockProviderHand::Right;
        grab.point = RockProviderPowerArmorPointV1::LeftArmorHand;
        g_releasing = false;
        (void)RockProviderApi::inst->requestPowerArmorGrabV1(owner, &grab, &g_command);

        RockProviderHandTargetDetailsV1 details{};
        (void)RockProviderApi::inst->getHandTargetDetailsV1(owner, RockProviderHand::Right, &details);
        // After success, details.powerArmorPoint identifies the actual grip.
        // reference.openState and activationBlocked require their validity bits;
        // neither is a general canActivate verdict.
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKPowerArmorInteraction",
            .pluginVersion = 1,
            .requestedCapabilities =
                static_cast<std::uint32_t>(provider::RockProviderConsumerCapabilityV1::FrameSnapshots) |
                static_cast<std::uint32_t>(provider::RockProviderConsumerCapabilityV1::TargetDetails) |
                static_cast<std::uint32_t>(provider::RockProviderConsumerCapabilityV1::PowerArmor) |
                static_cast<std::uint32_t>(provider::RockProviderConsumerCapabilityV1::InteractionCommands),
            .minimumTableBytes = provider::ROCK_PROVIDER_API_V1_POWER_ARMOR_TABLE_BYTES,
            .onStart = &start, .onStop = &stop, .onFrame = &frame,
        };
        return value;
    }
}
