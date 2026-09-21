# RPS UI SDK

The runtime is [released on Nexus Mods](https://www.nexusmods.com/fallout4/mods/108951).
For installation and PALM setup, see the [UI Framework guide](https://devartificial.pro/docs/rps-stack/ui-framework).

RPS UI Framework hosts world-space panels for independent Fallout 4 VR mods. It
owns stereo composition, scene-depth integration, pointer routing, input leases,
and physical panel resizing. A consumer supplies a render callback and owns its
UI implementation, including its own Dear ImGui context and DX11 backend.

Include `RPSUIFrameworkApi.h` through the header-only `RPS::UI` CMake target.
`RPS::SDK` includes ROCK, PAPER, and UI. These targets provide declarations;
`RPS_UI_Framework.dll` must be present in game. Its native controller tracking
and input service do not require ROCK or a skeleton provider.
The UI header also remains in the runtime repository at
`SDK/include/RPSUIFrameworkApi.h`; the SDK sync test requires byte equality.

| Guide | Contents |
| --- | --- |
| [Getting started](docs/GettingStarted.md) | Discovery, feature negotiation, panel registration, presentation, cleanup. |
| [Panel reference](docs/PublicApi.md) | Eight host methods and complete panel declarations. |
| [Input reference](docs/InputApi.md) | Six input/capture/calibration methods and their lifetime rules. |
| [Cooperation reference](docs/CooperationApi.md) | Four access/directory/retirement methods and safe teardown. |
| [Runtime contract](docs/RuntimeContract.md) | Render/control threads, pointer ownership, readiness, device lifetime, teardown. |
| [Dear ImGui integration](docs/ImGuiIntegration.md) | Consumer-owned context, backend and input mapping; no shared ImGui ABI. |
| `SDK/UI/examples/PanelConsumer.cpp` | Inert, compiled registration/presentation fragment with rollback and cleanup. |

The current source exposes the V1 panel, input and cooperation APIs. API existence, registration success,
framework readiness, and a panel actually being rendered are separate facts.
Release and input dependency information was checked on 2026-09-13. Runtime
rendering is not qualified by the SDK's compile and header-sync tests.
