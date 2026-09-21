# Power Armor integration

Power Armor interaction composes two independently negotiated interfaces. Bind References Read for classification/native state and Grab Read + Write for target details, command admission, polling and release. Add Core Callbacks only when using ROCK scheduling.

| Operation | Owning interface |
| --- | --- |
| `queryReferenceInteractionV1`, `queryPowerArmorTargetV1` | References |
| `getHandTargetDetailsV1`, `requestPowerArmorGrabV1` | Grab |
| Command result/cancellation/matching release | Grab |

Initialize all requests with `{}`. `references::ReferenceQueryV1` carries native identity/marker observation; `grab::PowerArmorGrabRequestV1` carries a separate frozen `ReferenceKey`, player's physical hand and armor point. It does not embed a References record. Exact flags and declarations are in [References](References.md) and [Grab](Grab.md).

## Classification and optional native state

An `Ok` query is not a positive PA classification and does not guarantee a frame
or valid bone pose. Inspect `references::TargetDetailFlagV1` on the containing
output:

- `PowerArmorClassification` means classification was available.
- `PowerArmorFrame` means `frameReference` identifies a resolved PA frame.
- `PowerArmorActor` means the queried reference was verified as an actor in PA;
  only that case supplies `actorFormId`. Querying furniture does not discover
  its occupant, and furniture reservations do not prove there is a wearer.
- Each nested reference has its own flags. `ActivationBlocked`, `OpenState`,
  and `FurnitureUse` determine whether the corresponding fields are available;
  a zero field with a missing flag is unknown, not a negative answer.

`references::NativeOpenStateV1` distinguishes `NotApplicable`, `Open`, `Opening`,
`Closed`, and `Closing`. `activationBlocked` is one native property, not a complete
can-activate verdict. `furnitureInUse` and `furnitureInUseIncludingReservations`
refer to the requested `furnitureMarkerIndex`; zero selects marker 0, not any
marker. These queries do not activate a reference or drive entry/exit animations.

## Armor-hand poses and command admission

`LeftArmorHand` and `RightArmorHand` name the armor's sides. `request.hand` names
the player's hand independently. Match each entry's `point` enum and check its
`valid` field. `world` and `frameLocal` are copied transforms in game units at
the animated armor-bone origin; they are not sampled human palm poses. The two
points can have different availability. Requery in the current callback instead
of retaining a live engine pointer or treating yesterday's pose as current.

Use the returned frame form ID and native handle in `grab::PowerArmorGrabRequestV1::target`, a `ReferenceKey`, together with the current nonzero world/skeleton/provider guards. Leave
`maxDistanceGame = 0` for ROCK's native proximity radius, currently 8 game units,
or supply a finite value greater than zero and at most 32 game units. Neither
query success nor proximity guarantees admission: ROCK rechecks generations,
hand availability, the frame, the animated point, and the live attachment path.

`RequestQueued` returns a command ID, not an attachment. Poll
`getInteractionCommandResultV1` for `Succeeded`, `Rejected`, or `Cancelled`.
Cancel a pending grab if its activity ends or writes become blocked, and still
observe its terminal result because it may have executed before cancellation.

## Grip readback and cleanup

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
[PowerArmorInteraction example](https://github.com/brunocatani/RPS_SDK/blob/main/SDK/ROCK/examples/mods/PowerArmorInteraction.cpp)
demonstrates classification/point validation, command polling, native release
readback, matching release, and generation cleanup.
