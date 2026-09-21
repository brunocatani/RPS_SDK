# RPS UI runtime contract

The host owns native controller tracking, OpenVR input, gameplay availability,
and UI input capture. Neither ROCK nor a skeleton provider is required. Your
consumer can use `RPS::UI` alone for its panel ABI. RPS UI is a separate plugin from the static
`RPS::Addresses`/`RPS::Runtime` engine-binding library.

## Control, render and game work

Registration and presentation enter a synchronized host registry. Render
callbacks run later, outside that registry lock, on the renderer path. The
consumer must synchronize its own data, serialize its control lifecycle, and
marshal game mutations to an appropriate game task or provider callback.
Avoid blocking I/O, inventory enumeration, config writes, and heavy allocation
inside rendering or input callbacks. Publish bounded value snapshots for the UI.

The host snapshots render work and gates callback admission. Legacy noncooperative base unregister is not a drain guarantee: keep callback code/data alive until its in-flight work is quiescent. The separate [Cooperation API](CooperationApi.md) provides safe unregister operations. They close new admission immediately and return `CallbackBusy` until active callbacks finish; retry later and release data only after `Ok`. Cooperative panels preserve this requirement even through base unregister. A destructor cannot safely discard callback data after an unsuccessful retirement.

Callbacks are `noexcept`. Handle failures internally; an exception escaping a
`noexcept` callback can terminate the process despite a host-side catch.

## Readiness and skipped frames

The host's readiness check covers renderer initialization and installation of
its native input service. Gameplay input additionally requires a ready game
session, unblocked menus, OpenVR, the player, and valid native wand tracking.
Captured scene depth and stereo
projection must be valid before consumer drawing. Failure there skips panel
render callbacks and composition; there is no always-visible overlay fallback.

`submitPanelPresentation == Ok` acknowledges stored state. `getPanelState.open`
reports that state, not proof of a displayed pixel. No public health API reports
the last successful composition, depth rejection, or projection validity.
Implement close/cancel and input release without waiting for another draw.

## Routed interaction

The host resolves pointer hits and the physical hand centrally. It gates click
ownership and captures native controller buttons while interacting.
Consumers should use the supplied pointer/button levels instead of adding an
independent hook. Reset local edge tracking when pointer validity or gameplay
availability is lost. A pressed gameplay trigger must not become a fresh UI
click merely because its ray enters a panel.

Configuration panels opt in to `ConfigNavigation`: stick and grip-back come
from the pointing hand. Hold-based circular wheels use their own opening/release
gesture through the independent input API and leave that flag unset. RPS UI does not define which
gameplay action opens a third-party panel or owns the consumer's gesture state.

`RPSUIInputApi.h` in the runtime repository declares the separate
`RPSUI_RequestInputApi` export. Its subscriptions deliver borrowed, value-only
`InputFrameV1` data on the game thread, with physical left/right hands and a
readiness flag. Capture requests are game-callback-thread only and must be
renewed every callback; stale capture expires automatically. An idle chord
reserves buttons only while the full combination is down. Once claimed, the
consumer retains capture until every chord button is released.

An input `unsubscribe` that returns false has a callback in flight: retain
the callback and context and retry. This input API handshake is separate from
the panel V1 unregister lifetime rule above. The panel ABI remains eight
functions; input and cooperation exports are separate runtime-owned APIs.

The host owns physical edge/corner resizing and separation of overlapping
panels. Preserve the resolved state when repositioning; do not continually
overwrite it with registration defaults. A fixed-size circular panel is still
drawn by the consumer into a rectangular texture with alpha.

## D3D and lifetime

Frame D3D pointers and frame storage are borrowed. Do not retain the RTV or frame
pointer after callback return. If your backend retains a device/context, retain
the necessary COM references explicitly and recreate its device resources when
`deviceGeneration` changes. The host owns the target, clearing, viewport,
stereo projection and outer D3D state restoration. Consumers own their ImGui
context, renderer backend resources, fonts, widget state and application data.
Prepare font/file data outside render callbacks.

## Cooperative retirement

The separate [Cooperation API](CooperationApi.md) supplies nonblocking safe panel/consumer unregister. `CallbackBusy` closes new admission while a callback drains; retry on a later control tick and free userdata only after `Ok`. Cooperative panels cannot bypass this rule through base unregister. Legacy noncooperative removal retains the older no-drain behavior. The independent input subscription has its own unsubscribe handshake; retire both when a consumer owns both services.
