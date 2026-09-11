#define definition PowerArmorInteractionDefinition
#include "../mods/PowerArmorInteraction.cpp"
#undef definition
#include "ExampleTestSupport.h"

namespace
{
    unsigned requests = 0, releases = 0, cancellations = 0;
    bool argumentsValid = true;
    RockProviderInteractionCommandStateV1 commandState = RockProviderInteractionCommandStateV1::Queued;
    RockProviderResultV1 queryPA(std::uint64_t owner, const RockProviderReferenceQueryV1* query, RockProviderPowerArmorTargetV1* out)
    {
        argumentsValid &= owner == testOwner && query->referenceFormId == 0x1234;
        out->frameReference.referenceFormId = 0x5678;
        out->frameReference.referenceNativeHandle = 77;
        out->points[0].valid = 1;
        return RockProviderResultV1::Ok;
    }
    RockProviderResultV1 grabPA(std::uint64_t owner, const RockProviderPowerArmorGrabRequestV1* request, std::uint64_t* id)
    {
        ++requests;
        argumentsValid &= owner == testOwner && request->target.referenceFormId == 0x5678 &&
            request->target.referenceNativeHandle == 77 && request->target.worldGeneration == 10 &&
            request->hand == RockProviderHand::Right && request->point == RockProviderPowerArmorPointV1::LeftArmorHand;
        *id = 99;
        return RockProviderResultV1::RequestQueued;
    }
    RockProviderResultV1 detailsPA(std::uint64_t, RockProviderHand, RockProviderHandTargetDetailsV1*) { return RockProviderResultV1::Ok; }
    RockProviderResultV1 resultPA(std::uint64_t, std::uint64_t, RockProviderInteractionCommandResultV1* out)
    {
        out->state = commandState; out->targetFormId = 0x5678;
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
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(requests == 0, "PowerArmor", "default is inert");
    g_selectedReference = 0x1234; g_activityWantsGrip = true;
    example.onFrame(testOwner, snapshot);
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(requests == 1 && !g_held, "PowerArmor", "queued admission is not successful attachment");
    commandState = RockProviderInteractionCommandStateV1::Succeeded;
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(g_held, "PowerArmor", "successful result records linked frame");
    g_activityWantsGrip = false;
    example.onFrame(testOwner, snapshot);
    example.onFrame(testOwner, snapshot);
    ok &= checkExample(releases == 1 && !g_held, "PowerArmor", "activity end releases only the matching target");
    g_activityWantsGrip = true; commandState = RockProviderInteractionCommandStateV1::Queued;
    example.onFrame(testOwner, snapshot);
    example.onStop(testOwner); example.onStop(testOwner);
    ok &= checkExample(cancellations == 1 && argumentsValid, "PowerArmor", "stop cancels the pending command once");
    g_selectedReference = 0; g_activityWantsGrip = false; RockProviderApi::inst = nullptr;
    return ok;
}
