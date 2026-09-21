# ROCK modular SDK

ROCK exposes physical interaction through **13 independently negotiated interfaces**. It remains one DLL and one runtime. Your plugin registers one owner, binds only the families and permissions it needs, and reads copied state or issues bounded requests.

| Start here | What you get |
| --- | --- |
| [Install](docs/Install.md) | Explicit headers and CMake targets. |
| [First consumer](docs/GettingStarted.md) | A complete final-hand observer with explicit teardown. |
| [API index](docs/ApiIndex.md) | Every interface, current version, callable and public declaration. |
| [Migration](docs/modular/Migration.md) | Move a monolithic consumer to the current contracts. |
| [Runtime contract](docs/RuntimeContract.md) | Threads, callbacks, generations, leases and event streams. |
| [Examples](docs/Examples.md) | Buildable SDK consumers and their purpose. |

## Integration model

Resolve `ROCKAPI_QueryInterfaceV1` from the already-loaded `ROCK.dll`. `ROCK/Client.h` negotiates Core, registers your owner and acquires typed feature tables. The helper compiles into your plugin; it is not another runtime or an import library. Public calls execute inside ROCK.

Core owns lifecycle and scheduling. Hands owns poses and physical firing roles. Grab owns occupancy and commands. Collision owns bodies/contacts and raycasts. Touch owns mechanisms. Weapon and WeaponParts separate equipped state from part/source interaction. Animation owns pose authority; Input owns controller observation/suppression; References owns native-reference observation; PlayerController owns bounded controller state/jump access; Diagnostics owns overlays; Configuration owns the settings catalog and persistence.

All families currently use major 1. Hands and WeaponParts use minor 1; the rest use minor 0. This is an intentional compatibility break from the previous provider. The old discovery exports are not fallback entry points into the current runtime.

## Scope

The API reference follows current local source as checked on 20 September 2026. Published download versions may differ. Negotiate the required interface and table extent in the installed DLL; a release name or build success cannot establish runtime support. The modular interface versions are independent of FRIK's API 2.3 and of the ROCK mod version.
