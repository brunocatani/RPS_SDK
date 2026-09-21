# PlayerController API

Interface ID **11**, version **1.0**. Include `ROCK/PlayerController.h`; link the header-only `RPS::ROCKPlayerController` target. Namespace: `rock::api::playercontroller`. All table calls return `rock::api::Status`. Follow the [shared runtime contract](../RuntimeContract.md) and [discovery rules](../DiscoveryAndCapabilities.md).

PlayerController exposes copied position, velocity, shape, support and implementation state, plus the existing guarded native jump request. Each output flag defines which measurements are valid.

The jump request uses positive finite game-unit height, up to 256, and current generation guards. It is admitted by the existing controller/runtime gates and invokes the native jump behavior. There is no public penetration check, teleport, arbitrary velocity setter or general locomotion ownership. Removed penetration query flags must not be reintroduced as if they were implemented.

## Entry points

The order below is local to this interface. Permissions never transfer between families. Exact argument types, enums, record defaults and bounds appear in the complete declarations below.

| Function | Permission | Thread | Purpose |
| --- | --- | --- | --- |
| `getPlayerControllerStateV1` | Read | Game owner | Read pointer-free controller/support state with optional bounded penetration query. |
| `requestPlayerControllerJumpV1` | Write | Game owner | Request a generation-guarded native jump after lifecycle and penetration validation. |
| `getSample` | Read | Any; no borrowed-callback reentry | Read the publication identity used to join family observations. |

## Types, flags, bounds, and signatures

These declarations are taken from the current public header. Initialize sized records and every sized array element with `{}`; check status, validity flags and copied counts before reading results.

<details>
<summary>Complete public declarations: ROCK/PlayerController.h</summary>

```cpp
#pragma once
#include "Abi.h"

#pragma pack(push, 8)
namespace rock::api::playercontroller {
    inline constexpr InterfaceId kInterfaceId = InterfaceId::PlayerController;
    inline constexpr std::uint32_t kMajor = 1;
    inline constexpr std::uint32_t kMinor = 0;
    inline constexpr float kMaxNativeJumpHeightGame = 256.0f;


    enum class PlayerControllerImplementationV1 : std::uint32_t
    {
        Unknown = 0,
        Proxy = 1,
        RigidBody = 2,
    };

    enum class PlayerSupportStateV1 : std::uint32_t
    {
        Unsupported = 0,
        Sliding = 1,
        Supported = 2,
    };

    enum class PlayerControllerStateFlagV1 : std::uint32_t
    {
        None = 0,
        Valid = 1u << 0,
        PositionValid = 1u << 1,
        VelocityValid = 1u << 2,
        ShapeValid = 1u << 3,
        SupportNormalValid = 1u << 4,
        Supported = 1u << 5,
        Sliding = 1u << 6,
        Proxy = 1u << 9,
        RigidBody = 1u << 10,
    };

    struct PlayerControllerJumpRequestV1
    {
        std::uint32_t size{
            sizeof(PlayerControllerJumpRequestV1)
        };
        std::uint32_t version{ 1 };
        float heightGameUnits{ 0.0f };
        std::uint32_t reserved0{ 0 };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t reserved[5]{};
    };

    struct PlayerControllerStateV1
    {
        std::uint32_t size{ sizeof(PlayerControllerStateV1) };
        std::uint32_t version{ 1 };
        std::uint64_t frameIndex{ 0 };
        std::uint32_t flags{ 0 };
        PlayerControllerImplementationV1 implementation{
            PlayerControllerImplementationV1::Unknown
        };
        PlayerSupportStateV1 supportState{
            PlayerSupportStateV1::Unsupported
        };
        std::uint32_t reserved0{ 0 };
        Point3 positionGame{};
        Point3 velocityGame{};
        Point3 supportNormalGame{};
        float radiusGame{ 0.0f };
        float heightGame{ 0.0f };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t reserved[4]{};
    };


    // Permission bits are local to this interface major. Discovery grants none.
    enum class PermissionV1 : std::uint32_t { Read=1, Write=2 };
    inline constexpr std::uint32_t kSupportedPermissions = 3;

    struct ApiV1 {
        static constexpr InterfaceId interfaceId=kInterfaceId;
        static constexpr std::uint32_t majorVersion=kMajor;
        static constexpr std::uint32_t minorVersion=kMinor;
        Status(ROCK_CALL* getPlayerControllerStateV1)(std::uint64_t ownerToken, PlayerControllerStateV1* outState) noexcept;
        Status(ROCK_CALL* requestPlayerControllerJumpV1)(std::uint64_t ownerToken, const PlayerControllerJumpRequestV1* request) noexcept;
        // Family-local publication identity; compare before combining readbacks.
        Status(ROCK_CALL* getSample)(OwnerToken owner, SampleV1* outSample) noexcept;
    };
}

#pragma pack(pop)
```

</details>
