# Equipped-weapon handling policy

Bind Weapon Read + Write and use `setEquippedWeaponHandlingAuthorityV1`, `getEquippedWeaponHandlingStateV1` and `clearEquippedWeaponHandlingAuthorityV1`. Handling is an owner-bound singleton lease; conflicting ownership returns `OwnerConflict`. Read Grab for actual occupancy, Hands for physical firing/support roles, and Weapon for captured grips and equipped identity.

## Supported flags

| Flag | Behavior |
| --- | --- |
| `FiringGripOwnership` | Physical firing-grip ownership policy. |
| `PrimaryDetach` | Detach the current firing hand under valid ownership. |
| `AmbidextrousHandoff` | Permit physical transfer to the opposite firing hand. |
| `GripZoneEquip` | Equip/claim through the physical firing-grip zone. |
| `GripZoneHoverHaptics` | Bounded equip-zone hover feedback policy. |
| `FiringGripProximitySupport` | Leased firing-grip proximity support tuning. |

These are the six implemented bits. Primary detach, handoff and grip-zone equip depend on firing-grip ownership. Retired shoulder-stash, Pip-Boy hand-equip and visual-bridge enable flags are absent from the current enum. Bridge correctness stays owned by ROCK. There is no exact-hand equip request; physical handoff changes firing roles without changing controller identities.

Publish a nonzero bounded lease with finite tuning and current guards. Renew only while the activity owns the feature. Explicit clear, expiry, generation loss, owner loss and fault cleanup retire the policy. Inspect live state rather than treating a historical accepted publication as current authority.

## Tuning ranges


Out-of-range or non-finite values reject the entire transaction.

| Field | Valid range | Unit / purpose |
|---|---:|---|
| `gripZoneEquipRadiusGameUnits` | 0.25–30.0 | Equip-zone radius |
| `gripZoneEquipSettleSeconds` | 0.0–5.0 | Time inside the zone before equip |
| `firingGripReattachRadiusGameUnits` | 0.25–30.0 | Reattach distance |
| `gripZoneHoverHapticIntensity` | 0.0–1.0 | Hover feedback strength |
| `firingGripProximitySupportRadiusGameUnits` | 0.25–30.0 | Visual support proximity |
| `weaponGripHapticDurationSeconds` | 0.01–0.50 | Grip pulse duration |
| `firingGripAttachHapticIntensity` | 0.0–1.0 | Attach feedback strength |
| `firingGripDetachHapticIntensity` | 0.0–1.0 | Detach feedback strength |
| `supportGripHapticIntensity` | 0.0–1.0 | Support-grip feedback strength |
| `firingGripPromotionRadiusGameUnits` | 0.25–30.0 | Lateral handoff-cylinder reach |
| `leftFiringAimYawDegrees` | -30.0–30.0 | Left-firing aim correction |
| `leftFiringAimPitchDegrees` | -30.0–30.0 | Left-firing aim correction |
| each `leftFiringAimOffsetGameUnits` axis | -15.0–15.0 | Left-firing position correction |
| `equipVisualBridgeTimeoutSeconds` | 0.25–5.0 | Native render recovery timeout |
| `equipVisualBridgeBlendSeconds` | 0.0–1.0 | Visual handoff blend |


Exact request defaults and fields are in the [Weapon declarations](Weapon.md#types-flags-bounds-and-signatures). Initialize the request with `{}` and set a valid flag/lease explicitly; a default zero lease is not an active publication.
