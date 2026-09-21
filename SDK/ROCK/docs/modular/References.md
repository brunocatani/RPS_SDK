# References API

Interface ID **10**, version **1.0**. Include `ROCK/References.h`; link the header-only `RPS::ROCKReferences` target. Namespace: `rock::api::references`. All table calls return `rock::api::Status`. Follow the [shared runtime contract](../RuntimeContract.md) and [discovery rules](../DiscoveryAndCapabilities.md).

References is Read-only native reference observation. Queries use a form ID, optional native-handle check and world/skeleton/provider guards. Native furniture markers are indexed: marker zero means marker zero, not any marker.

Reference state includes native open/activation/furniture observations. PA queries distinguish the touched object, linked frame and verified wearer. Armor-point transforms are animated armor-bone origins, not sampled human palm poses. Furniture reservations alone do not prove that a PA actor is wearing the frame.

References executes neither grabs nor player movement. Pass a validated identity/anchor to Grab for PA interaction; no References DTO is embedded in a Grab request.

## Entry points

The order below is local to this interface. Permissions never transfer between families. Exact argument types, enums, record defaults and bounds appear in the complete declarations below.

| Function | Permission | Thread | Purpose |
| --- | --- | --- | --- |
| `queryReferenceInteractionV1` | Read | Game owner | Read native activation-block/open state and indexed furniture use. |
| `queryPowerArmorTargetV1` | Read | Game owner | Classify PA furniture/actor, resolve its frame, and copy animated armor-hand poses. |
| `getSample` | Read | Any; no borrowed-callback reentry | Read the publication identity used to join family observations. |

## Types, flags, bounds, and signatures

These declarations are taken from the current public header. Initialize sized records and every sized array element with `{}`; check status, validity flags and copied counts before reading results.

<details>
<summary>Complete public declarations: ROCK/References.h</summary>

```cpp
#pragma once
#include "Abi.h"

#pragma pack(push, 8)
namespace rock::api::references {
    inline constexpr InterfaceId kInterfaceId = InterfaceId::References;
    inline constexpr std::uint32_t kMajor = 1;
    inline constexpr std::uint32_t kMinor = 0;
    inline constexpr std::uint32_t kPowerArmorPointCount = 2;


    enum class NativeOpenStateV1 : std::uint32_t
    {
        NotApplicable = 0, Open = 1, Opening = 2, Closed = 3, Closing = 4,
    };

    enum class TargetDetailFlagV1 : std::uint32_t
    {
        Reference = 1u << 0, Body = 1u << 1, Anchor = 1u << 2,
        Normal = 1u << 3, MeshPart = 1u << 4,
        ActivationBlocked = 1u << 5, OpenState = 1u << 6,
        FurnitureUse = 1u << 7, PowerArmorClassification = 1u << 8,
        PowerArmorFrame = 1u << 9, PowerArmorActor = 1u << 10,
    };

    struct ReferenceQueryV1
    {
        std::uint32_t size{ sizeof(ReferenceQueryV1) };
        std::uint32_t version{ 1 };
        std::uint32_t referenceFormId{ 0 };
        std::uint32_t referenceNativeHandle{ 0 };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::int32_t furnitureMarkerIndex{ 0 };
    };

    struct PowerArmorPointPoseV1
    {
        PowerArmorPointV1 point{};
        std::uint32_t valid{ 0 };
        Transform world{};
        Transform frameLocal{};
    };

    struct ReferenceInteractionV1
    {
        std::uint32_t size{ sizeof(ReferenceInteractionV1) };
        std::uint32_t version{ 1 };
        std::uint32_t flags{ 0 };
        std::uint32_t referenceFormId{ 0 };
        std::uint32_t referenceNativeHandle{ 0 };
        std::uint32_t baseFormId{ 0 };
        std::uint32_t baseFormType{ 0 };
        std::uint32_t activationBlocked{ 0 };
        NativeOpenStateV1 openState{};
        std::uint32_t furnitureInUse{ 0 };
        std::uint32_t furnitureInUseIncludingReservations{ 0 };
        std::int32_t furnitureMarkerIndex{ 0 };
        std::uint32_t worldGeneration{ 0 };
        std::uint64_t frameIndex{ 0 };
    };

    struct PowerArmorTargetV1
    {
        std::uint32_t size{ sizeof(PowerArmorTargetV1) };
        std::uint32_t version{ 1 };
        ReferenceInteractionV1 touchedReference{};
        ReferenceInteractionV1 frameReference{};
        std::uint32_t flags{ 0 };
        std::uint32_t actorFormId{ 0 };
        PowerArmorPointPoseV1 points[2]{};
    };


    // Permission bits are local to this interface major. Discovery grants none.
    enum class PermissionV1 : std::uint32_t { Read=1 };
    inline constexpr std::uint32_t kSupportedPermissions = 1;

    struct ApiV1 {
        static constexpr InterfaceId interfaceId=kInterfaceId;
        static constexpr std::uint32_t majorVersion=kMajor;
        static constexpr std::uint32_t minorVersion=kMinor;
        Status(ROCK_CALL* queryReferenceInteractionV1)(std::uint64_t ownerToken, const ReferenceQueryV1* query, ReferenceInteractionV1* outState) noexcept;
        Status(ROCK_CALL* queryPowerArmorTargetV1)(std::uint64_t ownerToken, const ReferenceQueryV1* query, PowerArmorTargetV1* outTarget) noexcept;
        // Family-local publication identity; compare before combining readbacks.
        Status(ROCK_CALL* getSample)(OwnerToken owner, SampleV1* outSample) noexcept;
    };
}

#pragma pack(pop)
```

</details>
