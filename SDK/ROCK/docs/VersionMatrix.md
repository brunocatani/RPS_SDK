# ROCK interface versions

The table lists the contracts defined by the named base headers. Weapon also exposes the compatible 1.1 extension below; existing consumers may continue requesting its 1.0 prefix.

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


Negotiate exact major, minimum minor and the table byte extent independently for each family. Incompatible signatures, record layouts, units or ownership semantics require a new affected major. Append-only functions use the owning family's minor version. Do not extend existing V1 array element strides in place.

Hands 1.1 adds physical firing/support role readback; WeaponParts 1.1 adds hierarchy paths. Core Presented and the current synchronized snapshot-read guarantees use the current matching header/runtime contracts; FRIK API 2.3 and mod/package release numbers are separate version domains.

A legacy header or a matching major number alone cannot make a monolithic consumer compatible with the modular runtime. See [migration](modular/Migration.md).

Weapon additionally offers [minor 1.1](modular/WeaponV1_1.md) through `ROCK/WeaponV1_1.h`, preserving the complete 1.0 prefix, records and Core 1.0 requirement.

| Compatible extension | Public header | CMake target | Appended / total calls |
| --- | --- | --- | --- |
| [Weapon 1.1](modular/WeaponV1_1.md) | `ROCK/WeaponV1_1.h` | `RPS::ROCKWeapon` | 4 / 20 |
