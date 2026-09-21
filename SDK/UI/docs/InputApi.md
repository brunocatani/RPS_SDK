# Independent UI input API

Include `RPSUIInputApi.h` from `RPS::UI`. `RequestInputApiV1()` resolves the no-argument `RPSUI_RequestInputApi` export from the already-loaded `RPS_UI_Framework.dll` and validates version 1 plus the full table extent. This service requires neither ROCK nor a skeleton provider.

| Function | Contract |
| --- | --- |
| `subscribe` | Register a value-only input callback and return a subscription token; retain code/context. |
| `unsubscribe` | Stop a subscription; false means a callback remains in flight. Keep its context and retry later. |
| `capture` | Publish button/chord capture for this subscription from its game callback; renew every callback. |
| `rawInputReadActive` | Calling-thread signal for cooperating hooks to leave the UI's physical controller read raw. |
| `capturedButtons` | Lock-free query from an input thread using current left/right physical pressed levels; zero when gameplay/capture is unavailable. |
| `setPointerAim` | Game-thread shared aim calibration for both hands; applies next input frame to all panels. Persistence belongs to the settings host. |

## Input identity and capture

`InputFrameV1::hands[0]` is physical left and `[1]` physical right. These indices differ from ROCK's Hand enum. Check `ready`, per-hand `valid` and the input sequence. Buttons are levels; derive edges locally and clear them when availability is lost.

Capture expires after three callbacks without renewal. Supported button IDs are Menu/B 1, Grip 2, A 7, thumbstick click 32 and Trigger 33. An idle chord reserves members only while all are pressed; after claiming the gesture, keep button capture until every member is released. Capture does not transfer ROCK interaction ownership.

Pointer pitch accepts [-180, 180] degrees and yaw [-90, 90], relative to native forward; positive pitch aims up and positive yaw aims right. PALM applies its own calibrated baseline plus user adjustment, so its displayed zero adjustment is not a zero native pitch. Avoid competing settings writers.

Callbacks borrow the frame, must not block or retain it, and run on the game input callback thread. Unsubscribe without spinning, retain context while it returns false, and clean up independently of render delivery.


<details>
<summary>Complete public declarations: RPSUIInputApi.h</summary>

```cpp
#pragma once
#include "RPSUIFrameworkApi.h"

namespace rpsui::sdk {
// Value-only game-thread input. Physical hands are left=0, right=1. No skeleton
// provider is required. Callbacks must not block or retain the borrowed frame.
struct HandInputV1 {
 float position[3]{};
 float forward[3]{};
 std::uint64_t pressed{};
 float stick[2]{};
 bool valid{};
};
struct InputFrameV1 {
 std::uint32_t structSize{sizeof(InputFrameV1)};
 std::uint32_t version{1};
 std::uint64_t sequence{};
 double seconds{};
 HandInputV1 hands[2]{};
 bool ready{},leftHanded{};
};
using InputCallbackV1=void(RPSUI_CALL*)(const InputFrameV1*,void*) noexcept;
struct InputCaptureV1 {
 std::uint32_t structSize{sizeof(InputCaptureV1)};
 // Capture is game-callback-thread only; renew every callback. Expires after
 // three callbacks. Supported buttons: B/Menu (1), Grip (2), A (7),
 // thumbstick click (32), Trigger (33).
 std::uint64_t buttons[2]{};
 // An idle chord reserves its members only while the complete chord is down.
 // Once captured, publish buttons until all chord members have been released.
 std::uint64_t chord[2]{};
};
struct PointerAimV1 {
 std::uint32_t structSize{sizeof(PointerAimV1)};
 // Physical hands: left=0, right=1. Degrees relative to native controller
 // forward: positive pitch aims up, positive yaw aims right.
 // Pitch range [-180,180] permits consumer forward calibration plus adjustment;
 // yaw range [-90,90].
 float pitchDegrees[2]{},yawDegrees[2]{};
};
struct InputApiV1 {
 std::uint32_t structSize{sizeof(InputApiV1)},version{1};
 std::uint64_t(RPSUI_CALL* subscribe)(InputCallbackV1,void*) noexcept{};
 // False means a callback is in flight; caller retains callback/context and retries.
 bool(RPSUI_CALL* unsubscribe)(std::uint64_t) noexcept{};
 bool(RPSUI_CALL* capture)(std::uint64_t,const InputCaptureV1*) noexcept{};
 // Optional cooperation for other input hooks. True only on the calling
 // thread while the UI samples physical OpenVR state; leave that read raw.
 bool(RPSUI_CALL* rawInputReadActive)() noexcept{};
 // Read-only, lock-free query from any input thread. Physical hand: 0=left,
 // 1=right. Supply current physical button levels to resolve pending chords.
 // Returns zero outside gameplay or after capture has been released/expired.
 std::uint64_t(RPSUI_CALL* capturedButtons)(unsigned hand,std::uint64_t leftPressed,std::uint64_t rightPressed) noexcept{};
 // Game thread only. Sets shared UI aim; the settings host (PALM) owns
 // persistence. Changes apply on the next input frame to every UI panel.
 bool(RPSUI_CALL* setPointerAim)(const PointerAimV1*) noexcept{};
};
inline const InputApiV1* RequestInputApiV1() noexcept {
 const auto module=GetModuleHandleW(L"RPS_UI_Framework.dll");
 const auto function=module?GetProcAddress(module,"RPSUI_RequestInputApi"):nullptr;
 if(!function)return nullptr;
 const auto* api=reinterpret_cast<const InputApiV1*(RPSUI_CALL*)() noexcept>(function)();
 return api && api->version==1 && api->structSize>=sizeof(InputApiV1)?api:nullptr;
}
}
```

</details>
