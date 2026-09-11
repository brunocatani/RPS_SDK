#include "ExampleRuntime.h"

#include <RE/Fallout.h>

#include <cstdio>

namespace
{
    using namespace rock::provider;

    // Set this to a world reference owned/resolved by your mod. The helper
    // borrows its pointer only to read formID; ROCK resolves that value later.
    std::uint32_t kTargetFormId = 0;
    constexpr std::uint64_t kCommandTimeoutFrames = 120;

    std::uint64_t g_commandId{ 0 };
    std::uint64_t g_queuedFrame{ 0 };
    std::uint32_t g_worldGeneration{ 0 };
    bool g_attempted{ false };

    constexpr std::uint32_t capability(
        const RockProviderConsumerCapabilityV1 value) noexcept
    {
        return static_cast<std::uint32_t>(value);
    }

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
            (void)RockProviderApi::inst->cancelInteractionCommandV1(
                ownerToken,
                g_commandId);
        }
        g_commandId = 0;
        g_attempted = false;
    }

    void frame(
        const std::uint64_t ownerToken,
        const RockProviderFrameSnapshot& snapshot) noexcept
    {
        if (snapshot.worldGeneration != g_worldGeneration) {
            if (g_commandId != 0) {
                (void)RockProviderApi::inst->cancelInteractionCommandV1(
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
                RockProviderLifecycleFlag::PhysicsWriteAllowed)) {
            return;
        }

        if (g_commandId == 0 && !g_attempted) {
            g_attempted = true;
            RockHandItems<RE::TESObjectREFR> hands{
                ownerToken, &RE::TESForm::GetFormByID<RE::TESObjectREFR> };
            auto* target = RE::TESForm::GetFormByID<RE::TESObjectREFR>(kTargetFormId);
            const auto grab = hands.RequestGrabItem(false, target, 150.0f);
            g_commandId = grab.commandId;
            if (grab.result == RockProviderResultV1::RequestQueued) {
                g_queuedFrame = snapshot.frameIndex;
            } else {
                char message[96]{};
                std::snprintf(message, sizeof(message),
                    "Force-grab admission failed: result=%u", static_cast<std::uint32_t>(grab.result));
                rock::sdk::example::logWarning(message);
            }
        }

        if (g_commandId == 0) {
            return;
        }

        RockProviderInteractionCommandResultV1 command{};
        if (RockProviderApi::inst->getInteractionCommandResultV1(
                ownerToken,
                g_commandId,
                &command) != RockProviderResultV1::Ok) {
            return;
        }

        const bool terminal =
            command.state == RockProviderInteractionCommandStateV1::Succeeded ||
            command.state == RockProviderInteractionCommandStateV1::Rejected ||
            command.state == RockProviderInteractionCommandStateV1::Cancelled;
        if (!terminal && snapshot.frameIndex - g_queuedFrame > kCommandTimeoutFrames) {
            (void)RockProviderApi::inst->cancelInteractionCommandV1(
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
            .requestedCapabilities =
                capability(provider::RockProviderConsumerCapabilityV1::FrameSnapshots) |
                capability(provider::RockProviderConsumerCapabilityV1::InteractionCommands),
            .minimumTableBytes =
                provider::ROCK_PROVIDER_API_V1_COMMAND_CANCELLATION_TABLE_BYTES,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
