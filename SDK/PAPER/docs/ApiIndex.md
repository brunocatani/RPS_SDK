# Complete API index

All PAPER Provider API V1 slots appear below in zero-based ABI order.

| Slot | Function | Purpose | Capability |
|---:|---|---|---|
| 0 | `getVersion` | Return provider ABI version. | — |
| 1 | `getModVersion` | Return provider diagnostic build number. | — |
| 2 | `isReady` | Report provider readiness. | — |
| 3 | `getRuntimeStateV1` | Copy coherent runtime state. | `RuntimeState` |
| 4 | `getConfigStateV1` | Copy published configuration and capture policy. | `RuntimeState` |
| 5 | `getCapturedTransformCountV1` | Read current native transform count. | `CapturedTransforms` |
| 6 | `copyCapturedTransformsV1` | Copy bounded native animation transforms. | `CapturedTransforms` |
| 7 | `getNativeHandPoseV1` | Copy hand-in-weapon and finger-local capture. | `NativeHandPose` |
| 8 | `registerConsumerV1` | Create a named owner and negotiate capabilities. | — |
| 9 | `unregisterConsumerV1` | Revoke owner callbacks, leases and authority. | Registered owner |
| 10 | `getGrantedCapabilitiesV1` | Read an owner's effective capability mask. | Registered owner |
| 11 | `setAnimationAuthorityV1` | Acquire or renew animation arbitration flags. | `AnimationAuthority` |
| 12 | `clearAnimationAuthorityV1` | Release caller animation authority. | `AnimationAuthority` |
| 13 | `registerEventCallbackV1` | Register an owner event callback. | `FrameCallbacks` |
| 14 | `unregisterEventCallbackV1` | Remove an owner event callback. | `FrameCallbacks` |
| 15 | `getReloadObservationLimitsV1` | Read reload limits and feature bits. | `ReloadObservations` |
| 16 | `getReloadCatalogStateV1` | Read reload catalog identity and counts. | `ReloadObservations` |
| 17 | `copyReloadCatalogNodesV1` | Copy hierarchy and baseline transform pages. | `ReloadObservations` |
| 18 | `copyReloadEvidenceV1` | Copy semantic reload evidence pages. | `ReloadObservations` |
| 19 | `copyReloadEvidencePointsV1` | Copy one evidence record's local geometry. | `ReloadEvidenceGeometry` |
| 20 | `getReloadFrameStateV1` | Read one coherent observation-frame state. | `ReloadObservations` |
| 21 | `copyReloadNodeObservationsV1` | Copy native-output/post-ROCK node observations. | `ReloadObservations` |
| 22 | `getReloadAnimationLimitsV1` | Read animation evidence limits and budget. | Animation read capability |
| 23 | `getReloadAnimationCatalogStateV1` | Read animation catalog/preharvest state. | Animation read capability |
| 24 | `getReloadAnimationLiveStateV1` | Read current live clip activity. | Animation read capability |
| 25 | `copyReloadAnimationClipsV1` | Copy clip records and provenance. | Animation read capability |
| 26 | `copyReloadAnimationTracksV1` | Copy one clip's transform tracks. | Animation read capability |
| 27 | `copyReloadAnimationSamplesV1` | Copy one track's bounded samples. | Animation read capability |
| 28 | `copyReloadAnimationAnnotationsV1` | Copy authored annotation markers. | Animation read capability |
| 29 | `copyReloadAnimationTriggersV1` | Copy event triggers. | Animation read capability |
| 30 | `copyReloadAnimationSkeletonV1` | Copy skeleton hierarchy/reference records. | Animation read capability |
| 31 | `getReloadStageStateV1` | Read aggregate reload-stage decisions. | `ReloadStageIdentification` |
| 32 | `copyReloadStagePartsV1` | Copy per-part stage evidence. | `ReloadStageIdentification` |
| 33 | `getNativePoseFrameStateV1` | Read resolved pose-pipeline frame state. | `NativePosePipeline` |
| 34 | `getNativeHandSolutionV1` | Read one hand's native/resolved/presented solution. | `NativePosePipeline` |
| 35 | `getWeaponMotionLimitsV1` | Read motion catalog limits. | `WeaponMotionCatalog` |
| 36 | `getWeaponMotionCatalogStateV1` | Read motion catalog identity and counts. | `WeaponMotionCatalog` |
| 37 | `copyWeaponMotionPartsV1` | Copy motion part records. | `WeaponMotionCatalog` |
| 38 | `copyWeaponMotionStagesV1` | Copy primary/return stage records. | `WeaponMotionCatalog` |
| 39 | `copyWeaponMotionStageKeysV1` | Copy a stage's normalized 24-key path. | `WeaponMotionCatalog` |
| 40 | `copyWeaponMotionFollowersV1` | Copy follower descriptors for one stage. | `WeaponMotionCatalog` |
| 41 | `copyWeaponMotionFollowerKeysV1` | Copy a follower's normalized path. | `WeaponMotionCatalog` |
| 42 | `getWeaponMotionLearningStateV1` | Read learning aggregate diagnostics. | `WeaponMotionDiagnostics` |
| 43 | `copyWeaponMotionRecordersV1` | Copy individual learning recorder states. | `WeaponMotionDiagnostics` |
| 44 | `getWeaponManipulationFrameStateV1` | Read manipulation aggregate state. | `WeaponManipulationTelemetry` |
| 45 | `getWeaponManipulationHandStateV1` | Read one hand's motion projection/progress. | `WeaponManipulationTelemetry` |
| 46 | `getWeaponMotionStoreStateV1` | Read compiled cache/store diagnostics. | `WeaponMotionDiagnostics` |
| 47 | `setDevelopmentCaptureV1` | Publish or renew bounded development scopes. | `DevelopmentCaptureControl` |
| 48 | `clearDevelopmentCaptureV1` | Clear caller development scopes. | `DevelopmentCaptureControl` |
| 49 | `getDevelopmentCaptureStateV1` | Read policy, scopes, lease and progress. | `DevelopmentCaptureControl` |

“Animation read capability” means either `ReloadAnimationTelemetry` or
`ReloadAnimationEvidence`. The former is passive. The latter expresses demand
for exact evidence, subject to the user's policy ceiling.

Normal consumers call `PaperApi::initialize`. It prefers
`PAPERAPI_GetProviderDescriptorV1` and uses `PAPERAPI_GetProviderApi` only for a
base-table legacy fallback.
