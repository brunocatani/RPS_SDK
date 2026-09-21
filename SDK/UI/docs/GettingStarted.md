# First RPS UI consumer

## Build and discovery

```cmake
find_package(RPS_SDK CONFIG REQUIRED)
target_link_libraries(MyPanelPlugin PRIVATE RPS::UI)
```

Use Windows x64 and include `RPSUIFrameworkApi.h`. This module alone requires
C++17. A complete F4SE plugin may require a newer language level through its own
dependencies. Linking the target neither links a host DLL import library nor
installs the UI runtime or Dear ImGui. ROCK is not a requirement for the UI host.

`rpsui::sdk::RequestApiV1()` finds an already loaded
`RPS_UI_Framework.dll`, resolves `RPSUI_RequestApi`, requests version 1, and
checks the table's size, version, and flavor. It returns null when discovery or
validation fails. It does not load the DLL. Retry from an appropriate consumer
lifecycle point; a missing provider should leave the feature inactive.

Check the function pointers you use and `isFrameworkReady()`. Register once with
a stable, globally unique `consumerId`, a display name, and only the feature
bits you need. Both strings must be nonempty and NUL-terminated within their
fixed buffers. Save `ownerToken`; check `grantedFeatures` before proceeding.
Registration itself can succeed before rendering is ready.

## Register and present a panel

Register `PanelRegistrationV1` with a globally unique `panelId`, display name,
valid pixel dimensions, physical width limits, and a non-null `noexcept`
callback. The host copies registration metadata but borrows callback code and
`userData`. A registered panel starts closed.

The compiled `SDK/UI/examples/PanelConsumer.cpp` fragment provides `start`, `present`, `state`, `neighbors`, and `stop`. It discovers both the base and Cooperation tables and registers a cooperative panel. Supply stable callback code/data, serialize control calls, and explicitly open after your own gesture. Keep the data alive until `stop()` returns `Ok`; retry `CallbackBusy` on a later control tick. The fragment has no F4SE entry point and performs no automatic game action.

`PanelPoseV1` describes a world-space panel in game units: center, unit right/up/
front basis, and physical width/height. The defaults describe a valid basis,
not a useful player-relative location. Derive placement from current provider
data and retain no engine pointers. Preserve the registered pixel aspect ratio.

Submit `PanelPresentationV1` on open, deliberate reposition, or close. Zero
`sequence` asks the host to advance its sequence; explicit nonzero sequences
must strictly increase. An open submission replaces the host pose and can
cancel an active resize. Repeatedly resubmitting your default pose would undo
user resizing. Read `getPanelState` for the host's resolved size and position;
the host may separate overlapping panels. `resetPanelSize` restores registration
width and its matching aspect ratio.

## Rendering and closing

The callback receives a host-owned D3D11 device, immediate context, render target,
panel dimensions and routed input. Draw only your panel content into that target.
The host later projects it into the two eyes. See [ImGui integration](ImGuiIntegration.md).

Close by submitting `open = 0`; keep registration if you will reopen. Close and
cleanup must run independently of render delivery: depth or projection failure
can prevent callbacks entirely. Clear your own input edges and pending actions
when your gameplay lifecycle becomes unavailable.

For permanent logical teardown, unregister the panel or unregister the consumer
to remove all its panels. Unregistration is not a wait for in-flight callbacks.
Keep callback code and `userData` alive until rendering is externally quiescent;
For a nonblocking callback-drain handshake, negotiate the separate [Cooperation API](CooperationApi.md) and use safe unregister; retain context while it returns `CallbackBusy`. This retires UI callback state, not every plugin subsystem. See [the lifetime contract](RuntimeContract.md).
