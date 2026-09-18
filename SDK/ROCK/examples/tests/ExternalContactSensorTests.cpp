// Compile the actual example into its own translation unit so private state
// can be seeded for cleanup without adding test hooks to a shipped DLL.
#define definition ExternalContactSensorDefinition
#include "../mods/ExternalContactSensor.cpp"
#undef definition
#include "ExampleTestSupport.h"

bool testExternalContactSensor()
{
    const auto& example = rock::sdk::example::ExternalContactSensorDefinition();
    if (!checkExample(!kEnableSensor, "ExternalContactSensor", "distributed default must be inert")) return false;
    rock::api::collision::ApiV1 collisionApi{};
    collisionApi.clearExternalBodiesForScopeV1 = [](std::uint64_t owner, std::uint64_t scope) noexcept { clearArgumentsValid &= scope == kScopeToken; return cleared(owner); };
    g_collision=&collisionApi;
    bool ok = example.onStart(testOwner);
    rock::api::core::SnapshotV1 frame{};
    frame.lifecycleFlags = ~std::uint32_t{0};
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 0, "ExternalContactSensor", "inert callbacks acquire no authority");
    g_registered = true;
    example.onStop(testOwner);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 1, "ExternalContactSensor", "stop clears each active lease exactly once");
    g_registered = true;
    kEnableSensor = true;
    frame.lifecycleFlags = 0;
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 2 && clearArgumentsValid,
        "ExternalContactSensor", "lifecycle loss clears authority for the correct owner");
    kEnableSensor = false;
    g_collision=nullptr;
    return ok;
}
