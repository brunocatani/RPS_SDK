# Shared types and results

`ROCK/Abi.h` owns the fixed shared value vocabulary. Feature-specific enums and records live in the corresponding family header. `Status::Ok` permits inspection of the output; its validity flags still determine which fields are usable. `RequestQueued` admits a command, whose eventual outcome must be read from Grab.

`Hand` is `None = 0`, `Right = 1`, `Left = 2`; do not use these numbers as UI input array indices. Samples carry frame, publication sequence and generations. Reference identity uses a form ID, optional native-handle check and lifecycle guards. Body IDs and source keys are never engine pointers.

Statuses distinguish invalid arguments/layouts, discovery incompatibility, readiness, ownership/permission failures, wrong thread, command state and revocation. Handle the status a call actually returns; declaring a status does not imply every endpoint emits it.

Transforms contain row-major rotation storage, a translation, and uniform scale. A zero-initialized rotation is not an identity matrix. Fields named `Game` use game units; `Havok` fields use Havok units. Obtain conversion scales from Collision.

<details>
<summary>Complete public declarations: ROCK/Abi.h</summary>

```cpp
#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>
#define ROCK_CALL __cdecl

// Frozen common value vocabulary. Feature records belong to their own headers.
#pragma pack(push, 8)
namespace rock::api {
    enum class Hand : std::uint32_t
    {
        None = 0,
        Right = 1,
        Left = 2,
    };

    enum class Status : std::uint32_t
    {
        Ok = 0,
        NotReady = 1,
        InvalidArgument = 2,
        InvalidSize = 3,
        UnsupportedVersion = 4,
        CapacityFull = 5,
        OwnerNotRegistered = 6,
        OwnerConflict = 7,
        PermissionDenied = 8,
        WorldNotReady = 9,
        TargetInvalid = 10,
        TargetUnavailable = 11,
        HandUnavailable = 12,
        HandBusy = 13,
        ObjectAlreadyOwned = 14,
        RequestQueued = 15,
        RequestNotFound = 16,
        WrongThread = 17,
        AlreadyCommitted = 18,
        UnknownInterface = 100,
        UnsupportedMajor = 101,
        UnsupportedMinor = 102,
        TableTooSmall = 103,
        OwnerRevoked = 104,
        InternalError = 105,
        Busy = 106,
        Ambiguous = 107,
        GenerationMismatch = 108,
    };

    struct Transform
    {
        float rotate[9]{};
        float translate[3]{};
        float scale{ 1.0f };
    };

    struct Point3
    {
        float x{ 0.0f };
        float y{ 0.0f };
        float z{ 0.0f };
    };

    enum class PowerArmorPointV1 : std::uint32_t
    {
        None = 0,
        LeftArmorHand = 1,
        RightArmorHand = 2,
    };

    struct Bounds3
    {
        Point3 min{};
        Point3 max{};
        std::uint32_t valid{ 0 };
        std::uint32_t reserved{ 0 };
    };

    using OwnerToken = std::uint64_t;
    enum class InterfaceId : std::uint32_t {
        Core=1, Hands=2, Collision=3, Grab=4, Touch=5, Weapon=6,
        WeaponParts=7, Animation=8, Input=9, References=10,
        PlayerController=11, Diagnostics=12, Configuration=13
    };
    struct ReferenceKey {
        std::uint32_t referenceFormId{};
        std::uint32_t referenceNativeHandle{};
        std::uint32_t worldGeneration{};
        std::uint32_t skeletonGeneration{};
        std::uint32_t providerGeneration{};
    };
    struct SampleV1 {
        std::uint64_t frameIndex{};
        std::uint64_t publicationSequence{};
        std::uint32_t worldGeneration{};
        std::uint32_t skeletonGeneration{};
        std::uint32_t providerGeneration{};
        std::uint32_t collisionGeneration{};
    };
    struct StreamV1 {
        std::uint64_t oldestSequence{};
        std::uint64_t latestSequence{};
        std::uint64_t nextSequence{};
        std::uint64_t lostCount{};
        std::uint32_t copiedCount{};
        std::uint32_t remainingCount{};
    };
    static_assert(sizeof(void*) == 8, "ROCK requires a 64-bit consumer");
    static_assert(sizeof(Hand) == 4 && sizeof(Status) == 4);

}

#pragma pack(pop)
```

</details>
