# Weapon 1.1: inventory equip and retained switching

Weapon 1.1 adds four functions after the unchanged Weapon 1.0 table. Include
`ROCK/WeaponV1_1.h`, use `rock::api::weapon::v1_1::Api`, and link
`RPS::ROCKWeapon`. `Client::acquire(3, table)` negotiates exact major 1,
minimum minor 1, the complete new table size, and Weapon Read + Write.
`ROCK/Weapon.h` remains the released 1.0 header. Existing consumers need no
rebuild or migration. Core and the other providers keep their contracts. This extension is implemented in current source; a downloaded ROCK build must still pass minor/table-size negotiation. Virtual Holsters or another consumer must adopt these calls to use them.

## Behavior

Call only after the consumer accepts an explicit draw gesture. `Hand::Right`
and `Hand::Left` select physical hands regardless of the configured dominant
hand. ROCK rejects an occupied destination. If the equipped weapon is carried
only by the other hand, ROCK converts that exact outgoing weapon to a retained
loose grab in its current hand using the existing Toggle Drop release controls,
then equips the incoming inventory instance in the requested hand. This retention
is an explicit contract of this new request; it does not change the consumer
setting or the existing trigger-equip path. A sheathed, uncarried weapon may be
replaced normally. This is one equipped firearm plus a physically held weapon.

Left firing requires ROCK's existing FRIK pose, node-ownership and parent-hand
capabilities, and the matching authored grip readiness. Failure never chooses
the opposite hand. The consumer must not run its own native equip concurrently.

### The retained switch in both directions

| Draw gesture | Incoming equipped weapon | Previous carried weapon |
| --- | --- | --- |
| Right-hand holster draw | Selected 10mm equips into the right hand. | The FAL carried by the left hand becomes a retained loose grab in that same hand. |
| Left-hand holster draw | Selected 10mm equips into the left hand. | The FAL carried by the right hand becomes a retained loose grab in that same hand. |

The retained FAL uses the existing double-drop / Toggle Drop sequence: the original gesture does not immediately drop it; a fresh grab-and-release gesture releases it. The incoming pistol becomes the equipped firing weapon. This operation does not create two simultaneously equipped firearms.

## Entry points

| Function | Permission | Meaning |
| --- | --- | --- |
| `captureInventoryWeapon` | Read | Capture an exact native inventory stack by base form and stack index. Reject equipped stacks and throwable weapons. |
| `requestInventoryEquip` | Write | Submit the captured stack and destination hand; return `RequestQueued` plus a command ID on admission. |
| `getInventoryEquipResult` | Read | Read owner-scoped progress and terminal outcome. |
| `cancelInventoryEquip` | Write | Cancel before native commitment; return `AlreadyCommitted` after outgoing removal or native equip. |

All additions require the ROCK frame thread: call in registered Core frame or
animation-phase callbacks, outside synchronous Grab callbacks. Capture and request
in the same game frame. Resolve the native stack from the consumer's existing
exact item selection; a base form alone does not distinguish modified copies.
Never save or dereference the opaque stack/instance keys. ROCK validates the
capture and retains the exact stack while the switch is pending; stack-index
changes after removing another item cannot select a different copy.

One switch may be outstanding because the game has one equipped weapon slot.
Another request returns `Busy`. `Queued` means no item mutation has committed;
`Switching` means the shared transfer owns the operation. `Succeeded` requires
the incoming firing grip and presentation, plus outgoing reattachment when
retention was needed. `Failed` and `Cancelled` are terminal too, but do not
imply that neither item moved. Flags retain the independently observed outgoing
removal/retention and incoming equip/grip/presentation facts. Reconcile actual
inventory and Grab hand snapshots before retrying or changing holster state.
Do not treat a historical result flag as proof that the player still holds an item.
The latest 64 terminal results are retained across owners within this provider
instance; an evicted or foreign command returns `RequestNotFound`.

Before commitment, cancellation or owner loss retires the request without equip.
After commitment, ROCK owns completion and recovery even if the consumer stops;
it keeps no consumer callback or engine pointer supplied by the consumer. On
world/skeleton/provider loss the request is cancelled and stale native work is
not resumed in the next world. Provider destruction discards its result history;
`NotReady` or `RequestNotFound` after lifecycle loss requires fresh snapshots,
not an automatic replay. The consumer releases its local command tracking before
closing its Core registration. No new permissions are granted by discovery.

## Result states and partial outcomes

`Queued` and `Switching` are pending states. `Succeeded`, `Failed` and `Cancelled` are terminal. Poll `getInventoryEquipResult` with the original owner and command ID; do not infer completion from `requestInventoryEquip` returning `RequestQueued`.

| Result flag | Observed milestone |
| --- | --- |
| `OutgoingRemoved` | The previous equipped instance was removed for its physical handoff. |
| `OutgoingRetained` | Its retained grab was successfully acquired. |
| `IncomingEquipAccepted` | Native equip and exact incoming stack validation succeeded. |
| `IncomingGripAcquired` | The requested physical firing hand acquired the incoming weapon. |
| `IncomingPresented` | Incoming grip presentation completed. |

A failed outgoing reattachment can coexist with an equipped incoming weapon. Inspect these flags and fresh inventory/hand state before retrying. Do not automatically equip again, spawn a replacement, or release the outgoing grab. A later manual equip does not erase the earlier inventory command's terminal outcome.

## Right and left consumer code

[`InventoryWeaponEquip.h`](../../examples/InventoryWeaponEquip.h) is the reusable, compiled helper;
[`InventoryWeaponEquip.cpp`](../../examples/InventoryWeaponEquip.cpp) shows both draw entry points. It stays inert
until the caller supplies a gesture and exact inventory stack.

```cpp
#include "InventoryWeaponEquip.h" // Copy the linked SDK example helper.

rock::sdk::example::InventoryWeaponEquip equip;
// client is an already connected rock::api::Client.
const auto connected = equip.connect(client); // explicitly requires Weapon 1.1
// Continue only when connected == rock::api::Status::Ok.

// Invoke ONE in the ROCK callback after your accepted draw gesture:
const auto admitted = drawWasLeftHand
    ? equip.drawLeft(weaponBaseFormId, exactStackIndex)
    : equip.drawRight(weaponBaseFormId, exactStackIndex);

// RequestQueued means accepted; do not run another native equip.
// On later callbacks, while equip.pending():
rock::api::weapon::v1_1::EquipResult result{};
const auto status = equip.poll(result);
// On Ok + Succeeded, confirm the draw in your holster state.
// On Failed/Cancelled, inspect flags and reconcile before another request.
// On activity/lifecycle shutdown, call equip.stop() before client.close().
```

An older ROCK provider reports `UnsupportedMinor`; do not access the added slots.
The old `v1` prefix remains callable with its original permissions and behavior.
The old Weapon transition snapshot reports this new operation with the existing
`Unknown` source value; use the new owner-scoped result to identify it.

## Compatibility and validation

The extension table contains the unchanged 16-slot `weapon::ApiV1` as its first
member, `v1`, followed by four added slots. On Windows x64 the old table is
128 bytes and the extended table is 160 bytes. No old record, permission, enum
meaning or Core requirement changes. Existing 1.0 consumers keep their original
headers and table-size request; only consumers using the additions need 1.1.
The old handling lease remains a separate operation and is not required to be
reimplemented by a holster mod.

The runtime and SDK builds, ABI/negotiation checks and both hand example tests
passed for this implementation. In-game holster switching is not yet qualified.
After integrating the calls, test both tabled directions, the retained weapon's
fresh release gesture, a busy drawing hand, and two modified copies of the same
base weapon. Verify the selected instance is equipped without duplication or loss.

## Complete public declarations

<details>
<summary>Complete public declarations: ROCK/WeaponV1_1.h</summary>

```cpp
#pragma once
#include "Weapon.h"
#include <cstddef>
#include <type_traits>

#pragma pack(push, 8)
namespace rock::api::weapon::v1_1 {
    inline constexpr InterfaceId kInterfaceId = weapon::kInterfaceId;
    inline constexpr std::uint32_t kMajor = 1;
    inline constexpr std::uint32_t kMinor = 1;

    // Capture on the ROCK frame thread immediately before requesting equip.
    // Keys are opaque comparison witnesses, never pointers to dereference or
    // identities to save. stackIndex is the native inventory stack index.
    struct InventoryWeapon {
        std::uint32_t size{sizeof(InventoryWeapon)};
        std::uint32_t baseFormId{};
        std::uint32_t stackIndex{};
        std::uint32_t count{};
        std::uint64_t stackKey{};
        std::uint64_t instanceKey{};
        std::uint64_t frameIndex{};
        std::uint32_t worldGeneration{};
        std::uint32_t skeletonGeneration{};
        std::uint32_t providerGeneration{};
        std::uint32_t reserved{};
    };

    struct EquipRequest {
        std::uint32_t size{sizeof(EquipRequest)};
        Hand hand{Hand::None};
        InventoryWeapon item{};
    };

    enum class EquipState : std::uint32_t {
        Unknown, Queued, Switching, Succeeded, Failed, Cancelled
    };
    enum class EquipFailure : std::uint32_t {
        None, LifecycleLost, OwnerLost, HandUnavailable, HandBusy,
        InventoryChanged, InfrastructureUnavailable, OutgoingTransferFailed,
        NativeEquipFailed, GripOrPresentationFailed, CancelledBeforeCommit,
        EquippedWeaponChanged, MenuBlocked
    };
    enum class EquipResultFlag : std::uint32_t {
        OutgoingRemoved = 1u << 0,
        OutgoingRetained = 1u << 1,
        IncomingEquipAccepted = 1u << 2,
        IncomingGripAcquired = 1u << 3,
        IncomingPresented = 1u << 4
    };
    struct EquipResult {
        std::uint32_t size{sizeof(EquipResult)};
        EquipState state{EquipState::Unknown};
        std::uint64_t commandId{};
        EquipFailure failure{EquipFailure::None};
        std::uint32_t flags{};
        Hand hand{Hand::None};
        std::uint32_t incomingFormId{};
        std::uint32_t outgoingFormId{};
        std::uint32_t outgoingReferenceFormId{};
        std::uint64_t transferSequence{};
        std::uint32_t worldGeneration{};
        std::uint32_t skeletonGeneration{};
        std::uint32_t providerGeneration{};
        std::uint32_t reserved{};
    };

    // Negotiate major 1, minimum minor 1 and sizeof(Api). Bind Weapon Read |
    // Write. All four additions are ROCK frame-thread calls, outside synchronous
    // Grab event callbacks. The original 1.0 table and headers remain unchanged.
    struct Api {
        static constexpr InterfaceId interfaceId=kInterfaceId;
        static constexpr std::uint32_t majorVersion=kMajor;
        static constexpr std::uint32_t minorVersion=kMinor;
        weapon::ApiV1 v1;
        Status(ROCK_CALL* captureInventoryWeapon)(OwnerToken, std::uint32_t baseFormId,
            std::uint32_t stackIndex, InventoryWeapon*) noexcept;
        // An explicit draw gesture equips into the requested physical hand.
        // A weapon carried only by the other hand is retained there using the
        // existing Toggle Drop controls. A busy destination rejects the request.
        // One switch may be outstanding. RequestQueued is admission, not success.
        Status(ROCK_CALL* requestInventoryEquip)(OwnerToken, const EquipRequest*, std::uint64_t*) noexcept;
        // Results are owner-scoped; the latest 64 terminal results are retained
        // for this provider lifetime. Inspect flags even when the switch fails.
        Status(ROCK_CALL* getInventoryEquipResult)(OwnerToken, std::uint64_t, EquipResult*) noexcept;
        // Cancellation is accepted only before outgoing removal/native equip.
        // Once committed, returns AlreadyCommitted and ROCK finishes cleanup
        // even if the requesting owner disappears. No consumer callback is kept.
        Status(ROCK_CALL* cancelInventoryEquip)(OwnerToken, std::uint64_t) noexcept;
    };
    static_assert(std::is_standard_layout_v<Api>);
    static_assert(offsetof(Api, v1) == 0);
    static_assert(offsetof(Api, captureInventoryWeapon) == sizeof(weapon::ApiV1));
    static_assert(sizeof(Api) == sizeof(weapon::ApiV1) + 4 * sizeof(void*));
}
#pragma pack(pop)
```
</details>
