// Compile the actual example into its own translation unit so private state
// can be seeded for cleanup without adding test hooks to a shipped DLL.
#define definition VisualHandOffsetDefinition
#include "../mods/VisualHandOffset.cpp"
#undef definition
#include "ExampleTestSupport.h"

bool testVisualHandOffset()
{
    const auto& example = rock::sdk::example::VisualHandOffsetDefinition();
    if (!checkExample(!kEnableVisualOffset, "VisualHandOffset", "distributed default must be inert")) return false;
    rock::api::animation::ApiV1 animationApi{};
    animationApi.clearHandVisualAuthorityV1 = [](std::uint64_t owner, rock::api::Hand hand) noexcept { clearArgumentsValid &= hand == rock::api::Hand::Right; return cleared(owner); };
    g_animation=&animationApi;
    bool ok = example.onStart(testOwner);
    rock::api::core::SnapshotV1 frame{};
    frame.lifecycleFlags = ~std::uint32_t{0};
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 0, "VisualHandOffset", "inert callbacks acquire no authority");
    g_published = true;
    example.onStop(testOwner);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 1, "VisualHandOffset", "stop clears each active lease exactly once");
    g_published = true;
    kEnableVisualOffset = true;
    frame.lifecycleFlags = 0;
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 2 && clearArgumentsValid,
        "VisualHandOffset", "lifecycle loss clears authority for the correct owner");
    kEnableVisualOffset = false;
    g_animation=nullptr;
    return ok;
}
