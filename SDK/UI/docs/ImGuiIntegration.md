# Dear ImGui in an RPS UI panel

RPS UI is a panel render host. It does not export Dear ImGui functions, share an
`ImGuiContext`, or impose an ImGui ABI on consumers. Build Dear ImGui and its
DX11 renderer backend in your own mod. A non-ImGui D3D11 renderer can implement
the same callback contract.

For each independently managed consumer renderer:

1. Prepare font bytes and persistent application data outside the render path.
2. On the render thread, create your own ImGui context and initialize
   `ImGui_ImplDX11_Init` with the callback device/context. Set `IniFilename` and
   `LogFilename` to null if the panel should not perform implicit ImGui file I/O.
3. Before every ImGui call, select your context. Preserve/restore the previous
   context if your integration shares an ImGui library instance with other code.
4. Set display size to the panel's pixel dimensions and framebuffer scale to 1.
   Supply finite positive delta time from the render frame.
5. Add pointer position/button and scroll input, call the backend's `NewFrame`,
   `ImGui::NewFrame`, draw widgets, then `ImGui::Render` and
   `ImGui_ImplDX11_RenderDrawData` into the provided target.
6. Rebuild backend/device resources on `deviceGeneration` change. Perform final
   backend/context destruction only after callback lifetime is safe.

Inside an initialized renderer, input mapping follows this pattern:

```cpp
// frame is callback-borrowed; io belongs to this consumer's selected context.
io.DisplaySize = ImVec2(float(frame->pixelWidth), float(frame->pixelHeight));
io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
io.DeltaTime = frame->deltaSeconds;
if (frame->pointerValid) {
    io.AddMousePosEvent(frame->pointerPixelX, frame->pointerPixelY);
    io.AddMouseButtonEvent(0, frame->primaryDown != 0);
    constexpr float scrollUnitsPerSecond = 5.5f; // Consumer UX choice.
    io.AddMouseWheelEvent(frame->scrollAxisX * scrollUnitsPerSecond * io.DeltaTime,
                         frame->scrollAxisY * scrollUnitsPerSecond * io.DeltaTime);
} else {
    io.AddMousePosEvent(-FLT_MAX, -FLT_MAX); // <cfloat>
    io.AddMouseButtonEvent(0, false);
}
const bool backDown = frame->pointerValid && frame->backDown;
const bool backPressed = backDown && !backWasDown;
backWasDown = backDown; // Render-thread-owned state; clear on close/lifecycle loss.
```

This is an integration fragment, not a second complete example plugin. The
SDK's compiled example covers the host registration/presentation calls without
requiring ImGui as a dependency. `backPressed` should enqueue a bounded UI action
or update local navigation; game/config writes belong on the game task thread.
Use `ConfigNavigation` only when the requested feature grant includes it.

For a radial wheel, combine `Transparent`, `FixedSize`, and `CircularInput`,
draw the circle yourself with alpha outside it, and omit `ConfigNavigation`.
The opening hold and release-to-select gesture remain the consumer's policy.

The current host has no keyboard/text-input API, shared font/texture registry,
modal arbitration API, application settings schema, or callback-drain API.
Treat these as absent capabilities. Do not describe them as supported because
Dear ImGui itself can draw the corresponding widgets.
