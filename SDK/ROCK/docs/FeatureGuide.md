# Feature guide

This guide groups the API by what a mod can build. Read `RuntimeContract.md` before using any writer or authority surface.

## Observe hands and interaction

Use frame snapshots, primary/offhand queries, hand frames, presented frames/poses, interaction state, collision availability, semantic contacts, and provider events to build gesture/state telemetry, contextual UI, contact-driven feedback, or input-aware interactions.

Physics hand frames and presented visual poses are intentionally different surfaces. Use physics frames for collision/control decisions and presented poses for visuals.

## Observe weapons

Use classification, evidence catalogs and point clouds, emitter snapshots, equipped-weapon state, grip state, composition, part poses, drive results, scope sight state, and semantic target resolution to build sights, emitters, attachment-aware logic, weapon diagnostics, or interaction affordances.

`weaponGenerationKey` binds all weapon-local identity. Rebuild caches whenever it changes.

## Control weapon parts

Weapon part targets declare what may be interacted with; drive targets request bounded transforms or semantic motion. Read target resolution first, refresh drives as leases, and inspect application results instead of assuming the requested transform was accepted.

Nonexclusive target flags allow compatible consumers to describe overlapping observations without taking hidden ownership. Actual handling/drive authority remains explicit.

## Integrate external physics bodies

Register body IDs owned by your mod, preferably in stable scope tokens. ROCK then emits owner/scoped contacts involving hands, weapons, held objects, body colliders, or other classified sources. Clear the scope before destroying/recycling bodies.

The provider does not own or keep your bodies alive. Registration is contact-routing metadata, not object lifetime transfer.

## Issue interaction commands

Force grab, force release, and thrown drop are queued through ROCK's canonical interaction paths. Commands validate owner, capability, hand, target identity, distance/generations, and current hand state before execution. Admission is not completion: retain the command ID and poll its terminal result.

Force grab is near-range and targets a live loose object/weapon. Force release defaults to a gentle physical drop. Thrown drop can use captured controller motion. Supplied Havok velocities are finite-checked but trusted and are not clamped.

## Power Armor and reference details

The Power Armor family extends V1 to 99 function pointers / 792 bytes on x64.
Require `ROCK_PROVIDER_API_V1_POWER_ARMOR_TABLE_BYTES` at initialization and
register the capabilities used by your integration:

| Function | Capability | Data or action |
| --- | --- | --- |
| `getHandTargetDetailsV1` | `TargetDetails` | Either player's hand: interaction state, resolved reference, body/layer, optional anchor/normal/mesh evidence, and the selected armor point. |
| `queryReferenceInteractionV1` | `TargetDetails` | Native reference identity, activation-block state, open state, and indexed furniture use. |
| `queryPowerArmorTargetV1` | `PowerArmor` | Queried reference, PA classification, linked frame identity, and two copied animated armor-hand poses. |
| `requestPowerArmorGrabV1` | `PowerArmor` + `InteractionCommands` | Queue a specific player's hand grabbing a specific armor hand; returns a normal `ForceGrab` command result. |

Request `FrameSnapshots` for owner frame callbacks. Run live queries at that
boundary, initialize every request/output with `{}`, and copy the current
world, skeleton, and provider generations. The PA grab requires all three
generation guards to be nonzero. `referenceFormId` identifies a live world
reference; `referenceNativeHandle`, when nonzero, adds an identity check.

### Classification and optional native state

An `Ok` query is not a positive PA classification and does not guarantee a frame
or valid bone pose. Inspect `RockProviderTargetDetailFlagV1` on the containing
output:

- `PowerArmorClassification` means classification was available.
- `PowerArmorFrame` means `frameReference` identifies a resolved PA frame.
- `PowerArmorActor` means the queried reference was verified as an actor in PA;
  only that case supplies `actorFormId`. Querying furniture does not discover
  its occupant, and furniture reservations do not prove there is a wearer.
- Each nested reference has its own flags. `ActivationBlocked`, `OpenState`,
  and `FurnitureUse` determine whether the corresponding fields are available;
  a zero field with a missing flag is unknown, not a negative answer.

`RockProviderNativeOpenStateV1` distinguishes `NotApplicable`, `Open`, `Opening`,
`Closed`, and `Closing`. `activationBlocked` is one native property, not a complete
can-activate verdict. `furnitureInUse` and `furnitureInUseIncludingReservations`
refer to the requested `furnitureMarkerIndex`; zero selects marker 0, not any
marker. These queries do not activate a reference or drive entry/exit animations.

### Armor-hand poses and command admission

`LeftArmorHand` and `RightArmorHand` name the armor's sides. `request.hand` names
the player's hand independently. Match each entry's `point` enum and check its
`valid` field. `world` and `frameLocal` are copied transforms in game units at
the animated armor-bone origin; they are not sampled human palm poses. The two
points can have different availability. Requery in the current callback instead
of retaining a live engine pointer or treating yesterday's pose as current.

Use the returned frame's form ID and native handle in the grab request. Leave
`maxDistanceGame = 0` for ROCK's native proximity radius, currently 8 game units,
or supply a finite value greater than zero and at most 32 game units. Neither
query success nor proximity guarantees admission: ROCK rechecks generations,
hand availability, the frame, the animated point, and the live attachment path.

`RequestQueued` returns a command ID, not an attachment. Poll
`getInteractionCommandResultV1` for `Succeeded`, `Rejected`, or `Cancelled`.
Cancel a pending grab if its activity ends or writes become blocked, and still
observe its terminal result because it may have executed before cancellation.

### Grip readback and cleanup

After success, use `getHandTargetDetailsV1` to observe the current target and
`powerArmorPoint`; the hand state's `surfaceGripMode` is `AnimatedArmorBone`.
Inspect anchor and normal validity independently. A bone-origin grip does not
promise a triangle normal or mesh-part identifier. Hand-target queries observe
either hand regardless of owner and do not grant ownership over a manual grip.

Native grab-button release can end a commanded attachment. A successful command
record remains historical: do not keep reporting held state after live readback
shows release, and do not automatically reacquire until your activity is rearmed.
For an activity that still owns a live grip, queue `requestForceReleaseV1` with
`RequireMatchingTarget`, the held frame form ID, and current generation guards.
That flag protects target identity; it is not a command-owner or grip-sequence
guard. Track live state and stop issuing releases once the attachment is gone.

On teardown, cancel pending commands, unregister callbacks, then unregister the
consumer. ROCK checks command ownership during touch-grab updates and releases
attachments whose command owner is no longer registered. Generation loss also
invalidates cached state. The inert
[PowerArmorInteraction example](../examples/mods/PowerArmorInteraction.cpp)
demonstrates classification/point validation, command polling, native release
readback, matching release, and generation cleanup.

## Reserve/suppress input

Hand input suppression and offhand reservation are owner-scoped leases. Publish only the exact flags/mode you need, refresh while active, inspect effective state, and clear/release immediately when the feature stops.

Raw wand state exposes sample metadata so a consumer can distinguish current input from stale/unavailable input. Native Pip-Boy/VATS/V.A.N.S. suppression remains explicit and separately feature-gated.

`getLogicalInputActionStateV1` observes the configured semantic `Jump` action and exposes sample/press sequences. Consumers should edge-detect `pressSequence`, ignore unavailable samples, and prime their cursor after lifecycle blocking so input generated in a menu is not replayed as gameplay.

## Drive animation and presentation

Native animation authority, phase callbacks, runtime publication, hand visual authority, presented pose readback, and handling authority form a coordinated animation surface. Acquire the narrow authority first, write only under `VisualWriteAllowed`, use generation guards, and deterministically clear every authority/publication.

The programmatic exact-hand weapon request was removed: a physical handoff is the only way to change the carrying hand. `requestEquippedWeaponHandV1` remains ABI-stable but declines every well-formed request with `HandUnavailable`, and `EquippedWeaponHandRequest` is no longer advertised.

## Build touch mechanisms and climbing

Touch-grab targets support:

- `FixedAnchor`: attach a contacting hand to an exact body or wildcard surface without changing body motion type;
- `LimitedHinge`: expose bounded rotational motion on a named live body;
- `LimitedPrismatic`: expose bounded linear motion on a named live body.

Targets are replaced transactionally per scope and refreshed as leases. Exact-body registrations resolve before wildcard registrations. One wildcard descriptor owns at most one resolved body; publish separate right/left descriptors for simultaneous two-hand surface interaction.

Before a script or native system moves a held mechanism, request yield and wait until state reaches `Yielded`. Republishing with a new target generation re-arms a yielded/invalidated descriptor.

Climbing consumers can combine touch-grab state, bounded world raycasts, logical Jump observation, and `PlayerController` state. The controller snapshot is pointer-free and can explicitly check current penetration. `requestPlayerControllerJumpV1` uses FO4VR's native jump state machine; ROCK rejects stale generations, blocked physics writes, invalid height, unavailable controllers, and penetrating starts. A consumer should validate its own ledge/floor/headroom geometry first, clear its touch targets only after the jump request succeeds, and require physical grab release before rearming.

## Query the world

World raycasts are owner-callback-only, bounded, and use ROCK's validated filter policy. The current extended limits publish eight raycasts per owner per frame and the request range is capped at 8192 game units. Use them for short diagnostic/sight/interaction probes, not bulk scene queries.

## Publish diagnostics

Debug overlay lines/text are copied into a bounded owner publication and rendered through ROCK's stereo overlay. Collider visualization override selects one exact body from the current complete weapon body catalog and refreshes as a rolling lease. These surfaces are useful for development tools and user-facing diagnostics without building a second VR renderer.

## Configuration editors and interactive panels

Use [Configuration.md](Configuration.md) for the separate catalog/write export,
[CurrentBehavior.md](CurrentBehavior.md) for attach-only/input/inventory contracts,
and [PlayerController.md](PlayerController.md) for logical Jump and controller access.
RPS UI supplies the panel host for interactive configuration and status displays;
consumers retain their own ImGui context and application policy.
