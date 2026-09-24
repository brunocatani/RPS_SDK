# Runtime and ownership contract

## Threads and callback lifetime

Discovery, registration, binding, Core snapshots/build identity, family `getSample`, and typed `copyEvents` do not require the animation owner thread. Configuration uses its own serialized store: `revision` has no file I/O, while `visit` and `setValue` belong on a configuration/game task outside latency-sensitive callbacks. Do not call ROCK from a borrowed visitor or synchronous Grab callback. Live queries and mutations run on ROCK's animation/game owner thread, established only by FRIK FrameBegin. Native graph observations cannot claim that thread. Before a game-thread identity exists, owner teardown and Core callback registration/removal are allowed for startup cleanup. After that identity is established, callback registration/removal and owner teardown must use that thread.

Core frame callbacks run at provider publication. Phase callbacks cover `BeforeRock`, `AfterRock`, `Complete`, `NativeGraphOutput`, and `Presented`. `Presented` runs after final FRIK world presentation and is for observation only. Hand readback at this phase is captured for the current frame; earlier phases can return the previous completed presentation with its own stamps. `Complete` precedes the final claimed-hand presentation pass; it does not promise that every rendered transform has already been finalized. Phase contexts describe their own frame and gates. Never infer that a phase permits physics writes solely because the callback is executing.

Callback userdata belongs to the consumer. Unregister on the owning thread before destroying it. Core callback slots are read immediately before each invocation, so removing a later callback during an earlier callback prevents that later invocation. A callback fault revokes the faulting owner's resources, preserving unrelated registrations and manual interaction.

Grab's synchronous event listener executes on the producer thread after ROCK's internal haptics have consumed the event. It is for immediate observation such as copying release kinematics. **Do not call ROCK from this listener.** Calls return `Busy`. The listener must not block, perform I/O, throw, retain event pointers, or free another live listener's userdata. Clear/unregister waits for a synchronous invocation to finish. Faults disable the listener immediately, reject further writes by that owner, and complete runtime cleanup on the game thread.

Configuration visitors borrow strings only for the current callback and must not call back into ROCK. Copy the required data, return, and perform writes afterward. This prevents iteration invalidation and registration teardown during a live visitor.

## Synchronized snapshots from task threads

The following copied reads require their family's Read permission but not the animation owner thread:

| Family | Reads |
| --- | --- |
| Hands | `getHandFrameV1`, `getHeadPose`, `getRoles` |
| Grab | `getHandInteractionStateV1` |
| Collision | `getEnvironment` |
| Weapon | `getEquippedWeaponStateV1` |
| WeaponParts | `getWeaponPartGripStateV1` |

These reads grant no scene pointers or write authority. Their frame/generation stamps describe the copied payload; do not overwrite them with a later `getSample`. Weapon-part source keys are resolved during owner-frame publication, so task reads do not walk live scene nodes. Presented-pose getters and live reference/source queries retain their owner-thread requirements. Synchronous Grab callbacks and Configuration visitors still forbid reentry.

## Samples, generations and units

`SampleV1` identifies the publication frame, state/publication sequence, world, skeleton, provider and collision generations. Compare the complete relevant sample when joining readbacks from different providers. A state sequence may remain unchanged across multiple frames; frame identity is also part of the sample. `BeforeRock` advances the provider clock from the measured game-loop timing. `NativeGraphOutput` observes the supplied timing without opening or advancing the next game frame, so a graph observation between frames cannot make the next publication reuse its predecessor. Phase callbacks can share frame identity with lifecycle notifications. Event sequences and state sequences are separate counters.

World/body identities are non-owning values. A body ID alone is not durable identity. Retain its world/collision generations and reject it after lifecycle changes. External registration requires the current nonzero world generation. A reference form ID can be strengthened with its native handle and generation guards. Weapon source keys are valid only with the corresponding weapon-generation key. None of these values grants permission to allocate, destroy or mutate an engine object.

Positions and distances labeled `Game` use game units. Fields labeled `Havok` use Havok units. Collision supplies both scale directions and their revision. Linear velocities use the explicitly named units per second; angular velocities use radians per second. Transforms contain nine rotation elements in row-major storage, translation and uniform scale, using the same transform convention as ROCK's runtime. Read validity flags before using a transform; a default zero rotation is not a valid pose.

All leases use the existing provider-publication boundary. A lease issued at published frame F for N frames is active before F+N. Retirement occurs at the publication after ROCK consumes that update's drives. This preserves a default one-frame drive submitted after a publication. Zero-length leases are invalid; requests above the family's supported maximum are clamped by the owning runtime service. Clear/release, owner loss and generation invalidation can retire a lease earlier. Do not substitute wall-clock timers for lease renewal.

## Buffers and failures

All calls return a fixed-width `Status`. Check the status before using outputs. `RequestQueued` is successful admission of a command, not successful execution. Use the returned command ID with the owning family's result getter: Grab commands use its result/state/stage readback; Weapon 1.1 uses `getInventoryEquipResult`. Cancellation can fail once a command is committed.

Records are frozen within their interface major. Initialize every sized input and output record with `{}`. Sized records require the exact current record size; records carrying a version require version 1. Initialize every sized element in output arrays as well. Capacities are element counts with the exact declared element stride, never byte counts. A capacity exceeding the published family maximum is rejected. A zero-capacity array can use a null data pointer, but required count/stream outputs must still be provided.

Successful copy calls report the number actually copied. Bounded snapshots without a total-count output provide up to the requested capacity; use the matching count function where available, or the family's published maximum to capture the complete bounded snapshot. `WeaponParts.copySources` provides copied and total counts. Do not treat a copied count equal to capacity as proof that no further records exist.

Valid output records/counts are cleared before owner admission. Invalid-size outputs are not overwritten. Inputs, errors and availability failures are distinct from successful empty snapshots. Record validity flags are authoritative even when a query succeeds. Catching an internal exception returns `InternalError` and produces a controlled diagnostic; exceptions do not cross the DLL boundary.

Caller buffers remain caller-owned. ROCK copies publications before returning. Configuration strings and synchronous callback records are borrowed only for the call. Public APIs expose no `bhkWorld`, `hknpWorld`, scene-node or reference pointers. A consumer that owns native physics work must obtain and protect its native context through its own validated runtime integration.

## Event cursors

Core, Collision, Grab, Touch, Weapon, WeaponParts, Animation, Input and Diagnostics expose their own typed event records. The shared `StreamV1` is only a cursor/retention envelope; it is not a feature-payload union. Each owner/family has a bounded 256-event retention ring and its own sequence. Other owners' events cannot create artificial gaps or consume that owner's retention. A sequence starts when the owner first binds Read, survives binding upgrades and provider recreation, and ends when that owner unregisters.

Pass the prior `nextSequence` as `afterSequence`. `copiedCount` is the output count, `remainingCount` allows draining, and `lostCount` counts records lost after that cursor. A cursor beyond the stream's latest sequence is invalid. An empty copy does not move the cursor. A newly created owner begins with cursor zero.

Collision's enriched external-contact stream has its own scope-aware cursor record. It retains a global contact sequence, while overwrite counters and gap detection are filtered to the requested owner/scope. Sequence gaps caused by another owner or scope do not indicate loss. `overwrittenCount` is cumulative retention loss for the scope, not a count of missed records since every supplied cursor. Scope zero reads all scopes for the owner. Explicit scope removal ends that scope's retained contact history.

Legacy F4SE messages 100–105 and 200 are no longer emitted as the consumer transport. Collision carries touch/touch-end notices; Grab carries grab/release notices and the detailed lifecycle events, including stash, consume, two-hand and release kinematics; Core carries physics initialization/shutdown and lifecycle changes. Event sequence and game frame are separate values. Producer events use the game clock and the producing physics instance's lifecycle generations, including initialization before its first snapshot publication. Internal haptics continue using their existing runtime event source.


## Version compatibility

Use exact major, minimum minor and table extent for each interface. Published records are fixed within their contract; never reinterpret a newer layout using an older header. Table discovery does not prove world readiness, a permission grant, or runtime qualification of an interaction. See [discovery](DiscoveryAndCapabilities.md) and [migration](modular/Migration.md).

## Weapon 1.1 inventory commands

All four [inventory-equip additions](modular/WeaponV1_1.md), including result polling, require the ROCK frame thread. They do not inherit the any-thread guarantee of `getEquippedWeaponStateV1`. Capture and request must use the same game frame and current nonzero lifecycle generations. Inventory keys are temporary opaque witnesses, never persistent item identifiers or native pointers.

Before commitment, owner loss or cancellation retires the request. After outgoing removal or native equip, ROCK owns completion/recovery even if the requesting owner unregisters; it retains no consumer callback. The outgoing retained grab becomes player-owned. Results are owner-scoped, with the latest 64 terminal results retained across owners for this provider instance, separate from event-stream retention. On provider recreation or an evicted command, reconcile current inventory/hand state instead of replaying the draw.
