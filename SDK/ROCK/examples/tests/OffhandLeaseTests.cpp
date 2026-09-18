// Compile the actual example into its own translation unit so private state
// can be seeded for cleanup without adding test hooks to a shipped DLL.
#define definition OffhandLeaseDefinition
#include "../mods/OffhandLease.cpp"
#undef definition
#include "ExampleTestSupport.h"

bool testOffhandLease()
{
    const auto& example = rock::sdk::example::OffhandLeaseDefinition();
    if (!checkExample(!kEnableReservation, "OffhandLease", "distributed default must be inert")) return false;
    rock::api::grab::ApiV1 grabApi{};
    grabApi.releaseOffhandReservationV1 = &cleared;
    g_grab=&grabApi;
    bool ok = example.onStart(testOwner);
    rock::api::core::SnapshotV1 frame{};
    frame.lifecycleFlags = ~std::uint32_t{0};
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 0, "OffhandLease", "inert callbacks acquire no authority");
    g_leaseActive = true;
    example.onStop(testOwner);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 1, "OffhandLease", "stop clears each active lease exactly once");
    g_leaseActive = true;
    kEnableReservation = true;
    frame.lifecycleFlags = 0;
    example.onFrame(testOwner, frame);
    example.onStop(testOwner);
    ok &= checkExample(clearCalls == 2 && clearArgumentsValid,
        "OffhandLease", "lifecycle loss clears authority for the correct owner");
    kEnableReservation = false;
    g_grab=nullptr;
    return ok;
}
