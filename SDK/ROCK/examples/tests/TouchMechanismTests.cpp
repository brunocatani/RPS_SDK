// Compile the actual example into its own translation unit so private state
// can be seeded for cleanup without adding test hooks to a shipped DLL.
#define definition TouchMechanismDefinition
#include "../mods/TouchMechanism.cpp"
#undef definition
#include "ExampleTestSupport.h"

bool testTouchMechanism()
{
    const auto& example = rock::sdk::example::TouchMechanismDefinition();
    if (!checkExample(!kEnableMechanism, "TouchMechanism", "distributed default must be inert")) return false;
    rock::api::touch::ApiV1 touchApi{};
    touchApi.clearTouchGrabTargetsForScopeV1 = [](std::uint64_t owner, std::uint64_t scope) noexcept { clearArgumentsValid &= scope == kScopeToken; return cleared(owner); };
    g_touch=&touchApi;
    bool ok = example.onStart(testOwner);
    rock::api::core::SnapshotV1 frame{};
    frame.lifecycleFlags = ~std::uint32_t{0};
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 0, "TouchMechanism", "inert callbacks acquire no authority");
    g_published = true;
    example.onStop(testOwner);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 1, "TouchMechanism", "stop clears each active lease exactly once");
    g_published = true;
    kEnableMechanism = true;
    frame.lifecycleFlags = 0;
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 2 && clearArgumentsValid,
        "TouchMechanism", "lifecycle loss clears authority for the correct owner");
    kEnableMechanism = false;
    g_touch=nullptr;
    return ok;
}
