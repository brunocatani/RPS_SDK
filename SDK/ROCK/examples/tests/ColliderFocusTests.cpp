// Compile the actual example into its own translation unit so private state
// can be seeded for cleanup without adding test hooks to a shipped DLL.
#define definition ColliderFocusDefinition
#include "../mods/ColliderFocus.cpp"
#undef definition
#include "ExampleTestSupport.h"

bool testColliderFocus()
{
    const auto& example = rock::sdk::example::ColliderFocusDefinition();
    if (!checkExample(!kEnableFocus, "ColliderFocus", "distributed default must be inert")) return false;
    rock::api::diagnostics::ApiV1 diagnosticsApi{};
    diagnosticsApi.clearColliderVisualizationOverrideV1 = &cleared;
    g_diagnostics=&diagnosticsApi;
    bool ok = example.onStart(testOwner);
    rock::api::core::SnapshotV1 frame{};
    frame.lifecycleFlags = ~std::uint32_t{0};
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 0, "ColliderFocus", "inert callbacks acquire no authority");
    g_published = true;
    example.onStop(testOwner);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 1, "ColliderFocus", "stop clears each active lease exactly once");
    g_published = true;
    kEnableFocus = true;
    frame.lifecycleFlags = 0;
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 2 && clearArgumentsValid,
        "ColliderFocus", "lifecycle loss clears authority for the correct owner");
    kEnableFocus = false;
    g_diagnostics=nullptr;
    return ok;
}
