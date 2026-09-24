# Migrate a ROCK consumer

The current ROCK runtime requires the modular discovery contract. Previous monolithic discovery exports are removed; legacy headers in the SDK do not make those DLL exports available.

## Required changes

1. Resolve `ROCKAPI_QueryInterfaceV1` from loaded ROCK.dll and negotiate Core V1 exactly. Register one owner name, then independently acquire each required feature table and its local permissions. `Client.h` implements this startup pattern without importing all feature headers.
2. Replace the combined frame with Core lifecycle, Hands poses, Grab occupancy, Weapon equipped/role state and Collision environment. Compare frame/generation identity when joining them.
3. Replace engine witnesses with values. A body ID needs its world/collision generation; a reference uses form/native-handle guards; a weapon source uses its generation and source key. Enumerate sources and use `querySourcePose` to replace direct node reads, including non-collider nodes.
4. Use the dedicated inventory request instead of a mode bit on a world-grab request. PA grabbing accepts a ReferenceKey plus anchor kind. Remove the unsupported exact-hand equip request and inert handling bits.
5. Register callbacks under the single owner, using Core's callback permission. Apply generation/readiness gates inside the callback. Retain no borrowed callback or configuration pointers. Synchronous grab listeners must copy data and return without calling ROCK.
6. Replace global event/F4SE payload parsing with the relevant typed streams. Keep one cursor per owner/family. Preserve synchronous release observation where the behavior requires it; do not process the polling and callback forms twice.
7. Use scope registration for external bodies and Touch targets. World generation and owner/scope ownership are explicit. Clear authorities/scopes/listeners before releasing their backing resources or userdata.
8. Check `Status`, then validity and copied counts. Initialize every sized array element. `RequestQueued` means admission; observe command application or failure separately.
9. Remove copied provider headers and runtime source-directory include fallbacks. Link the explicit SDK targets. Keep legacy headers only in the SDK's clearly named legacy lane for unmigrated checkouts.
10. Build the affected consumer on its existing branch, run its relevant tests, and qualify interactions in the assembled FO4VR stack. A successful build does not establish in-game behavior.

## Changed data joins

| Previous assumption | Modular contract |
|---|---|
| One global version, table size and capability mask | Exact major/minor/table extent and permission mask for each provider |
| Frame snapshot is every feature's state | Family-owned readbacks, joined by samples and generations |
| Hand frame / captured grip determines occupancy | Grab's canonical hand interaction state |
| `sourceRoot`, `interactionRoot`, `node`, `weaponNode` are usable public pointers | Generation-bound source keys and copied poses |
| External body `ownerToken` in each record is a consumer token | Parent owner + scope in the call; record carries world and body generation |
| `HandTargetDetails.reference` embeds reference semantics | Grab target identity joined to a References query |
| PA grab embeds a reference-query DTO | Frozen ReferenceKey and armor-anchor kind |
| Provider event sequence is the game frame | Separate event sequence, game frame and feature subject/state sequence |
| Structure-size lookup makes changed V1 array strides safe | Fixed per-function record layouts; new functions/records for extensions |
| Penetration flags are implemented | No penetration query in PlayerController V1 |
| Configuration has its own discovery export | Configuration V1 discovered through the same small bootstrap |

## Old slot inventory

The historical inventory below accounts for all former monolithic slots, including already retired operations. New public signatures, status handling and record layouts are defined by the individual interface pages, not by the old names in this inventory.

## Appendix A — Complete old operation disposition

This table accounts for the 99 former slots. The destinations below identify the current family or retired operation; old signatures are not ABI-compatible with the new tables. Use the current family declarations for each migrated call. Metadata and combined-state operations now split across families.

| Old slot | Former function | Current destination | Disposition |
| ---: | --- | --- | --- |
| 0 | getVersion | Bootstrap | Replace with per-interface major/minor in discovery metadata; no global provider API version. |
| 1 | getModVersion | Core | Core build identity; keep mod/SDK/interface versions distinct. |
| 2 | isProviderReady | Core | Core lifecycle availability plus local feature readiness; do not promise all features are writable. |
| 3 | registerFrameCallback | Core | Retire ownerless form; replace with Core owner-bound subscription. |
| 4 | unregisterFrameCallback | Core | Retire ownerless form with slot 3. |
| 5 | getFrameSnapshot | Core / feature-local contracts | Decompose across Core, Hands, Grab, Weapon, Collision and reservation owners; share sample identity. |
| 6 | queryWeaponContactAtPoint | WeaponParts | Retain weapon-evidence point query with part/body keys and explicit units. |
| 7 | clearExternalBodies | Collision | Collision clear-all-scopes for this owner; do not unregister the Core owner. |
| 8 | setOffhandInteractionReservation | Grab | Retire legacy setter in favor of the Grab lease contract at slots 77–80. |
| 9 | registerExternalBodiesV1 | Collision | Consolidate into Collision owner/scoped registration; preserve replace semantics. |
| 10 | getWeaponEvidenceDetailCountV1 | WeaponParts | Use WeaponParts.getWeaponEvidenceDetailCountV1. |
| 11 | copyWeaponEvidenceDetailsV1 | WeaponParts | Copy evidence catalog; preserve catalog entries beyond the compact eight-body snapshot. |
| 12 | getWeaponEvidenceDetailPointCountV1 | WeaponParts | Count/query points by generation-qualified part/body identity. |
| 13 | copyWeaponEvidenceDetailPointsV1 | WeaponParts | Copy evidence points with explicit coordinate space and bounded counts. |
| 14 | getBodyContactSnapshotV1 | Collision | Body-contact stream/snapshot with game-frame stamps and contact validity. |
| 15 | getPrimaryHandV1 | Weapon | Current firing role remains in Weapon; Hands.getRoles supplies a publication-stamped primary/offhand pair. |
| 16 | getOffhandHandV1 | Weapon | Current support role remains in Weapon; roles follow firing-hand transfers, while Left/Right stay fixed physical identities. |
| 17 | getHandFrameV1 | Hands | Raw physical-hand pose only; move occupancy/body/weapon-role details to their owners. |
| 18 | registerConsumerV1 | Core | Core registration once per consumer, followed by local interface permission bindings. |
| 19 | unregisterConsumerV1 | Core | Core revocation across every owner-bound feature resource. |
| 20 | getGrantedCapabilitiesV1 | Core | Replace global mask with grants keyed by interface ID and major. |
| 21 | getProviderLimitsV1 | Core / feature-local contracts | Split into Core limits and feature-local limits/feature metadata. |
| 22 | getExternalContactSnapshotForOwnerV1 | Collision | Retire duplicate legacy contact representation; use the richer Collision cursor contract. |
| 23 | requestForceGrabV1 | Grab | Separate world-reference and inventory-item request shapes; retain one Grab command lifecycle. |
| 24 | getInteractionCommandResultV1 | Grab | Grab-local command result, including its own failure reasons and terminal history. |
| 25 | requestForceReleaseV1 | Grab | Preserve normal release and matching-target semantics; identity key replaces pointer witness. |
| 26 | requestThrownDropV1 | Grab | Preserve throw velocity units and normal release/restore path. |
| 27 | setHandInputSuppressionV1 | Input | Input-local suppression/chord lease publication. |
| 28 | clearHandInputSuppressionV1 | Input | Input-local clear with explicit idempotent owner behavior. |
| 29 | setWeaponPartTargetsV1 | WeaponParts | WeaponParts semantic target publication, with generation-qualified source keys. |
| 30 | clearWeaponPartTargetsV1 | WeaponParts | Clear only this owner's part targets. |
| 31 | setWeaponPartDriveTargetsV1 | WeaponParts | WeaponParts drive publication; preserve one-frame consumption and matcher fidelity. |
| 32 | clearWeaponPartDriveTargetsV1 | WeaponParts | Clear drives and preserve scene-state restoration. |
| 33 | queryEquippedWeaponClassificationV1 | Weapon | Weapon classification snapshot. |
| 34 | getWeaponPartGripStateV1 | WeaponParts | WeaponParts captured-grip details; generic occupancy stays in Grab. |
| 35 | getRawWandButtonStateV1 | Input | Input sample status, physical side, touch/buttons, sequences and freshness. |
| 36 | isNativePipboyInputSuppressedV1 | Input | Replace misleading broad boolean with precisely named native-action suppression state. |
| 37 | getWeaponEmitterCountV1 | Weapon | Weapon emitter count/catalog metadata. |
| 38 | copyWeaponEmittersV1 | Weapon | Weapon emitter snapshots with explicit origin/direction/space. |
| 39 | setNativeAnimationAuthorityV1 | Animation | Animation native-authority lease. |
| 40 | clearNativeAnimationAuthorityV1 | Animation | Animation native-authority release. |
| 41 | getNativeAnimationAuthorityStateV1 | Animation | Animation authority/runtime observation. |
| 42 | registerAnimationPhaseCallbackV1 | Core | Core phase subscription; do not require Animation for generic scheduling. |
| 43 | unregisterAnimationPhaseCallbackV1 | Core | Core phase unsubscribe, preserving in-flight callback rules. |
| 44 | getEquippedWeaponGripStateV1 | Weapon | Weapon grip/muzzle/frame snapshot without foreign mutable DTOs. |
| 45 | setHandVisualAuthorityV1 | Animation | Animation hand/finger visual publication; common transforms, family-owned record. |
| 46 | clearHandVisualAuthorityV1 | Animation | Animation visual-authority clear/restore. |
| 47 | publishNativeAnimationRuntimeV1 | Animation | Animation native runtime-provider publication. |
| 48 | setEquippedWeaponHandlingAuthorityV1 | Weapon | Weapon handling authority; remove only audited inert fields/flags. |
| 49 | clearEquippedWeaponHandlingAuthorityV1 | Weapon | Weapon handling release. |
| 50 | getEquippedWeaponHandlingStateV1 | Weapon | Weapon handling/occupancy observation; keep generic occupancy canonical in Grab. |
| 51 | publishDebugOverlayV1 | Diagnostics | Diagnostics bounded overlay publication. |
| 52 | clearDebugOverlayV1 | Diagnostics | Diagnostics owner overlay clear. |
| 53 | getPresentedHandFrameV1 | Hands | Hands presented pose snapshot with explicit sample phase; no nested Grab/Collision state. |
| 54 | getProviderLimitsExtV1 | Family contracts | Use the public family constants and fixed record bounds. |
| 55 | getPublicStructureSizeV1 | Core / feature-local contracts | Retire the global structure-ID registry; fixed family records and local ABI manifests replace it. |
| 56 | registerFrameCallbackForOwnerV1 | Core | Core owner frame callback. |
| 57 | unregisterFrameCallbackForOwnerV1 | Core | Core owner frame unsubscribe. |
| 58 | getHandInteractionStateV1 | Grab | Grab canonical occupancy/interaction snapshot with stable sequences. |
| 59 | copyProviderEventsSinceV1 | Core / feature-local contracts | Split global event payload into Core lifecycle/revocation and typed feature streams. |
| 60 | getEquippedWeaponStateV1 | Weapon | Weapon current identity/presentation plus distinct transition/terminal histories. |
| 61 | registerExternalBodiesForScopeV1 | Collision | Collision scoped external-body replacement. |
| 62 | clearExternalBodiesForScopeV1 | Collision | Collision scoped clear. |
| 63 | copyExternalContactsSinceV1 | Collision | Collision enriched contact cursor with precise owner/scope loss semantics. |
| 64 | queryWeaponPartTargetResolutionV1 | WeaponParts | WeaponParts target resolution and winning-owner/matcher evidence. |
| 65 | copyWeaponPartPoseSnapshotV1 | WeaponParts | WeaponParts pose readback keyed by part identity. |
| 66 | copyWeaponPartDriveApplicationResultsV1 | WeaponParts | WeaponParts actual drive-application results, not publication success alone. |
| 67 | getScopeSightStateV1 | Weapon | Weapon scope/sight state and actual activation-source semantics. |
| 68 | getWeaponCompositionStateV1 | Weapon | Weapon composition identity/coverage metadata. |
| 69 | copyWeaponCompositionEntriesV1 | Weapon | Weapon bounded composition entries with sample identity. |
| 70 | getSelectedAuthoredGripPoseV1 | Weapon | Weapon authored grip/finger baselines; Hands owns generic presented-pose reads. |
| 71 | getPresentedHandPoseV1 | Hands | Hands presented hand/finger pose; Animation owns overrides. |
| 72 | copySemanticHandContactsV1 | Collision | Collision semantic hand contacts, including begin/continue/end and freshness. |
| 73 | copyPlayerColliderDescriptorsV1 | Collision | Collision generated-collider descriptors and participation flags. |
| 74 | getHandCollisionAvailabilityV1 | Collision | Collision per-hand availability/known-filter/enabled counts. |
| 75 | cancelInteractionCommandV1 | Grab | Grab cancellation with existing queued-versus-committed distinction. |
| 76 | getHandInputSuppressionStateV1 | Input | Input caller/effective suppression and lease observation. |
| 77 | acquireOffhandReservationV1 | Grab | Grab acquire offhand interaction reservation. |
| 78 | renewOffhandReservationV1 | Grab | Grab renew reservation. |
| 79 | releaseOffhandReservationV1 | Grab | Grab release reservation. |
| 80 | getOffhandReservationStateV1 | Grab | Grab reservation ownership/expiry observation. |
| 81 | clearNativeAnimationRuntimeV1 | Animation | Animation runtime-publication clear. |
| 82 | setTouchGrabTargetsForScopeV1 | Touch | Touch scoped target replacement. |
| 83 | clearTouchGrabTargetsForScopeV1 | Touch | Touch scoped clear and safe active-target retirement. |
| 84 | copyTouchGrabStatesForScopeV1 | Touch | Touch state snapshots, epochs, contact/coordinate data and release reasons. |
| 85 | requestTouchGrabYieldV1 | Touch | Touch asynchronous yield and restoration acknowledgement contract. |
| 86 | requestEquippedWeaponHandV1 | Retire | The old exact-hand function stays retired. For a new inventory draw, explicitly negotiate Weapon 1.1 and use `requestInventoryEquip`; its new contract does not revive the old slot. |
| 87 | queryWorldRaycastV1 | Collision | Collision bounded world raycast; preserve filtering and per-owner budget. |
| 88 | setColliderVisualizationOverrideV1 | Diagnostics | Diagnostics focus by immutable weapon/body identity, not a Weapon DTO. |
| 89 | clearColliderVisualizationOverrideV1 | Diagnostics | Diagnostics focus clear. |
| 90 | getLogicalInputActionStateV1 | Input | Input logical-action state and sample/press sequences. |
| 91 | getPlayerControllerStateV1 | PlayerController | PlayerController copied state; omit unsupported penetration flags. |
| 92 | requestPlayerControllerJumpV1 | PlayerController | PlayerController existing guarded native jump; do not turn it into a generic movement API. |
| 93 | getRawWandThumbstickV1 | Input | Input physical thumbstick sample with availability/freshness. |
| 94 | getNativeInputContextV1 | Input | Input native-action/menu/activation context with truthful naming. |
| 95 | getHandTargetDetailsV1 | Grab | Grab target details plus identity-based References composition; no embedded foreign DTO. |
| 96 | queryReferenceInteractionV1 | References | References native interaction query. |
| 97 | queryPowerArmorTargetV1 | References | References PA classification, linked-frame identity and armor-anchor observation. |
| 98 | requestPowerArmorGrabV1 | Grab | Grab PA-point command using a stable reference/anchor identity; no References query DTO embedded. |

Configuration is outside this 99-slot table:

| Current operation/export | Current destination | Disposition |
| --- | --- | --- |
| GetROCKConfigurationApi | Discovery → Configuration V1 | Removed; acquire Configuration through ROCKAPI_QueryInterfaceV1. |
| revision | Configuration V1 | Preserve no-I/O, any-thread revision observation. |
| visit | Configuration V1 | Serialized catalog traversal from a configuration task; strings are callback-borrowed. |
| setValue | Configuration V1 | Preserve case-insensitive key/owning-group identity, persistence and deferred reload. |
| ROCKAPI_GetDescriptorV1 | New discovery bootstrap | Retire old monolithic descriptor; never return a new-layout table through it. |
| ROCKAPI_GetProviderApi | New discovery bootstrap | Retire old table accessor. |
| ROCKAPI_GetApi | New discovery bootstrap | Retire the legacy alias. |


## Final presentation and hand roles

Use Hands 1.1 `getRoles` for physical firing/support assignments and WeaponParts 1.1 `querySourcePath` for hierarchy paths. Core `Complete` remains a control boundary; Core `Presented` supplies final same-frame hand observation after FRIK world final. PAPER observers needing that final pose use `PresentationComplete`, while `FrameComplete` retains control/lease semantics.

[Current callable index](../ApiIndex.md) · [First consumer](../GettingStarted.md).

## Opting into Weapon 1.1

Existing Weapon 1.0 binaries and source using `ROCK/Weapon.h` need no migration.
For explicit inventory equip into either physical hand, include
`ROCK/WeaponV1_1.h` and acquire `weapon::v1_1::Api` with Read + Write. Negotiate
minor 1 and the extended table size before touching the added slots. Use the
[new contract and both hand examples](WeaponV1_1.md), and handle a terminal
result instead of equipping again after queue admission. The released prefix,
record layouts, Core requirement and old calls remain unchanged.
