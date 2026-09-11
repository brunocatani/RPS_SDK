// Compile the actual example into its own translation unit so private state
// can be seeded for cleanup without adding test hooks to a shipped DLL.
#define definition WeaponPartDriverDefinition
#include "../mods/WeaponPartDriver.cpp"
#undef definition
#include "ExampleTestSupport.h"

bool testWeaponPartDriver()
{
    const auto& example = rock::sdk::example::WeaponPartDriverDefinition();
    if (!checkExample(!kEnableDemoMotion, "WeaponPartDriver", "distributed default must be inert")) return false;
    RockProviderApi api{};
    api.clearWeaponPartTargetsV1 = &cleared;
    api.clearWeaponPartDriveTargetsV1 = &cleared;
    RockProviderApi::inst = &api;
    bool ok = example.onStart(testOwner);
    RockProviderFrameSnapshot frame{};
    frame.lifecycleFlags = ~std::uint32_t{0};
    frame.weaponGenerationKey = 7;
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 0, "WeaponPartDriver", "inert callbacks acquire no authority");
    g_targetPublished = g_drivePublished = true;
    example.onStop(testOwner);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 2, "WeaponPartDriver", "stop clears each active lease exactly once");
    g_targetPublished = g_drivePublished = true;
    kEnableDemoMotion = true;
    frame.lifecycleFlags = 0;
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 4 && clearArgumentsValid,
        "WeaponPartDriver", "lifecycle loss clears authority for the correct owner");
    kEnableDemoMotion = false;
    RockProviderApi::inst = nullptr;
    return ok;
}
