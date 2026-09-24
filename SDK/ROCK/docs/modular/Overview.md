# ROCK modular API

ROCK remains one DLL, one runtime, one owner registry and one scheduler. Its 13 public families are independently negotiated through `ROCKAPI_QueryInterfaceV1`. Current source is the active ROCK checkout; references to a separate `ROCK_v0.9` checkout are historical.

See the [complete interface map](../ApiIndex.md), [discovery and permissions](../DiscoveryAndCapabilities.md), [runtime contract](../RuntimeContract.md), [data model](../DataModel.md), [first consumer](../GettingStarted.md), and [migration](Migration.md).

Core owns registration, binding and lifecycle. Hands owns poses and physical firing/support roles; Grab owns effective occupancy and commands. Collision owns bodies, contacts and raycasts; Touch owns scoped mechanisms. Weapon and WeaponParts separate equipped observations from source/part interaction. Animation owns visual/native authority, Input owns controller observations and suppression, References owns copied native-reference observations, PlayerController owns bounded controller state and native jump admission, Diagnostics owns overlays, and Configuration owns the compiled catalog and persistence.

The seven task-thread snapshot reads are documented in the runtime contract. Live scene queries and mutations retain their owner-thread restrictions. FRIK FrameBegin establishes that thread; NativeGraphOutput cannot claim it. Core Presented supplies final same-frame hand observation after FRIK world final, while Complete remains an earlier control boundary.

All current families use major 1. Hands and WeaponParts use minor 1. [Weapon 1.1](WeaponV1_1.md) adds exact inventory equip into either hand, with the previous weapon retained in its carrying hand; the Weapon 1.0 prefix stays available. These versions are independent from FRIK API 2.3, the ROCK mod version, and the SDK package version.
