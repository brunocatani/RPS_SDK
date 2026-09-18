#include <ROCK/Collision.h>
#include "ExampleRuntime.h"

#include <array>
#include <cstdio>

namespace
{
    const rock::api::collision::ApiV1* g_collision{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(3,g_collision)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    // Supply a live external Havok body's current ID and generation from your
    // own runtime. The SDK example deliberately has no engine-specific resolver.
    bool kEnableSensor = false;
    constexpr std::uint32_t kExternalBodyId = 0x7FFF'FFFF;
    constexpr std::uint32_t kExternalBodyGeneration = 1;
    constexpr std::uint64_t kScopeToken = 0x434F4E5441435401ull;

    std::uint64_t g_cursor{ 0 };
    bool g_registered{ false };


    void clear(const std::uint64_t ownerToken) noexcept
    {
        if (g_registered) {
            (void)g_collision->clearExternalBodiesForScopeV1(
                ownerToken,
                kScopeToken);
            g_registered = false;
        }
        g_cursor = 0;
    }

    bool start(const std::uint64_t) noexcept
    {
        g_cursor = 0;
        g_registered = false;
        if (!kEnableSensor) {
            rock::sdk::example::logInfo(
                "External contact sensor is opt-in; connect it to your live body resolver");
        }
        return true;
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        clear(ownerToken);
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        if (!kEnableSensor || kExternalBodyId == 0x7FFF'FFFF ||
            !hasLifecycleFlag(
                snapshot.lifecycleFlags,
                rock::api::core::LifecycleFlag::WorldAvailable)) {
            clear(ownerToken);
            return;
        }

        if (!g_registered) {
            rock::api::collision::ExternalBodyRegistration body{};
            body.bodyId = kExternalBodyId;
            body.worldGeneration = snapshot.worldGeneration;
            body.generation = kExternalBodyGeneration;
            body.role = rock::api::collision::ExternalBodyRole::ActorRagdollBone;
            body.contactPolicy =
                rock::api::collision::ExternalBodyContactPolicy::ReportAllSourceKinds;
            g_registered =
                g_collision->registerExternalBodiesForScopeV1(
                    ownerToken,
                    kScopeToken,
                    &body,
                    1) == rock::api::Status::Ok;
            if (!g_registered) {
                return;
            }
        }

        std::array<rock::api::collision::ExternalContactRecordV1, 16> contacts{};
        rock::api::collision::ExternalContactStreamStateV1 stream{};
        if (g_collision->copyExternalContactsSinceV1(
                ownerToken,
                kScopeToken,
                g_cursor,
                contacts.data(),
                static_cast<std::uint32_t>(contacts.size()),
                &stream) != rock::api::Status::Ok) {
            return;
        }

        for (std::uint32_t index = 0; index < stream.copiedCount; ++index) {
            const auto& contact = contacts[index];
            char message[224]{};
            std::snprintf(
                message,
                sizeof(message),
                "External contact sequence=%llu source=%u hand=%u quality=%u weight=%.3f",
                static_cast<unsigned long long>(contact.sequence),
                static_cast<std::uint32_t>(contact.sourceKind),
                static_cast<std::uint32_t>(contact.sourceHand),
                static_cast<std::uint32_t>(contact.quality),
                contact.contactPointWeightSum);
            rock::sdk::example::logInfo(message);
        }
        if (stream.copiedCount != 0) {
            g_cursor = stream.lastCopiedSequence;
        }
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKExternalContactSensor",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
