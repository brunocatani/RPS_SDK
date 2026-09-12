# RPS UI runtime contract

The host uses ROCK for current hand frames, gameplay lifecycle and suppression
leases. Your consumer can use `RPS::UI` alone for its panel ABI; the running host
still requires ROCK. RPS UI is a separate plugin from the static
`RPS::Addresses`/`RPS::Runtime` engine-binding library.

## Control, render and game work

Registration and presentation enter a synchronized host registry. Render
callbacks run later, outside that registry lock, on the renderer path. The
consumer must synchronize its own data, serialize its control lifecycle, and
marshal game mutations to an appropriate game task or provider callback.
Avoid blocking I/O, inventory enumeration, config writes, and heavy allocation
inside rendering or ROCK callbacks. Publish bounded value snapshots for the UI.

The host takes snapshots of callback pointers and `userData`. Unregistering
removes future registry entries; an already captured render snapshot may still
invoke the callback. Unregister is not a join. V1 has no unregister-and-drain
operation or cancellation acknowledgement. Keep callback code/data alive for
the process or arrange external render quiescence before destroying them. An
ordinary RAII destructor that unregisters and immediately destroys callback
data cannot establish that guarantee on its own.

Callbacks are `noexcept`. Handle failures internally; an exception escaping a
`noexcept` callback can terminate the process despite a host-side catch.

## Readiness and skipped frames

The host's readiness check covers renderer initialization and ROCK callback
registration. Gameplay input additionally requires an available world, skeleton,
provider, and unblocked menu/config state. Captured scene depth and stereo
projection must be valid before consumer drawing. Failure there skips panel
render callbacks and composition; there is no always-visible overlay fallback.

`submitPanelPresentation == Ok` acknowledges stored state. `getPanelState.open`
reports that state, not proof of a displayed pixel. No public health API reports
the last successful composition, depth rejection, or projection validity.
Implement close/cancel and input release without waiting for another draw.

## Routed interaction

The host resolves pointer hits and the physical hand centrally. It gates click
ownership and renews short ROCK input-suppression leases while interacting.
Consumers should use the supplied pointer/button levels instead of adding an
independent hook. Reset local edge tracking when pointer validity or gameplay
availability is lost. A pressed gameplay trigger must not become a fresh UI
click merely because its ray enters a panel.

Configuration panels opt in to `ConfigNavigation`: stick and grip-back come
from the pointing hand. Hold-based circular wheels use their own opening/release
gesture through ROCK and leave that flag unset. RPS UI does not define which
gameplay action opens a third-party panel or owns the consumer's gesture state.

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
