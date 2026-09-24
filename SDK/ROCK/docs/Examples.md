# Buildable ROCK examples

The SDK examples consume the modular API. Their shared F4SE bootstrap is teaching scaffolding; copy an example into your own plugin rather than depending on that scaffolding at runtime. Authority-changing examples must remain inert or explicitly gesture-gated until configured.

| Source under `SDK/ROCK/examples/mods/` | Purpose |
| --- | --- |
| `CapabilityReporter.cpp` | Reports independently negotiated families and owner permissions. |
| `HandStateMonitor.cpp` | Reads Grab occupancy and bounded lifecycle events. |
| `WeaponInspector.cpp` | Reads equipped identity, classification, emitters and evidence. |
| `WeaponCatalogDumper.cpp` | Observes weapon source/evidence catalogs. |
| `ForceGrabCommand.cpp` | Inert world-grab command until a target is explicitly configured. |
| `PowerArmorInteraction.cpp` | Inert PA query/admission/result/release lifecycle example. |
| `OffhandLease.cpp` | Acquires and explicitly releases a gesture-gated reservation. |
| `InputChordLease.cpp` | Coordinates a bounded input suppression lease. |
| `WeaponPartDriver.cpp` | Targets and drives a selected part with generation and cleanup guards. |
| `TouchMechanism.cpp` | Registers scoped fixed/hinge/prismatic mechanisms with explicit retirement. |
| `SurfaceClimber.cpp` | Demonstrates surface targets and release observation. |
| `ExternalContactSensor.cpp` | Registers a consumer body scope and polls contact observations. |
| `ContactVisualizer.cpp` | Observes contact data and publishes bounded diagnostics. |
| `ColliderFocus.cpp` | Publishes a leased collider visualization override. |
| `VisualHandOffset.cpp` | Publishes and clears narrow visual hand authority. |
| `AnimationObserver.cpp` | Observes native animation phases and authority state. |
| `MuzzleRayVisualizer.cpp` | Uses weapon emitter data, bounded Collision raycasts and Diagnostics. |

`MinimalProviderConsumer.cpp` is a compiled object fragment showing a Collision contact observer with explicit start/stop. `SDK/ROCK/modular_examples/ReadHands.cpp` demonstrates a small typed hand consumer. These fragments do not deploy DLLs.

## Build examples locally

From the RPS SDK checkout with its toolchain dependencies configured:

```powershell
cmake --preset custom-fast
cmake --build --preset custom-fast --config Release --target ROCKSDKExamplePlugins -- /m:1 /p:CL_MPCount=2
ctest --preset custom-fast -j 4
```

The example plugins use C++23 and CommonLibF4VR. Header-only consumer targets use C++20. Build success checks compilation, not in-game behavior; do not install every example into a play setup. Exercise your selected feature's admission, generation change, lease and shutdown cases in its intended runtime environment.

## Inventory equip into either hand

[`InventoryWeaponEquip.h`](../examples/InventoryWeaponEquip.h) is a reusable C++20 helper that explicitly negotiates Weapon 1.1 and exposes `drawRight`, `drawLeft`, `poll` and `stop`. [`InventoryWeaponEquip.cpp`](../examples/InventoryWeaponEquip.cpp) shows both gesture entry points. These live directly under `SDK/ROCK/examples/` and compile as the inert `ROCKSDKInventoryWeaponEquip` object target; they do not deploy a plugin or detect holsters.

Use the [inventory-equip contract](modular/WeaponV1_1.md) for exact stack selection, retained switching, result flags, cancellation and lifecycle cleanup.
