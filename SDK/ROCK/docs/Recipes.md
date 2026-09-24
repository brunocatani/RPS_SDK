# Consumer recipes

Each recipe assumes a successfully registered owner and the listed interface permissions. Schedule gameplay calls on the game owner thread and invalidate cached state on lifecycle loss. The [first consumer](GettingStarted.md) shows complete connection and cleanup.

## Final hand observation

Bind Hands Read and Core Read + Callbacks. Subscribe to Core phase callbacks, wait for `AnimationPhaseV1::Presented`, query `getPresentedHandPoseV1`, and check the pose's frame/generations against the phase context. The finger mask independently identifies valid finger locals. For physical firing/support assignment, read Hands `getRoles`; for availability, read Grab `getHandInteractionStateV1`.

## Loss-aware Grab events

Bind Grab Read. This fragment makes one bounded batch per call; invoke it from your scheduled owner callback. The returned cursor belongs to this owner and this family only.

```cpp
#include <ROCK/Grab.h>
#include <array>

rock::api::Status readGrabEvents(
    const rock::api::grab::ApiV1& grab, rock::api::OwnerToken owner,
    std::uint64_t& cursor, std::array<rock::api::grab::EventV1, 16>& events,
    rock::api::StreamV1& stream) noexcept
{
    events = {};
    stream = {};
    const auto status = grab.copyEvents(owner, cursor, events.data(),
        static_cast<std::uint32_t>(events.size()), &stream);
    if (status == rock::api::Status::Ok && stream.copiedCount)
        cursor = stream.nextSequence;
    return status;
}
```

Process only `stream.copiedCount` entries after `Ok`. `lostCount` requires rebuilding current state from snapshots; `remainingCount` permits another bounded batch on a later tick. Do not create an unbounded drain loop. Start the cursor at zero when a new owner is created. Synchronous Grab callbacks are an alternative observation path: copy the borrowed event and return without calling ROCK.

## Consumable and throwable inventory handoff

Bind Grab Read + Write. Queue only after your own explicit user action, with current nonzero world/skeleton/provider generations and a base-form ID. This request moves a consumable or throwable weapon into an automatically selected available hand. Set `Hand::None`; it does not equip firearms. For a holstered firearm and explicit right/left selection, use [Weapon 1.1](modular/WeaponV1_1.md).

```cpp
#include <ROCK/Core.h>
#include <ROCK/Grab.h>

rock::api::Status queueInventoryItem(
    const rock::api::grab::ApiV1& grab, rock::api::OwnerToken owner,
    const rock::api::core::SnapshotV1& frame,
    std::uint32_t baseFormId, std::uint64_t& command) noexcept
{
    command = 0;
    const auto writable = static_cast<std::uint32_t>(
        rock::api::core::LifecycleFlag::PhysicsWriteAllowed);
    if (!frame.providerReady || (frame.lifecycleFlags & writable) == 0 ||
        !frame.worldGeneration || !frame.skeletonGeneration ||
        !frame.providerGeneration) return rock::api::Status::NotReady;
    rock::api::grab::InventoryGrabRequestV1 request{};
    request.hand = rock::api::Hand::None; // ROCK chooses an available hand.
    request.baseFormId = baseFormId;
    request.worldGeneration = frame.worldGeneration;
    request.skeletonGeneration = frame.skeletonGeneration;
    request.providerGeneration = frame.providerGeneration;
    return grab.requestInventoryGrab(owner, &request, &command);
}
```

Accept only `RequestQueued` as command admission. Poll `getInteractionCommandResultV1` until `Succeeded`, `Rejected` or `Cancelled`; inspect stage/failure details. Cancel pending work when its activity ends. A completed grab can become player-owned; do not release an unrelated manual grip during cleanup.

## Holster draw and retained weapon switching

Acquire `rock::api::weapon::v1_1::Api` with Weapon Read + Write. On the ROCK frame thread, capture the exact inventory stack and submit one request with `Hand::Right` or `Hand::Left`. Capture and submit in the same game frame. The existing weapon, if carried only by the other hand, becomes a retained physical grab there using Toggle Drop release controls.

The [compiled helper](../examples/InventoryWeaponEquip.h) provides both calls after successful `connect(client)`:

```cpp
const auto admitted = drawingHandIsLeft
    ? equip.drawLeft(weaponFormId, exactStackIndex)
    : equip.drawRight(weaponFormId, exactStackIndex);
```

Accept `RequestQueued`, then call `equip.poll(result)` on later frame callbacks. Update holster state after `Succeeded`; on `Failed` or `Cancelled`, reconcile the result flags and actual item state before retrying. Do not also run the consumer's native equip. Call `equip.stop()` before closing its Core owner; `AlreadyCommitted` means ROCK owns the remaining cleanup. See the [complete 1.1 contract and both hand examples](modular/WeaponV1_1.md).

## Weapon sources and drives

Bind WeaponParts Read to enumerate `copySources` at the current weapon-generation key. Respect copied/total counts and the 4096-source limit. Query current transforms with `querySourcePose`; use `querySourcePath` only when your plugin already owns an independently validated native weapon context. Never cast a source key to a pointer or guess identity from duplicate names.

Add Write only for part targeting/driving. Publish narrowly matched targets, read resolution/grip state, publish finite generation-matched drive targets with a short lease, and inspect `copyWeaponPartDriveApplicationResultsV1`. Clear drives before clearing targets and ending the activity; ROCK restores captured native locals. See the [WeaponParts contract](modular/WeaponParts.md).

## Scoped external contacts

Bind Collision Read + Write. Register consumer-owned bodies under an owner-local scope with nonzero current world generation and body generation. Replace a scope transactionally; clear it before destroying any body. Poll `copyExternalContactsSinceV1` using its own stream state and `lastCopiedSequence`. This contact cursor is not `StreamV1::nextSequence`; unrelated owners/scopes do not count as contact loss.

## Configuration UI

Bind Configuration Read + Write. Copy records and strings during `visit`, render that copied catalog, and schedule `setValue` from the configuration task. Do not reenter ROCK from the visitor. Use the owning `Group`, key, and supplied error buffer; watch `revision` to observe application. On shutdown, remove UI/input callbacks through their own APIs and close the ROCK owner on its allowed thread.
