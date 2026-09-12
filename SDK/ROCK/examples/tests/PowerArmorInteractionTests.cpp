#define definition PowerArmorInteractionDefinition
#include "../mods/PowerArmorInteraction.cpp"
#undef definition
#include "ExampleTestSupport.h"

namespace
{
    unsigned requests = 0, releases = 0, cancellations = 0;
    bool argumentsValid = true;
    bool classificationAvailable = true;
    bool nativeHeld = false;
    RockProviderInteractionCommandStateV1 commandState = RockProviderInteractionCommandStateV1::Queued;
    RockProviderResultV1 queryPA(std::uint64_t owner, const RockProviderReferenceQueryV1* query, RockProviderPowerArmorTargetV1* out)
    {
        argumentsValid &= owner == testOwner && query->referenceFormId == 0x1234;
        out->frameReference.referenceFormId = 0x5678;
        out->frameReference.referenceNativeHandle = 77;
        if (classificationAvailable) out->flags =
            static_cast<std::uint32_t>(RockProviderTargetDetailFlagV1::PowerArmorClassification) |
            static_cast<std::uint32_t>(RockProviderTargetDetailFlagV1::PowerArmorFrame);
        // Consumers select the point enum, not the array position.
        out->points[1].point = RockProviderPowerArmorPointV1::LeftArmorHand;
        out->points[1].valid = 1;
        return RockProviderResultV1::Ok;
    }
    RockProviderResultV1 grabPA(std::uint64_t owner, const RockProviderPowerArmorGrabRequestV1* request, std::uint64_t* id)
    {
        ++requests;
        argumentsValid &= owner == testOwner && request->target.referenceFormId == 0x5678 &&
            request->target.referenceNativeHandle == 77 && request->target.worldGeneration == 10 &&
            request->target.skeletonGeneration == 20 && request->target.providerGeneration == 30 &&
            request->hand == RockProviderHand::Right && request->point == RockProviderPowerArmorPointV1::LeftArmorHand;
        *id = 99;
        return RockProviderResultV1::RequestQueued;
    }
    RockProviderResultV1 detailsPA(std::uint64_t owner, RockProviderHand hand, RockProviderHandTargetDetailsV1* out)
    {
        argumentsValid &= owner == testOwner && hand == RockProviderHand::Right;
        if (nativeHeld) {
            out->handState.phase = RockProviderHandInteractionPhaseV1::Holding;
            out->handState.targetFormId = 0x5678;
            out->handState.surfaceGripMode = RockProviderSurfaceGripModeV1::AnimatedArmorBone;
            out->powerArmorPoint = RockProviderPowerArmorPointV1::LeftArmorHand;
        }
        return RockProviderResultV1::Ok;
    }
    RockProviderResultV1 resultPA(std::uint64_t, std::uint64_t id, RockProviderInteractionCommandResultV1* out)
    {
        out->state = commandState; out->targetFormId = 0x5678;
        if (commandState == RockProviderInteractionCommandStateV1::Succeeded) nativeHeld = id == 99;
        return RockProviderResultV1::Ok;
    }
    RockProviderResultV1 releasePA(std::uint64_t owner, const RockProviderForceReleaseRequestV1* request, std::uint64_t* id)
    {
        ++releases;
        argumentsValid &= owner == testOwner && request->targetFormId == 0x5678 &&
            request->flags == static_cast<std::uint32_t>(RockProviderForceReleaseFlagV1::RequireMatchingTarget);
        *id = 100;
        return RockProviderResultV1::RequestQueued;
    }
    RockProviderResultV1 cancelPA(std::uint64_t owner, std::uint64_t id)
    {
        ++cancellations; argumentsValid &= owner == testOwner && id == 99;
        commandState = RockProviderInteractionCommandStateV1::Cancelled;
        return RockProviderResultV1::Ok;
    }
}

bool testPowerArmorInteraction()
{
    const auto& example = rock::sdk::example::PowerArmorInteractionDefinition();
    RockProviderApi api{};
    api.queryPowerArmorTargetV1 = queryPA; api.requestPowerArmorGrabV1 = grabPA;
    api.getHandTargetDetailsV1 = detailsPA; api.getInteractionCommandResultV1 = resultPA;
    api.requestForceReleaseV1 = releasePA; api.cancelInteractionCommandV1 = cancelPA;
    RockProviderApi::inst = &api;
    bool ok = example.onStart(testOwner);
    RockProviderFrameSnapshot snapshot{};
    snapshot.worldGeneration = 10; snapshot.skeletonGeneration = 20; snapshot.providerGeneration = 30;
    snapshot.lifecycleFlags = static_cast<std::uint32_t>(RockProviderLifecycleFlag::PhysicsWriteAllowed);
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(requests == 0, "PowerArmor", "default is inert");
    g_selectedReference = 0x1234; g_activityWantsGrip = true;
    classificationAvailable = false;
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(requests == 0, "PowerArmor", "classification flags are required even when fields are populated");
    g_activityWantsGrip = false;
    example.onFrame(testOwner, snapshot);
    g_activityWantsGrip = true; classificationAvailable = true;
    example.onFrame(testOwner, snapshot);
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(requests == 1 && !g_held, "PowerArmor", "queued admission is not successful attachment");
    commandState = RockProviderInteractionCommandStateV1::Succeeded;
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(g_held, "PowerArmor", "successful result records linked frame");
    nativeHeld = false;
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(!g_held && requests == 1, "PowerArmor", "native release clears the grip without automatic reacquisition");
    g_activityWantsGrip = false;
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(releases == 0, "PowerArmor", "an already released grip does not issue a release command");
    g_activityWantsGrip = true;
    example.onFrame(testOwner, snapshot);
    example.onFrame(testOwner, snapshot);
    g_activityWantsGrip = false;
    example.onFrame(testOwner, snapshot);
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(releases == 1 && !g_held, "PowerArmor", "activity end releases only the matching target");
    g_activityWantsGrip = true; commandState = RockProviderInteractionCommandStateV1::Queued;
    example.onFrame(testOwner, snapshot);
    g_activityWantsGrip = false;
    example.onFrame(testOwner, snapshot);
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(cancellations == 1 && !g_command && !g_held, "PowerArmor", "activity end cancels a queued grab and consumes its terminal result");
    g_activityWantsGrip = true; commandState = RockProviderInteractionCommandStateV1::Queued;
    example.onFrame(testOwner, snapshot);
    example.onStop(testOwner); example.onStop(testOwner);
    ok &= checkExample(cancellations == 2, "PowerArmor", "stop cancels the pending command once");

    for (unsigned generation = 0; generation < 3; ++generation) {
        example.onStart(testOwner);
        commandState = RockProviderInteractionCommandStateV1::Queued;
        example.onFrame(testOwner, snapshot);
        auto changed = snapshot;
        if (generation == 0) ++changed.worldGeneration;
        if (generation == 1) ++changed.skeletonGeneration;
        if (generation == 2) ++changed.providerGeneration;
        changed.lifecycleFlags = 0;
        const auto before = requests;
        example.onFrame(testOwner, changed);
        ok &= checkExample(!g_command && !g_held && requests == before,
            "PowerArmor", "generation loss cancels without reacquiring while writes are blocked");
    }
    ok &= checkExample(cancellations == 5 && argumentsValid, "PowerArmor", "all request identities and lifecycle cancellations match");
    g_selectedReference = 0; g_activityWantsGrip = false; RockProviderApi::inst = nullptr;
    return ok;
}
