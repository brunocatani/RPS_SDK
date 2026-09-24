# Current API behavior

This page describes current source contracts checked on 21 September 2026. Download availability and runtime qualification are separate from API source support.

## Observation boundaries

Core `Complete` is control completion. Core `Presented` follows final FRIK world presentation. Hands' presented pose getters retain the last completed capture with its own frame and generations; use `Presented` for current-frame results. Hands 1.1 `getRoles` follows physical firing/support assignments through handoffs.

Grab is the occupancy authority. Native weapon carry can occupy a hand without a captured ROCK grip. Attach-only weapon-part state does not grant carry ownership or permission to move a whole weapon. References provides native meaning for the identity reported by Grab.

## Weapon source identity

WeaponParts enumerates a bounded source catalog, including nodes without generated colliders. Source keys stay tied to weapon generation; `querySourcePose` reads current parent/root/world transforms, and 1.1 `querySourcePath` provides parent keys and child indices. Duplicate names do not establish identity. Rebuild or generation changes invalidate retained identities.

## Commands and authority

World force-grab and inventory transfer have distinct requests. Power Armor attachment requires current reference/generation guards and a supported animated armor-hand point. Queue admission is not success: read command stage/outcome and current hand state. Native release may end a successful attachment later.

Weapon handling exposes six implemented policy bits. The old removed exact-hand request and retired inert handling flags remain unavailable in Weapon 1.0. The opt-in [Weapon 1.1 inventory operation](modular/WeaponV1_1.md) supports either physical hand and retained switching. Animation visual authority does not relocate physical hands; Collision registration does not allocate or transfer ownership of bodies.

## Configuration and consumers

The current Configuration interface replaces separate configuration discovery. PALM binds it through Core and uses the compiled settings catalog. PAPER, SCISSORS, climbing and the SDK examples consume modular families. Their own feature policy and release status remain in their project guides.

See [migration](modular/Migration.md) for the old-to-current mapping and the [reference](ApiIndex.md) for exact signatures.

## Task-thread snapshot readers

The [runtime contract](RuntimeContract.md#synchronized-snapshots-from-task-threads) lists the seven synchronized hand, role, environment, occupancy, weapon and part-grip readers available from task threads. Read permissions and payload validity remain mandatory. Live queries/writes retain owner-thread guards; only FRIK FrameBegin establishes that owner.

## Occupancy and transition interpretation

`grab::HandInteractionStateV1` is the availability authority. `FiringGrip`, `NativeWeaponCarry`, `RockGripActive` and `AttachOnly` distinguish different ownership facts. A captured weapon-part grip can be inactive while native carry occupies the hand; Catching and Pulling are also busy phases. For a Weapon target, the form identifies the equipped base form and the retained target identity carries its weapon generation, not a loose world reference.

Weapon current identity (`weaponFormId`, `weaponGenerationKey`) is separate from active transition identity and retained terminal history. Check `Valid`, `WeaponEquipped`, `WeaponDrawn`, `WeaponPresent` and `PresentationKnown` as applicable. Interpret native renderability and hand-pose handoff only for the matching current observation. A prior terminal result does not establish present ownership or visibility.
