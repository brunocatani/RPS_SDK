# RPS UI SDK

RPS UI Framework hosts world-space panels for independent Fallout 4 VR mods. It
owns stereo composition, scene-depth integration, pointer routing, input leases,
and physical panel resizing. A consumer supplies a render callback and owns its
UI implementation, including its own Dear ImGui context and DX11 backend.

Include `RPSUIFrameworkApi.h` through the header-only `RPS::UI` CMake target.
`RPS::SDK` includes ROCK, PAPER, and UI. These targets provide declarations;
`RPS_UI_Framework.dll` and its ROCK runtime dependency must be present in game.
The UI header also remains in the runtime repository at
`SDK/include/RPSUIFrameworkApi.h`; the SDK sync test requires byte equality.

| Guide | Contents |
| --- | --- |
| [Getting started](docs/GettingStarted.md) | Discovery, feature negotiation, panel registration, presentation, cleanup. |
| [API reference](docs/PublicApi.md) | All eight functions, values, dimensions, flags, results, compatibility limits. |
| [Runtime contract](docs/RuntimeContract.md) | Render/control threads, pointer ownership, readiness, device lifetime, teardown. |
| [Dear ImGui integration](docs/ImGuiIntegration.md) | Consumer-owned context, backend and input mapping; no shared ImGui ABI. |
| [Example](examples/PanelConsumer.cpp) | Inert, compiled registration/presentation fragment with rollback and cleanup. |

This is V1 of a pre-release provider. API existence, registration success,
framework readiness, and a panel actually being rendered are separate facts.
The documented source snapshot is 2026-09-11; runtime rendering is not qualified
by the SDK's compile and header-sync tests.
