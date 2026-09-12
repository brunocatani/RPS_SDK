# Current interaction contracts

Checked against current local source on 2026-09-11. API V1 remains V1; recent
behavior changes do not imply a new version number or a changed slot for every
feature. Check negotiated table extent, feature bits and capability grants.

## Attach-only weapon parts

`RockProviderWeaponPartGrabModeV1::AttachOnly` is provider-mandated attachment to
a part. The hand follows that part without steering or carrying the weapon. It
must not become a carry pivot, count as the final carrier, inherit the firing
grip, or be upgraded to full authority by proximity or authored grip selection.
`FullTwoHandAuthority` likewise must not be downgraded by the local proximity rule.

Read `getWeaponPartGripStateForHandV1` for either physical hand. Its
`RockProviderWeaponPartGripKindV1`, `attachOnly`, target identity and generation
describe what the hand holds. `active` alone is not evidence of carry authority.
Downstream manipulation observers should preserve this attach-only distinction.

Current equipped-weapon input distinguishes three cases:

| Grip | Button behavior |
| --- | --- |
| Firing grip, either hand | Toggle behavior; firing-grip detachment still requires the applicable handling policy. |
| Ordinary dynamic/full-authority/authored support grip | Follows the consumer `bToggleGrab` setting. |
| Attach-only part | Always hold-to-grab; release the button to detach. |

Loose objects, fixed surfaces and provider touch grabs also use hold-to-grab.
Do not latch a reload handle indefinitely because the player enabled support
toggle-grab. Part targets/drives remain generation-bound leased publications;
remove or refresh them according to the existing scope/ownership contract.

## Current handoff zones and part indicators

Handoff and firing-grip reattachment use the same lateral-cylinder geometry
and sphere indicators. The existing promotion distance now controls handoff
cylinder reach; cylinder width uses the existing reattachment cylinder-radius
setting. Current canonical/captured firing frames and generation identity are
required, so stale data cannot create a hover zone.

Visible attach-only part markers are generated from the same target resolver
used for contact routing. Higher-priority full-authority matches suppress an
attach-only mark; source-local centers, visibility, bounds and generation checks
control marker publication. This is runtime behavior behind existing part-target
APIs. `copyWeaponPartTargets` is an internal helper, not a new public SDK call.

## Inventory-to-hand commands

`requestForceGrabV1` supports `FromPlayerInventory` when feature word 1 advertises
`InventoryForceGrab`. This mode uses an owned ALCH/throwable **base form ID** in
`targetFormId`, not a world reference. Set `hand = None`, use only that flag,
retain the default invalid body ID, and leave `maxDistanceGame = 0`. ROCK selects
a free hand, preferring the right, and transfers exactly one item without using
or equipping it. The ordinary loose-world-object mode still requires an explicit
hand and a world reference form ID.

Admission returns `RequestQueued` with a command ID. Poll the terminal result
or consume owner events; do not decrement a UI inventory count on admission.
The provider keeps inventory unchanged before a drop failure and attempts to
return the exact drop on failed attachment. A failed rollback must remain a
reported runtime error rather than being represented as successful handoff.
Keep the normal cancellation and generation-loss cleanup path.

## Input and UI

The current provider table has 99 slots (792 x64 bytes). Slots 93 and 94 add
`getRawWandThumbstickV1` and `getNativeInputContextV1`. Before reading either
pointer, require its boundary using `offsetof + sizeof(member)`; V1 alone is
insufficient. Native context is frame-callback-only: zero is unavailable,
`MenuActive` gives native menus priority, and `PrimaryActivationTarget` protects
native activation holds. Raw thumbstick failure returns false and zero axes.

`SuppressGrenadeQuickDraw` lets a usable consumer temporarily replace ROCK's
quick draw interaction. Release suppression when its replacement cannot run.
Pip-Boy native suppression is lease-derived; simply holding an object is no
longer a flashlight suppression reason. Native V.A.N.S. and right-stick Favorites
suppression are mandatory behavior, not optional consumer INI toggles.

The removed exact-hand request slot remains callable only for ABI compatibility
and declines requests. Its retained flag and struct names do not restore that
feature. Physical bilateral handling, authored grip selection and native
handedness must be distinguished in consumer UI.

## Configuration and presentation

Use the separate [configuration API](Configuration.md) for consumer/developer
settings. Use [RPS UI](../../UI/README.md) for new interactive world panels;
ROCK's bounded debug-overlay API remains useful for diagnostic lines and text.
ROCK's own provider adapter owns final skeleton presentation. Physics hand
frames and presented hand poses can differ within a frame; select the surface
that matches the operation and never mix generation identities.
