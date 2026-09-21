# Diagnostics API

Interface ID **12**, version **1.0**. Include `ROCK/Diagnostics.h`; link the header-only `RPS::ROCKDiagnostics` target. Namespace: `rock::api::diagnostics`. All table calls return `rock::api::Status`. Follow the [shared runtime contract](../RuntimeContract.md) and [discovery rules](../DiscoveryAndCapabilities.md).

Diagnostics publishes a bounded stereo overlay and a generation-bound collider focus. Publications are copied before return; caller arrays and text storage remain caller-owned. Overlay limits apply per publisher and to the combined output, with leases retiring stale publications.

World-anchored text uses its explicit flag and anchor. Overlay publication does not authorize native physics writes. Collider focus must identify a current weapon/body generation and expires or clears independently from overlay state. Diagnostics defaults remain closed when no consumer publishes.

## Entry points

The order below is local to this interface. Permissions never transfer between families. Exact argument types, enums, record defaults and bounds appear in the complete declarations below.

| Function | Permission | Thread | Purpose |
| --- | --- | --- | --- |
| `publishDebugOverlayV1` | Write | Game owner | Copy an expiring owner line/text publication. |
| `clearDebugOverlayV1` | Write | Game owner | Remove caller overlay content. |
| `setColliderVisualizationOverrideV1` | Write | Game owner | Focus ROCK's complete collider overlay on one current weapon body. |
| `clearColliderVisualizationOverrideV1` | Write | Game owner | Restore the unchanged config-driven collider overlay. |
| `getSample` | Read | Any; no borrowed-callback reentry | Read the publication identity used to join family observations. |
| `copyEvents` | Read | Any; no borrowed-callback reentry | Read this owner/family event stream with retention and loss information. |

## Types, flags, bounds, and signatures

These declarations are taken from the current public header. Initialize sized records and every sized array element with `{}`; check status, validity flags and copied counts before reading results.

<details>
<summary>Complete public declarations: ROCK/Diagnostics.h</summary>

```cpp
#pragma once
#include "Abi.h"

#pragma pack(push, 8)
namespace rock::api::diagnostics {
    inline constexpr InterfaceId kInterfaceId = InterfaceId::Diagnostics;
    inline constexpr std::uint32_t kMajor = 1;
    inline constexpr std::uint32_t kMinor = 0;
    inline constexpr std::uint32_t kMaxOverlayPublishers = 8;
    inline constexpr std::uint32_t kMaxLinesPerPublisher = 1024;
    inline constexpr std::uint32_t kMaxTextPerPublisher = 16;
    inline constexpr std::uint32_t kMaxCombinedLines = 2048;
    inline constexpr std::uint32_t kMaxCombinedText = 64;
    inline constexpr std::uint32_t kMaxLeaseFrames = 120;
    inline constexpr std::uint32_t kEventCapacityPerOwner = 256;


    enum class DebugOverlayTextFlagV1 : std::uint32_t
    {
        None = 0,
        WorldAnchored = 1u << 0,
    };

    struct ColliderVisualizationRequestV1
    {
        std::uint32_t size{
            sizeof(ColliderVisualizationRequestV1)
        };
        std::uint32_t version{ 1 };
        std::uint64_t weaponGenerationKey{ 0 };
        std::uint32_t bodyId{ 0x7FFF'FFFF };
        std::uint32_t partKind{ 0 };
        std::uint32_t leaseFrames{ 0 };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t reserved[6]{};
    };

    struct DebugOverlayLineV1
    {
        std::uint32_t size{ sizeof(DebugOverlayLineV1) };
        std::uint32_t version{ 1 };
        float startGame[3]{};
        float endGame[3]{};
        float color[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
        std::uint32_t reserved[2]{};
    };

    struct DebugOverlayTextV1
    {
        std::uint32_t size{ sizeof(DebugOverlayTextV1) };
        std::uint32_t version{ 1 };
        std::uint32_t flags{ 0 };
        std::uint32_t reserved0{ 0 };
        char text[128]{};
        float x{ 18.0f };
        float y{ 18.0f };
        float textSize{ 2.0f };
        float color[4]{ 0.90f, 1.0f, 0.95f, 0.92f };
        float worldAnchorGame[3]{};
        std::uint32_t reserved[4]{};
    };

    struct DebugOverlayPublicationV1
    {
        std::uint32_t size{ sizeof(DebugOverlayPublicationV1) };
        std::uint32_t version{ 1 };
        std::uint32_t lineCount{ 0 };
        std::uint32_t textCount{ 0 };
        const DebugOverlayLineV1* lines{ nullptr };
        const DebugOverlayTextV1* textEntries{ nullptr };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t leaseFrames{ 0 };
        std::uint32_t reserved[4]{};
    };


    // Permission bits are local to this interface major. Discovery grants none.
    enum class PermissionV1 : std::uint32_t { Read=1, Write=2 };
    inline constexpr std::uint32_t kSupportedPermissions = 3;

    struct EventV1 {
        std::uint32_t size{sizeof(EventV1)};
        std::uint32_t kind{};
        std::uint64_t sequence{};
        std::uint64_t frameIndex{};
        std::uint64_t ownerToken{};
        std::uint32_t worldGeneration{};
        std::uint32_t skeletonGeneration{};
        std::uint32_t providerGeneration{};
        std::uint32_t authority{}; std::uint32_t reason{};
    };
    enum class EventKindV1 : std::uint32_t { AuthorityLost=1 };

    struct ApiV1 {
        static constexpr InterfaceId interfaceId=kInterfaceId;
        static constexpr std::uint32_t majorVersion=kMajor;
        static constexpr std::uint32_t minorVersion=kMinor;
        Status(ROCK_CALL* publishDebugOverlayV1)(std::uint64_t ownerToken, const DebugOverlayPublicationV1* publication) noexcept;
        Status(ROCK_CALL* clearDebugOverlayV1)(std::uint64_t ownerToken) noexcept;
        Status(ROCK_CALL* setColliderVisualizationOverrideV1)(std::uint64_t ownerToken, const ColliderVisualizationRequestV1* request) noexcept;
        Status(ROCK_CALL* clearColliderVisualizationOverrideV1)(std::uint64_t ownerToken) noexcept;
        // Family-local publication identity; compare before combining readbacks.
        Status(ROCK_CALL* getSample)(OwnerToken owner, SampleV1* outSample) noexcept;
        Status(ROCK_CALL* copyEvents)(OwnerToken, std::uint64_t afterSequence, EventV1*, std::uint32_t capacity, StreamV1*) noexcept;
    };
}

#pragma pack(pop)
```

</details>
