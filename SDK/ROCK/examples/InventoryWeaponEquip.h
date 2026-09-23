#pragma once
#include <ROCK/Client.h>
#include <ROCK/WeaponV1_1.h>

namespace rock::sdk::example {
    // Call only from ROCK frame/phase callbacks. Your mod detects the draw
    // gesture and resolves its exact inventory stack before calling drawRight
    // or drawLeft. This helper has no automatic input or equip behavior.
    class InventoryWeaponEquip {
        const api::weapon::v1_1::Api* _weapon{};
        api::OwnerToken _owner{};
        std::uint64_t _command{};

        api::Status draw(api::Hand hand, std::uint32_t form, std::uint32_t stack) noexcept {
            if (!_weapon) return api::Status::NotReady;
            if (_command) return api::Status::Busy;
            api::weapon::v1_1::EquipRequest request{};
            request.hand = hand;
            auto status = _weapon->captureInventoryWeapon(_owner, form, stack, &request.item);
            if (status != api::Status::Ok) return status;
            status = _weapon->requestInventoryEquip(_owner, &request, &_command);
            if (status != api::Status::RequestQueued) _command = 0;
            return status;
        }
    public:
        InventoryWeaponEquip() = default;
        InventoryWeaponEquip(const InventoryWeaponEquip&) = delete;
        InventoryWeaponEquip& operator=(const InventoryWeaponEquip&) = delete;
        // Client must already be connected. This explicitly negotiates Weapon
        // major 1, minimum minor 1 and the extended table size, then binds R/W.
        api::Status connect(api::Client& client) noexcept {
            if (_weapon || _command) return api::Status::Busy;
            const auto status = client.acquire(3, _weapon);
            if (status == api::Status::Ok) _owner = client.owner();
            return status;
        }
        api::Status drawRight(std::uint32_t baseFormId, std::uint32_t stackIndex) noexcept {
            return draw(api::Hand::Right, baseFormId, stackIndex);
        }
        api::Status drawLeft(std::uint32_t baseFormId, std::uint32_t stackIndex) noexcept {
            return draw(api::Hand::Left, baseFormId, stackIndex);
        }
        [[nodiscard]] bool pending() const noexcept { return _command != 0; }
        api::Status poll(api::weapon::v1_1::EquipResult& result) noexcept {
            result = {};
            if (!_weapon || !_command) return api::Status::RequestNotFound;
            const auto status = _weapon->getInventoryEquipResult(_owner, _command, &result);
            using State = api::weapon::v1_1::EquipState;
            if (status == api::Status::Ok && (result.state == State::Succeeded ||
                result.state == State::Failed || result.state == State::Cancelled)) _command = 0;
            return status;
        }
        // Stop on activity/lifecycle loss, before client.close(). After native
        // commitment ROCK owns completion; do not issue another native equip,
        // release the retained weapon, or retry from this cleanup path.
        api::Status stop() noexcept {
            const auto status = _weapon && _command ?
                _weapon->cancelInventoryEquip(_owner, _command) : api::Status::Ok;
            if (status == api::Status::WrongThread || status == api::Status::Busy) return status;
            _command = 0;
            _owner = 0;
            _weapon = nullptr;
            return status;
        }
    };
}
