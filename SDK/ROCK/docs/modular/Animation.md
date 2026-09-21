# Animation API

Interface ID **8**, version **1.0**. Include `ROCK/Animation.h`; link the header-only `RPS::ROCKAnimation` target. Namespace: `rock::api::animation`. All table calls return `rock::api::Status`. Follow the [shared runtime contract](../RuntimeContract.md) and [discovery rules](../DiscoveryAndCapabilities.md).

Animation owns native-animation suppression/capture authority, native-runtime publication and hand/finger visual authority. Core owns when callbacks run; an animation writer must separately acquire its Core scheduling permission.

Every publication carries generation guards and a bounded lease. A visual hand override is not a physical-hand relocation and does not authorize collision manipulation. Use Hands for raw/presented readback. The 15-bit finger mask identifies the supplied local transforms; do not publish invalid transforms or claim unsupplied fingers.

Clear hand visuals, native authority and native-runtime publication through their matching operations. Owner fault, lease expiry and provider/skeleton loss retire stale state. A native-runtime publication is a statement by the owning runtime consumer about capture status; it does not make ROCK synthesize missing native capture behavior.

## Entry points

The order below is local to this interface. Permissions never transfer between families. Exact argument types, enums, record defaults and bounds appear in the complete declarations below.

| Function | Permission | Thread | Purpose |
| --- | --- | --- | --- |
| `setNativeAnimationAuthorityV1` | Write | Game owner | Acquire/refresh selective native animation authority. |
| `clearNativeAnimationAuthorityV1` | Write | Game owner | Release caller native animation authority. |
| `getNativeAnimationAuthorityStateV1` | Read | Game owner | Read effective authority and runtime capture health. |
| `setHandVisualAuthorityV1` | Write | Game owner | Publish/refresh a hand-root and/or finger pose. |
| `clearHandVisualAuthorityV1` | Write | Game owner | Clear caller visual authority for one/both hands. |
| `publishNativeAnimationRuntimeV1` | Write | Game owner | Publish native hook/capture runtime health lease. |
| `clearNativeAnimationRuntimeV1` | Write | Game owner | Explicitly clear caller runtime-health publication. |
| `getSample` | Read | Any; no borrowed-callback reentry | Read the publication identity used to join family observations. |
| `copyEvents` | Read | Any; no borrowed-callback reentry | Read this owner/family event stream with retention and loss information. |

## Types, flags, bounds, and signatures

These declarations are taken from the current public header. Initialize sized records and every sized array element with `{}`; check status, validity flags and copied counts before reading results.

<details>
<summary>Complete public declarations: ROCK/Animation.h</summary>

```cpp
#pragma once
#include "Abi.h"

#pragma pack(push, 8)
namespace rock::api::animation {
    inline constexpr InterfaceId kInterfaceId = InterfaceId::Animation;
    inline constexpr std::uint32_t kMajor = 1;
    inline constexpr std::uint32_t kMinor = 0;
    inline constexpr std::uint32_t kMaxNativeAuthorityOwners = 64;
    inline constexpr std::uint32_t kMaxNativeAuthorityLeaseFrames = 1200;
    inline constexpr std::uint32_t kMaxHandVisualLeaseFrames = 120;
    inline constexpr std::uint32_t kMaxRuntimePublicationLeaseFrames = 120;
    inline constexpr std::uint32_t kEventCapacityPerOwner = 256;


    enum class NativeAnimationAuthorityFlagV1 : std::uint32_t
    {
        None = 0,
        Arms = 1u << 0,
        Hands = 1u << 1,
        Weapon = 1u << 2,
        ReloadPose = (1u << 0) | (1u << 1) | (1u << 2),
    };

    enum class NativeAnimationAuthorityStatusFlagV1 : std::uint32_t
    {
        None = 0,
        HookInstalled = 1u << 0,
        RuntimeEnabled = 1u << 1,
        CaptureValid = 1u << 2,
        LocalReloadTestLeaseActive = 1u << 3,
        HookInstallFailed = 1u << 4,
        ThreadMismatch = 1u << 5,
        CaptureFault = 1u << 6,
        RuntimeProviderAvailable = 1u << 7,
    };

    enum class HandVisualAuthorityFlagV1 : std::uint32_t
    {
        None = 0,
        WorldTransform = 1u << 0,
        FingerLocalTransforms = 1u << 1,
    };

    struct NativeAnimationAuthorityRequestV1
    {
        std::uint32_t size{ sizeof(NativeAnimationAuthorityRequestV1) };
        std::uint32_t version{ 1 };
        std::uint32_t flags{ 0 };
        std::uint32_t leaseFrames{ 0 };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t reserved[9]{};
    };

    struct NativeAnimationAuthorityStateV1
    {
        std::uint32_t size{ sizeof(NativeAnimationAuthorityStateV1) };
        std::uint32_t version{ 1 };
        std::uint32_t activeFlags{ 0 };
        std::uint32_t statusFlags{ 0 };
        std::uint32_t activeOwnerCount{ 0 };
        std::uint32_t capturedTransformCount{ 0 };
        std::uint64_t captureSequence{ 0 };
        std::uint32_t reserved[8]{};
    };

    struct HandVisualAuthorityRequestV1
    {
        std::uint32_t size{ sizeof(HandVisualAuthorityRequestV1) };
        std::uint32_t version{ 1 };
        Hand hand{ Hand::None };
        std::uint32_t flags{ 0 };
        std::int32_t priority{ 0 };
        std::uint16_t fingerLocalTransformMask{ 0 };
        std::uint16_t reserved0{ 0 };
        Transform worldTransform{};
        Transform fingerLocalTransforms[15]{};
        std::uint32_t leaseFrames{ 0 };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t reserved[4]{};
    };

    struct NativeAnimationRuntimePublicationV1
    {
        std::uint32_t size{ sizeof(NativeAnimationRuntimePublicationV1) };
        std::uint32_t version{ 1 };
        std::uint32_t statusFlags{ 0 };
        std::uint32_t capturedTransformCount{ 0 };
        std::uint64_t captureSequence{ 0 };
        std::uint32_t leaseFrames{ 0 };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t reserved[6]{};
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
        Status(ROCK_CALL* setNativeAnimationAuthorityV1)(std::uint64_t ownerToken, const NativeAnimationAuthorityRequestV1* request) noexcept;
        Status(ROCK_CALL* clearNativeAnimationAuthorityV1)(std::uint64_t ownerToken) noexcept;
        Status(ROCK_CALL* getNativeAnimationAuthorityStateV1)(OwnerToken ownerToken, NativeAnimationAuthorityStateV1* outState) noexcept;
        Status(ROCK_CALL* setHandVisualAuthorityV1)(std::uint64_t ownerToken, const HandVisualAuthorityRequestV1* request) noexcept;
        Status(ROCK_CALL* clearHandVisualAuthorityV1)(std::uint64_t ownerToken, Hand hand) noexcept;
        Status(ROCK_CALL* publishNativeAnimationRuntimeV1)(std::uint64_t ownerToken, const NativeAnimationRuntimePublicationV1* publication) noexcept;
        Status(ROCK_CALL* clearNativeAnimationRuntimeV1)(std::uint64_t ownerToken) noexcept;
        // Family-local publication identity; compare before combining readbacks.
        Status(ROCK_CALL* getSample)(OwnerToken owner, SampleV1* outSample) noexcept;
        Status(ROCK_CALL* copyEvents)(OwnerToken, std::uint64_t afterSequence, EventV1*, std::uint32_t capacity, StreamV1*) noexcept;
    };
}

#pragma pack(pop)
```

</details>
