#include "ExampleRuntime.h"

#include <cstdio>

namespace
{
    rock::api::QueryInterfaceV1 g_query{};
    bool connect(rock::api::Client&,rock::api::QueryInterfaceV1 query) noexcept {g_query=query;return query!=nullptr;}

    using rock::sdk::example::hasLifecycleFlag;



    bool start(const std::uint64_t) noexcept {
        for(std::uint32_t id=1;id<=13;++id) {
            const rock::api::InterfaceDescriptorV1* descriptor{};
            const auto status=g_query(static_cast<rock::api::InterfaceId>(id),1,0,sizeof(void*),&descriptor);
            char message[160]{};
            if(status==rock::api::Status::Ok && descriptor)
                std::snprintf(message,sizeof(message),"Interface=%u major=%u minor=%u bytes=%u",id,descriptor->major,descriptor->minor,descriptor->tableByteSize);
            else std::snprintf(message,sizeof(message),"Interface=%u unavailable status=%u",id,static_cast<unsigned>(status));
            rock::sdk::example::logInfo(message);
        }
        return true;
    }

    void stop(const std::uint64_t) noexcept {}

    void frame(
        const std::uint64_t,
        const rock::api::core::SnapshotV1&) noexcept
    {}
}

namespace rock::sdk::example
{
    const Definition& definition() noexcept
    {
        static const Definition value{
            .pluginName = "ROCKSDKCapabilityReporter",
            .pluginVersion = 1,
            .onConnect = &connect,
            .onStart = &start,
            .onStop = &stop,
            .onFrame = &frame,
        };
        return value;
    }
}
