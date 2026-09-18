#include <ROCK/Collision.h>
#include <ROCK/Diagnostics.h>
#include "ExampleRuntime.h"

#include <array>
#include <cstdio>

namespace
{
    const rock::api::collision::ApiV1* g_collision{};
    const rock::api::diagnostics::ApiV1* g_diagnostics{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(1,g_collision)==rock::api::Status::Ok &&
            client.acquire(3,g_diagnostics)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;


    bool start(const std::uint64_t) noexcept
    {
        return true;
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        (void)g_diagnostics->clearDebugOverlayV1(ownerToken);
    }

    void appendContactLines(
        const std::uint64_t ownerToken,
        const rock::api::Hand hand,
        std::array<rock::api::diagnostics::DebugOverlayLineV1, 40>& lines,
        std::uint32_t& lineCount) noexcept
    {
        std::array<rock::api::collision::SemanticHandContactV1, 20> contacts{};
        std::uint32_t contactCount{ 0 };
        if (g_collision->copySemanticHandContactsV1(
                ownerToken,
                hand,
                0,
                contacts.data(),
                static_cast<std::uint32_t>(contacts.size()),
                &contactCount) != rock::api::Status::Ok) {
            return;
        }

        for (std::uint32_t index = 0;
             index < contactCount && lineCount < lines.size();
             ++index) {
            const auto& contact = contacts[index];
            auto& line = lines[lineCount++];
            line.startGame[0] = contact.contactPointGame.x;
            line.startGame[1] = contact.contactPointGame.y;
            line.startGame[2] = contact.contactPointGame.z;
            line.endGame[0] = contact.contactPointGame.x + contact.contactNormalGame.x * 4.0f;
            line.endGame[1] = contact.contactPointGame.y + contact.contactNormalGame.y * 4.0f;
            line.endGame[2] = contact.contactPointGame.z + contact.contactNormalGame.z * 4.0f;
            if (hand == rock::api::Hand::Right) {
                line.color[0] = 0.2f;
                line.color[1] = 1.0f;
                line.color[2] = 0.35f;
            } else {
                line.color[0] = 1.0f;
                line.color[1] = 0.55f;
                line.color[2] = 0.15f;
            }
            line.color[3] = 1.0f;
        }
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        if (!hasLifecycleFlag(
                snapshot.lifecycleFlags,
                rock::api::core::LifecycleFlag::VisualWriteAllowed)) {
            (void)g_diagnostics->clearDebugOverlayV1(ownerToken);
            return;
        }

        std::array<rock::api::diagnostics::DebugOverlayLineV1, 40> lines{};
        std::uint32_t lineCount{ 0 };
        appendContactLines(ownerToken, rock::api::Hand::Right, lines, lineCount);
        appendContactLines(ownerToken, rock::api::Hand::Left, lines, lineCount);

        std::array<rock::api::collision::PlayerColliderDescriptorV1, 96> colliders{};
        std::uint32_t colliderCount{ 0 };
        (void)g_collision->copyPlayerColliderDescriptorsV1(
            ownerToken,
            colliders.data(),
            static_cast<std::uint32_t>(colliders.size()),
            &colliderCount);

        rock::api::diagnostics::DebugOverlayTextV1 text{};
        std::snprintf(
            text.text,
            sizeof(text.text),
            "Semantic contacts: %u | player colliders: %u",
            lineCount,
            colliderCount);
        text.x = 18.0f;
        text.y = 18.0f;
        text.textSize = 1.6f;

        rock::api::diagnostics::DebugOverlayPublicationV1 publication{};
        publication.lineCount = lineCount;
        publication.textCount = 1;
        publication.lines = lines.data();
        publication.textEntries = &text;
        publication.worldGeneration = snapshot.worldGeneration;
        publication.skeletonGeneration = snapshot.skeletonGeneration;
        publication.providerGeneration = snapshot.providerGeneration;
        publication.leaseFrames = 2;
        (void)g_diagnostics->publishDebugOverlayV1(
            ownerToken,
            &publication);
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKContactVisualizer",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
