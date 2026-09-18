#include <ROCK/Client.h>
#include <ROCK/Collision.h>
#include <array>
namespace {
rock::api::Client client;
const rock::api::core::ApiV1* core{};
const rock::api::collision::ApiV1* collision{};
std::uint64_t callback{}, cursor{};
void ROCK_CALL frame(const rock::api::core::SnapshotV1* snapshot,void*) {
    if(!snapshot || !snapshot->providerReady)return;
    std::array<rock::api::collision::ExternalContactRecordV1,16> contacts{};
    rock::api::collision::ExternalContactStreamStateV1 stream{};
    if(collision->copyExternalContactsSinceV1(client.owner(),0,cursor,contacts.data(),static_cast<std::uint32_t>(contacts.size()),&stream)!=rock::api::Status::Ok)return;
    // Process copied contacts here. Inspect GapBeforeFirstCopied to detect ring loss.
    if(stream.copiedCount)cursor=stream.lastCopiedSequence;
}
}
// Pass ROCKAPI_QueryInterfaceV1 resolved from the already loaded ROCK.dll.
bool StartRockConsumer(rock::api::QueryInterfaceV1 query) noexcept {
    using rock::api::Status;
    if(client.connect(query,"MinimalProviderConsumer")!=Status::Ok)return false;
    if(client.acquire(5,core)!=Status::Ok || client.acquire(1,collision)!=Status::Ok ||
       core->registerFrameCallbackForOwnerV1(client.owner(),frame,nullptr,&callback)!=Status::Ok) {
        (void)client.close();return false;
    }
    return true;
}
bool StopRockConsumer() noexcept {
    if(client.close()!=rock::api::Status::Ok)return false;
    callback=0;cursor=0;core=nullptr;collision=nullptr;return true;
}
