# Complete ROCK API index

ROCK exposes **13 independently versioned interface families** through `ROCKAPI_QueryInterfaceV1`. Every family is currently major 1; Hands and WeaponParts are minor 1, all others minor 0. This is a compatibility break from the former monolithic provider. See the [migration guide](modular/Migration.md).

## Interface map

| ID | Interface | Version | Public header | CMake target | Calls |
| --- | --- | --- | --- | --- | --- |
| 1 | [Core](modular/Core.md) | 1.0 | `ROCK/Core.h` | `RPS::ROCKCore` | 12 |
| 2 | [Hands](modular/Hands.md) | 1.1 | `ROCK/Hands.h` | `RPS::ROCKHands` | 6 |
| 3 | [Collision](modular/Collision.md) | 1.0 | `ROCK/Collision.h` | `RPS::ROCKCollision` | 12 |
| 4 | [Grab](modular/Grab.md) | 1.0 | `ROCK/Grab.h` | `RPS::ROCKGrab` | 17 |
| 5 | [Touch](modular/Touch.md) | 1.0 | `ROCK/Touch.h` | `RPS::ROCKTouch` | 6 |
| 6 | [Weapon](modular/Weapon.md) | 1.0 | `ROCK/Weapon.h` | `RPS::ROCKWeapon` | 16 |
| 7 | [WeaponParts](modular/WeaponParts.md) | 1.1 | `ROCK/WeaponParts.h` | `RPS::ROCKWeaponParts` | 18 |
| 8 | [Animation](modular/Animation.md) | 1.0 | `ROCK/Animation.h` | `RPS::ROCKAnimation` | 9 |
| 9 | [Input](modular/Input.md) | 1.0 | `ROCK/Input.h` | `RPS::ROCKInput` | 10 |
| 10 | [References](modular/References.md) | 1.0 | `ROCK/References.h` | `RPS::ROCKReferences` | 3 |
| 11 | [PlayerController](modular/PlayerController.md) | 1.0 | `ROCK/PlayerController.h` | `RPS::ROCKPlayerController` | 3 |
| 12 | [Diagnostics](modular/Diagnostics.md) | 1.0 | `ROCK/Diagnostics.h` | `RPS::ROCKDiagnostics` | 6 |
| 13 | [Configuration](modular/Configuration.md) | 1.0 | `ROCK/Configuration.h` | `RPS::ROCKConfiguration` | 3 |

Each family reference includes every table member and the complete public declarations. [Shared types and results](modular/Abi.md) covers the common vocabulary. A repeated name such as `getSample` belongs to each table separately; there is no global slot numbering or global capability mask.

## Callable index

### Core

`getModVersion`, `isProviderReady`, `getFrameSnapshot`, `registerConsumerV1`, `unregisterConsumerV1`, `registerAnimationPhaseCallbackV1`, `unregisterAnimationPhaseCallbackV1`, `registerFrameCallbackForOwnerV1`, `unregisterFrameCallbackForOwnerV1`, `getSample`, `bindInterface`, `copyEvents`.

[Contract and signatures](modular/Core.md#entry-points).

### Hands

`getHandFrameV1`, `getPresentedHandFrameV1`, `getPresentedHandPoseV1`, `getSample`, `getHeadPose`, `getRoles`.

[Contract and signatures](modular/Hands.md#entry-points).

### Collision

`clearExternalBodies`, `getBodyContactSnapshotV1`, `registerExternalBodiesForScopeV1`, `clearExternalBodiesForScopeV1`, `copyExternalContactsSinceV1`, `copySemanticHandContactsV1`, `copyPlayerColliderDescriptorsV1`, `getHandCollisionAvailabilityV1`, `queryWorldRaycastV1`, `getSample`, `getEnvironment`, `copyEvents`.

[Contract and signatures](modular/Collision.md#entry-points).

### Grab

`requestForceGrabV1`, `getInteractionCommandResultV1`, `requestForceReleaseV1`, `requestThrownDropV1`, `getHandInteractionStateV1`, `cancelInteractionCommandV1`, `acquireOffhandReservationV1`, `renewOffhandReservationV1`, `releaseOffhandReservationV1`, `getOffhandReservationStateV1`, `getHandTargetDetailsV1`, `requestPowerArmorGrabV1`, `getSample`, `requestInventoryGrab`, `copyEvents`, `setEventCallback`, `clearEventCallback`.

[Contract and signatures](modular/Grab.md#entry-points).

### Touch

`setTouchGrabTargetsForScopeV1`, `clearTouchGrabTargetsForScopeV1`, `copyTouchGrabStatesForScopeV1`, `requestTouchGrabYieldV1`, `getSample`, `copyEvents`.

[Contract and signatures](modular/Touch.md#entry-points).

### Weapon

`getPrimaryHandV1`, `getOffhandHandV1`, `queryEquippedWeaponClassificationV1`, `getWeaponEmitterCountV1`, `copyWeaponEmittersV1`, `getEquippedWeaponGripStateV1`, `setEquippedWeaponHandlingAuthorityV1`, `clearEquippedWeaponHandlingAuthorityV1`, `getEquippedWeaponHandlingStateV1`, `getEquippedWeaponStateV1`, `getScopeSightStateV1`, `getWeaponCompositionStateV1`, `copyWeaponCompositionEntriesV1`, `getSelectedAuthoredGripPoseV1`, `getSample`, `copyEvents`.

[Contract and signatures](modular/Weapon.md#entry-points).

### WeaponParts

`queryWeaponContactAtPoint`, `getWeaponEvidenceDetailCountV1`, `copyWeaponEvidenceDetailsV1`, `getWeaponEvidenceDetailPointCountV1`, `copyWeaponEvidenceDetailPointsV1`, `setWeaponPartTargetsV1`, `clearWeaponPartTargetsV1`, `setWeaponPartDriveTargetsV1`, `clearWeaponPartDriveTargetsV1`, `getWeaponPartGripStateV1`, `queryWeaponPartTargetResolutionV1`, `copyWeaponPartPoseSnapshotV1`, `copyWeaponPartDriveApplicationResultsV1`, `getSample`, `copySources`, `copyEvents`, `querySourcePose`, `querySourcePath`.

[Contract and signatures](modular/WeaponParts.md#entry-points).

### Animation

`setNativeAnimationAuthorityV1`, `clearNativeAnimationAuthorityV1`, `getNativeAnimationAuthorityStateV1`, `setHandVisualAuthorityV1`, `clearHandVisualAuthorityV1`, `publishNativeAnimationRuntimeV1`, `clearNativeAnimationRuntimeV1`, `getSample`, `copyEvents`.

[Contract and signatures](modular/Animation.md#entry-points).

### Input

`setHandInputSuppressionV1`, `clearHandInputSuppressionV1`, `getRawWandButtonStateV1`, `isNativePipboyInputSuppressedV1`, `getHandInputSuppressionStateV1`, `getLogicalInputActionStateV1`, `getRawWandThumbstickV1`, `getNativeInputContextV1`, `getSample`, `copyEvents`.

[Contract and signatures](modular/Input.md#entry-points).

### References

`queryReferenceInteractionV1`, `queryPowerArmorTargetV1`, `getSample`.

[Contract and signatures](modular/References.md#entry-points).

### PlayerController

`getPlayerControllerStateV1`, `requestPlayerControllerJumpV1`, `getSample`.

[Contract and signatures](modular/PlayerController.md#entry-points).

### Diagnostics

`publishDebugOverlayV1`, `clearDebugOverlayV1`, `setColliderVisualizationOverrideV1`, `clearColliderVisualizationOverrideV1`, `getSample`, `copyEvents`.

[Contract and signatures](modular/Diagnostics.md#entry-points).

### Configuration

`revision`, `visit`, `setValue`.

[Contract and signatures](modular/Configuration.md#entry-points).
