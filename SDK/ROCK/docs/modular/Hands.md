# Hands API

Interface ID **2**, version **1.1**. Include `ROCK/Hands.h`; link the header-only `RPS::ROCKHands` target. Namespace: `rock::api::hands`. All table calls return `rock::api::Status`. Follow the [shared runtime contract](../RuntimeContract.md) and [discovery rules](../DiscoveryAndCapabilities.md).

Hands provides raw physical-hand frames, presented frames, full presented finger poses and HMD data. Physical hand identity is Left/Right, independently of which hand fires the weapon. Read firing roles from Weapon and occupancy from Grab. Hands has Read permission only.

`getHeadPose.valid` has bit 0 for the HMD transform and bit 1 for its forward vector. A missing scene node is reported through validity, never as a public node pointer. Presented-hand and raw-hand records can describe different authorities at the same frame; use the record's flags. Finger arrays have 15 local transforms, with the published mask indicating which transforms are valid.

`getPresentedHandFrameV1` and `getPresentedHandPoseV1` return the last completed presentation with its original frame and generation stamps. Early phases can see the preceding frame; before the first final capture, data is unavailable. Use Core `AnimationPhaseV1::Presented` for same-frame final readback. `getRoles` (Hands 1.1) follows the current firing/support assignment, including handoffs; it is independent of configured dominance and never changes physical Left/Right identity.

## Entry points

The order below is local to this interface. Permissions never transfer between families. Exact argument types, enums, record defaults and bounds appear in the complete declarations below.

| Function | Permission | Thread | Purpose |
| --- | --- | --- | --- |
| `getHandFrameV1` | Read | Any; synchronized copied snapshot | Copy a root-flattened physics-authority hand frame. |
| `getPresentedHandFrameV1` | Read | Game owner | Read final presented hand-root transform. |
| `getPresentedHandPoseV1` | Read | Game owner | Read final hand root plus 15 finger locals. |
| `getSample` | Read | Any; no borrowed-callback reentry | Read the publication identity used to join family observations. |
| `getHeadPose` | Read | Any; synchronized copied snapshot | Copy the HMD transform and forward-vector validity. |
| `getRoles` | Read | Any; synchronized copied snapshot | Copy generation-stamped physical firing/support hand assignments. |


## Snapshot threading

`getHandFrameV1`, `getHeadPose`, `getRoles` may be called from task threads. They copy synchronized value snapshots; check status, validity and the payload's own frame/generation stamps. This does not relax live-query, mutation, callback-registration or teardown rules. Never assume a separately read `getSample` describes an older payload.

## Types, flags, bounds, and signatures

These declarations are taken from the current public header. Initialize sized records and every sized array element with `{}`; check status, validity flags and copied counts before reading results.

<details>
<summary>Complete public declarations: ROCK/Hands.h</summary>

```cpp
#pragma once
#include "Abi.h"

#pragma pack(push, 8)
namespace rock::api::hands {
    inline constexpr InterfaceId kInterfaceId = InterfaceId::Hands;
    inline constexpr std::uint32_t kMajor = 1;
    inline constexpr std::uint32_t kMinor = 1;
    inline constexpr std::uint32_t kFingerLocalTransformCount = 15;


    enum class HandFrameFlagV1 : std::uint32_t
    {
        None = 0,
        Valid = 1u << 0,
        Left = 1u << 1,
        HasSceneNode = 1u << 4,
        RootFlattenedAuthority = 1u << 5,
        PresentedVisual = 1u << 6,
    };

    enum class PresentedHandPoseFlagV1 : std::uint32_t
    {
        None = 0,
        Valid = 1u << 0,
        HandWorldValid = 1u << 1,
        FingerLocalsValid = 1u << 2,
        RootFlattenedReadback = 1u << 3,
    };

    struct HandFrameV1
    {
        std::uint32_t size{ sizeof(HandFrameV1) };
        std::uint32_t version{ 1 };
        Hand hand{ Hand::None };
        std::uint32_t flags{ 0 };
        Transform transform{};
        std::uint32_t reserved[7]{};
        std::uint64_t frameIndex{ 0 };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint64_t stateSequence{ 0 };
    };

    struct PresentedHandPoseV1
    {
        std::uint32_t size{ sizeof(PresentedHandPoseV1) };
        std::uint32_t version{ 1 };
        std::uint64_t frameIndex{ 0 };
        Hand hand{ Hand::None };
        std::uint32_t flags{ 0 };
        Transform handWorld{};
        std::uint16_t fingerLocalTransformMask{ 0 };
        std::uint16_t reserved0{ 0 };
        Transform fingerLocalTransforms[15]{};
        std::uint64_t presentationSequence{ 0 };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t reserved[5]{};
    };

    struct HeadPoseV1 { std::uint32_t size{sizeof(HeadPoseV1)}; std::uint32_t valid{}; SampleV1 sample{}; Transform transform{}; float forwardWorld[3]{}; };

    // Roles identify the physical hands assigned to ROCK's firing/support roles
    // in this publication. They follow firing-hand transfers; they are not the
    // player's configured dominant hand. Left/Right identities never change.
    struct RolesV1 {
        std::uint32_t size{sizeof(RolesV1)};
        std::uint32_t version{1};
        SampleV1 sample{};
        Hand primary{Hand::None};
        Hand offhand{Hand::None};
    };

    // Permission bits are local to this interface major. Discovery grants none.
    enum class PermissionV1 : std::uint32_t { Read=1 };
    inline constexpr std::uint32_t kSupportedPermissions = 1;

    struct ApiV1 {
        static constexpr InterfaceId interfaceId=kInterfaceId;
        static constexpr std::uint32_t majorVersion=kMajor;
        static constexpr std::uint32_t minorVersion=kMinor;
        // Any-thread copied control snapshot. Check Status, flags and generations.
        Status(ROCK_CALL* getHandFrameV1)(OwnerToken ownerToken, Hand hand, HandFrameV1* outFrame) noexcept;
        // Last completed presentation, with its capture frame/generations.
        // Early phases may return the preceding frame; before the first final
        // capture, readback is unavailable. Presented supplies same-frame data.
        Status(ROCK_CALL* getPresentedHandFrameV1)(OwnerToken ownerToken, Hand hand, HandFrameV1* outFrame) noexcept;
        Status(ROCK_CALL* getPresentedHandPoseV1)(std::uint64_t ownerToken, Hand hand, PresentedHandPoseV1* outPose) noexcept;
        // Family-local publication identity; compare before combining readbacks.
        Status(ROCK_CALL* getSample)(OwnerToken owner, SampleV1* outSample) noexcept;
        // Any-thread copies; payload and SampleV1 identify the same publication.
        Status(ROCK_CALL* getHeadPose)(OwnerToken, HeadPoseV1*) noexcept;
        Status(ROCK_CALL* getRoles)(OwnerToken, RolesV1*) noexcept;
    };
}

#pragma pack(pop)
```

</details>
