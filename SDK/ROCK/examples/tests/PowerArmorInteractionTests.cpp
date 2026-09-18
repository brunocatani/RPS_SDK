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
    rock::api::grab::InteractionCommandStateV1 commandState = rock::api::grab::InteractionCommandStateV1::Queued;
    rock::api::Status queryPA(std::uint64_t owner, const rock::api::references::ReferenceQueryV1* query, rock::api::references::PowerArmorTargetV1* out) noexcept {
        argumentsValid &= owner == testOwner && query->referenceFormId == 0x1234;
        out->frameReference.referenceFormId = 0x5678;
        out->frameReference.referenceNativeHandle = 77;
        if (classificationAvailable) out->flags =
            static_cast<std::uint32_t>(rock::api::references::TargetDetailFlagV1::PowerArmorClassification) |
            static_cast<std::uint32_t>(rock::api::references::TargetDetailFlagV1::PowerArmorFrame);
        // Consumers select the point enum, not the array position.
        out->points[1].point = rock::api::PowerArmorPointV1::LeftArmorHand;
        out->points[1].valid = 1;
        return rock::api::Status::Ok;
    }
    rock::api::Status grabPA(std::uint64_t owner, const rock::api::grab::PowerArmorGrabRequestV1* request, std::uint64_t* id) noexcept {
        ++requests;
        argumentsValid &= owner == testOwner && request->target.referenceFormId == 0x5678 &&
            request->target.referenceNativeHandle == 77 && request->target.worldGeneration == 10 &&
            request->target.skeletonGeneration == 20 && request->target.providerGeneration == 30 &&
            request->hand == rock::api::Hand::Right && request->point == rock::api::PowerArmorPointV1::LeftArmorHand;
        *id = 99;
        return rock::api::Status::RequestQueued;
    }
    rock::api::Status detailsPA(std::uint64_t owner, rock::api::Hand hand, rock::api::grab::HandTargetDetailsV1* out) noexcept {
        argumentsValid &= owner == testOwner && hand == rock::api::Hand::Right;
        if (nativeHeld) {
            out->handState.phase = rock::api::grab::HandInteractionPhaseV1::Holding;
            out->handState.targetFormId = 0x5678;
            out->handState.surfaceGripMode = rock::api::grab::SurfaceGripModeV1::AnimatedArmorBone;
            out->powerArmorPoint = rock::api::PowerArmorPointV1::LeftArmorHand;
        }
        return rock::api::Status::Ok;
    }
    rock::api::Status resultPA(std::uint64_t, std::uint64_t id, rock::api::grab::InteractionCommandResultV1* out) noexcept {
        out->state = commandState; out->targetFormId = 0x5678;
        if (commandState == rock::api::grab::InteractionCommandStateV1::Succeeded) nativeHeld = id == 99;
        return rock::api::Status::Ok;
    }
    rock::api::Status releasePA(std::uint64_t owner, const rock::api::grab::ForceReleaseRequestV1* request, std::uint64_t* id) noexcept {
        ++releases;
        argumentsValid &= owner == testOwner && request->targetFormId == 0x5678 &&
            request->flags == static_cast<std::uint32_t>(rock::api::grab::ForceReleaseFlagV1::RequireMatchingTarget);
        *id = 100;
        return rock::api::Status::RequestQueued;
    }
    rock::api::Status cancelPA(std::uint64_t owner, std::uint64_t id) noexcept {
        ++cancellations; argumentsValid &= owner == testOwner && id == 99;
        commandState = rock::api::grab::InteractionCommandStateV1::Cancelled;
        return rock::api::Status::Ok;
    }
}

bool testPowerArmorInteraction()
{
    const auto& example = rock::sdk::example::PowerArmorInteractionDefinition();
    rock::api::grab::ApiV1 grabApi{};
    rock::api::references::ApiV1 referencesApi{};
    referencesApi.queryPowerArmorTargetV1 = queryPA; grabApi.requestPowerArmorGrabV1 = grabPA;
    grabApi.getHandTargetDetailsV1 = detailsPA; grabApi.getInteractionCommandResultV1 = resultPA;
    grabApi.requestForceReleaseV1 = releasePA; grabApi.cancelInteractionCommandV1 = cancelPA;
    g_grab=&grabApi; g_references=&referencesApi;
    bool ok = example.onStart(testOwner);
    rock::api::core::SnapshotV1 snapshot{};
    snapshot.worldGeneration = 10; snapshot.skeletonGeneration = 20; snapshot.providerGeneration = 30;
    snapshot.lifecycleFlags = static_cast<std::uint32_t>(rock::api::core::LifecycleFlag::PhysicsWriteAllowed);
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
    commandState = rock::api::grab::InteractionCommandStateV1::Succeeded;
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
    g_activityWantsGrip = true; commandState = rock::api::grab::InteractionCommandStateV1::Queued;
    example.onFrame(testOwner, snapshot);
    g_activityWantsGrip = false;
    example.onFrame(testOwner, snapshot);
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(cancellations == 1 && !g_command && !g_held, "PowerArmor", "activity end cancels a queued grab and consumes its terminal result");
    g_activityWantsGrip = true; commandState = rock::api::grab::InteractionCommandStateV1::Queued;
    example.onFrame(testOwner, snapshot);
    example.onStop(testOwner); example.onStop(testOwner);
    ok &= checkExample(cancellations == 2, "PowerArmor", "stop cancels the pending command once");

    for (unsigned generation = 0; generation < 3; ++generation) {
        example.onStart(testOwner);
        commandState = rock::api::grab::InteractionCommandStateV1::Queued;
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
    g_selectedReference = 0; g_activityWantsGrip = false; g_grab=nullptr; g_references=nullptr;
    return ok;
}
