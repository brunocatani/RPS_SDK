#include <ROCK/Grab.h>
#include "ExampleRuntime.h"

#include <cstdio>

namespace
{
    const rock::api::grab::ApiV1* g_grab{};
    bool connect(rock::api::Client& client,rock::api::QueryInterfaceV1) noexcept {
        return client.acquire(3,g_grab)==rock::api::Status::Ok;
    }

    using rock::sdk::example::hasLifecycleFlag;

    // Replace this constant with a real activity predicate in your mod.
    bool kEnableReservation = false;

    bool g_leaseActive{ false };
    std::uint64_t g_lastExpiry{ ~std::uint64_t{ 0 } };


    bool start(const std::uint64_t) noexcept
    {
        g_leaseActive = false;
        g_lastExpiry = ~std::uint64_t{ 0 };
        if (!kEnableReservation) {
            rock::sdk::example::logInfo(
                "Offhand reservation is opt-in; bind it to your activity state before enabling");
        }
        return true;
    }

    void release(const std::uint64_t ownerToken) noexcept
    {
        if (g_leaseActive) {
            (void)g_grab->releaseOffhandReservationV1(ownerToken);
            g_leaseActive = false;
        }
    }

    void stop(const std::uint64_t ownerToken) noexcept
    {
        release(ownerToken);
    }

    void frame(
        const std::uint64_t ownerToken,
        const rock::api::core::SnapshotV1& snapshot) noexcept
    {
        if (!kEnableReservation ||
            !hasLifecycleFlag(
                snapshot.lifecycleFlags,
                rock::api::core::LifecycleFlag::PhysicsWriteAllowed)) {
            release(ownerToken);
            return;
        }

        rock::api::grab::OffhandReservationRequestV1 request{};
        request.reservation = rock::api::grab::OffhandReservation::ReloadReserved;
        request.leaseFrames = 2;
        request.worldGeneration = snapshot.worldGeneration;
        request.skeletonGeneration = snapshot.skeletonGeneration;
        request.providerGeneration = snapshot.providerGeneration;
        const auto result = g_leaseActive ?
            g_grab->renewOffhandReservationV1(ownerToken, &request) :
            g_grab->acquireOffhandReservationV1(ownerToken, &request);
        g_leaseActive = result == rock::api::Status::Ok;
        if (!g_leaseActive) {
            return;
        }

        rock::api::grab::OffhandReservationStateV1 state{};
        if (g_grab->getOffhandReservationStateV1(
                ownerToken,
                &state) == rock::api::Status::Ok &&
            state.expiresAfterFrame != g_lastExpiry) {
            g_lastExpiry = state.expiresAfterFrame;
            char message[160]{};
            std::snprintf(
                message,
                sizeof(message),
                "Offhand reservation=%u active=%u remaining=%u",
                static_cast<std::uint32_t>(state.reservation),
                state.active,
                state.remainingFrames);
            rock::sdk::example::logInfo(message);
        }
    }
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKOffhandLease",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
