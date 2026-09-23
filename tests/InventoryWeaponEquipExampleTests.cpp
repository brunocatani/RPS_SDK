#include "../SDK/ROCK/examples/InventoryWeaponEquip.h"
#include <cassert>

namespace {
    using namespace rock::api;
    namespace next = weapon::v1_1;
    next::EquipRequest captured{};
    next::EquipState state{next::EquipState::Queued};
    Status captureStatus{Status::Ok}, cancelStatus{Status::Ok};
    unsigned submitted{}, cancelled{};
    Status ROCK_CALL registerOwner(const core::RegistrationV1*, core::OwnerV1* owner) noexcept {
        owner->ownerToken=42; return Status::Ok;
    }
    Status ROCK_CALL unregisterOwner(OwnerToken owner) noexcept { return owner==42 ? Status::Ok : Status::InvalidArgument; }
    Status ROCK_CALL bind(OwnerToken owner, InterfaceId family, std::uint32_t major, std::uint32_t permissions) noexcept {
        return owner==42 && family==InterfaceId::Weapon && major==1 && permissions==3 ? Status::Ok : Status::PermissionDenied;
    }
    Status ROCK_CALL capture(OwnerToken, std::uint32_t form, std::uint32_t stack, next::InventoryWeapon* item) noexcept {
        item->baseFormId=form; item->stackIndex=stack; item->stackKey=111;
        return captureStatus;
    }
    Status ROCK_CALL request(OwnerToken, const next::EquipRequest* item, std::uint64_t* command) noexcept {
        captured=*item; *command=++submitted; state=next::EquipState::Queued; return Status::RequestQueued;
    }
    Status ROCK_CALL poll(OwnerToken, std::uint64_t command, next::EquipResult* result) noexcept {
        result->commandId=command; result->state=state;
        result->flags=static_cast<std::uint32_t>(next::EquipResultFlag::IncomingEquipAccepted);
        return Status::Ok;
    }
    Status ROCK_CALL cancel(OwnerToken, std::uint64_t) noexcept { ++cancelled; return cancelStatus; }
    const core::ApiV1 coreApi{.registerConsumerV1=&registerOwner, .unregisterConsumerV1=&unregisterOwner, .bindInterface=&bind};
    const next::Api weaponApi{{}, &capture, &request, &poll, &cancel};
    const InterfaceDescriptorV1 coreDescriptor{sizeof(InterfaceDescriptorV1), InterfaceId::Core, 1, 0, sizeof(coreApi), 1, 0, 0, &coreApi};
    const InterfaceDescriptorV1 weaponDescriptor{sizeof(InterfaceDescriptorV1), InterfaceId::Weapon, 1, 1, sizeof(weaponApi), 1, 0, 0, &weaponApi};
    Status ROCK_CALL query(InterfaceId id, std::uint32_t major, std::uint32_t minor, std::uint32_t bytes,
        const InterfaceDescriptorV1** descriptor) noexcept {
        *descriptor=nullptr;
        if (major!=1) return Status::UnsupportedMajor;
        const auto& candidate=id==InterfaceId::Core ? coreDescriptor : weaponDescriptor;
        if (minor>candidate.minor) return Status::UnsupportedMinor;
        if (bytes>candidate.tableByteSize) return Status::TableTooSmall;
        *descriptor=&candidate;
        return Status::Ok;
    }
}
int main() {
    static_assert(sizeof(weapon::ApiV1)==128 && weapon::ApiV1::minorVersion==0);
    static_assert(sizeof(next::Api)==160 && sizeof(next::InventoryWeapon)==56);
    static_assert(sizeof(next::EquipRequest)==64 && sizeof(next::EquipResult)==64);
    Client client;
    assert(client.connect(&query,"Inventory equip example test")==Status::Ok);
    rock::sdk::example::InventoryWeaponEquip equip;
    assert(equip.connect(client)==Status::Ok);
    assert(equip.drawRight(0x1234,7)==Status::RequestQueued);
    assert(captured.hand==Hand::Right && captured.item.baseFormId==0x1234 && captured.item.stackIndex==7);
    assert(equip.drawLeft(0x5678,8)==Status::Busy && submitted==1);
    next::EquipResult result{};
    assert(equip.poll(result)==Status::Ok && equip.pending());
    state=next::EquipState::Succeeded;
    assert(equip.poll(result)==Status::Ok && !equip.pending());
    assert(equip.drawLeft(0x5678,8)==Status::RequestQueued);
    assert(captured.hand==Hand::Left && captured.item.baseFormId==0x5678 && captured.item.stackIndex==8);
    state=next::EquipState::Failed;
    assert(equip.poll(result)==Status::Ok && !equip.pending());
    assert(result.flags==static_cast<std::uint32_t>(next::EquipResultFlag::IncomingEquipAccepted));
    captureStatus=Status::TargetUnavailable;
    assert(equip.drawRight(1,2)==Status::TargetUnavailable && !equip.pending() && submitted==2);
    captureStatus=Status::Ok;
    assert(equip.drawLeft(3,4)==Status::RequestQueued);
    cancelStatus=Status::WrongThread;
    assert(equip.stop()==Status::WrongThread && equip.pending());
    cancelStatus=Status::AlreadyCommitted;
    assert(equip.stop()==Status::AlreadyCommitted && !equip.pending() && cancelled==2);
    assert(equip.drawRight(1,2)==Status::NotReady);
    assert(client.close()==Status::Ok);
}
