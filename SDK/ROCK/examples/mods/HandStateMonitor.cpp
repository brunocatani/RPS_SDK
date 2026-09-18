#include <ROCK/Grab.h>
#include "ExampleRuntime.h"

#include <RE/Fallout.h>

#include <array>
#include <cstdio>

namespace
{
    const rock::api::grab::ApiV1* g_grab{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(1,g_grab)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    std::uint64_t g_lastStateSequence{ 0 };
    std::uint64_t g_lastEventSequence{ 0 };


    void reportHand(
        const std::uint64_t ownerToken,
        const rock::api::Hand hand) noexcept
    {
        rock::api::grab::HandInteractionStateV1 state{};
        if (g_grab->getHandInteractionStateV1(
                ownerToken,
                hand,
                &state) != rock::api::Status::Ok ||
            !(state.flags & static_cast<std::uint32_t>(rock::api::grab::HandInteractionFlagV1::Valid))) {
            return;
        }

        rock::api::grab::HandTargetDetailsV1 details{};
        const auto heldResult=g_grab->getHandTargetDetailsV1(ownerToken,hand,&details);
        // Resolve the copied reference identity only within this callback.
        const auto heldForm=heldResult==rock::api::Status::Ok &&
            state.phase==rock::api::grab::HandInteractionPhaseV1::Holding &&
            (state.flags&static_cast<std::uint32_t>(rock::api::grab::HandInteractionFlagV1::LooseObject)) ? details.referenceFormId : 0;
        auto* held=heldForm?RE::TESForm::GetFormByID<RE::TESObjectREFR>(heldForm):nullptr;
        // Generic occupancy includes native carry. HeldItem resolves loose
        // references only; it is not a generic occupied/free-hand predicate.
        const auto has = [&state](rock::api::grab::HandInteractionFlagV1 flag) {
            return (state.flags & static_cast<std::uint32_t>(flag)) != 0;
        };
        char message[288]{};
        std::snprintf(
            message,
            sizeof(message),
            "%s hand phase=%u nativeCarry=%u rockGrip=%u attachOnly=%u targetKind=%u form=%08X body=%08X heldBodies=%u heldRef=%08X heldResult=%u",
            hand == rock::api::Hand::Right ? "Right" : "Left",
            static_cast<std::uint32_t>(state.phase),
            has(rock::api::grab::HandInteractionFlagV1::NativeWeaponCarry) ? 1u : 0u,
            has(rock::api::grab::HandInteractionFlagV1::RockGripActive) ? 1u : 0u,
            has(rock::api::grab::HandInteractionFlagV1::AttachOnly) ? 1u : 0u,
            static_cast<std::uint32_t>(state.targetKind),
            state.targetFormId,
            state.primaryBodyId,
            state.heldBodyCount,
            held ? held->GetFormID() : 0,
            static_cast<std::uint32_t>(heldResult));
        rock::sdk::example::logInfo(message);
    }

    void pollEvents(const std::uint64_t ownerToken) noexcept
    {
        std::array<rock::api::grab::EventV1, 16> events{};
        rock::api::StreamV1 stream{};
        const auto result = g_grab->copyEvents(
            ownerToken,
            g_lastEventSequence,
            events.data(),
            static_cast<std::uint32_t>(events.size()),
            &stream);
        if (result != rock::api::Status::Ok) {
            return;
        }

        for (std::uint32_t index = 0; index < stream.copiedCount; ++index) {
            const auto& event = events[index];
            char message[192]{};
            std::snprintf(
                message,
                sizeof(message),
                "Event sequence=%llu kind=%u hand=%u form=%08X result=%u",
                static_cast<unsigned long long>(event.sequence),
                static_cast<std::uint32_t>(event.kind),
                static_cast<std::uint32_t>(event.hand),
                event.formId,
                event.commandState);
            rock::sdk::example::logInfo(message);
        }
        if (stream.copiedCount != 0) {
            g_lastEventSequence = stream.nextSequence;
        }
    }

    bool start(const std::uint64_t) noexcept
    {
        g_lastStateSequence = 0;
        g_lastEventSequence = 0;
        return true;
    }

    void stop(const std::uint64_t) noexcept
    {
        g_lastStateSequence = 0;
        g_lastEventSequence = 0;
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        if (snapshot.stateSequence != g_lastStateSequence) {
            reportHand(ownerToken, rock::api::Hand::Right);
            reportHand(ownerToken, rock::api::Hand::Left);
            g_lastStateSequence = snapshot.stateSequence;
        }
        pollEvents(ownerToken);
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKHandStateMonitor",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
