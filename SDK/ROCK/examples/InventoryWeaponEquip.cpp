#include "InventoryWeaponEquip.h"

namespace rock::sdk::example {
    // Both examples preserve an existing weapon carried by the opposite hand.
    // Invoke one only after your own draw gesture. A base form alone is not an
    // exact selection: resolve the particular inventory stack in your mod.
    api::Status drawWeaponOnRightGesture(InventoryWeaponEquip& equip,
        std::uint32_t baseFormId, std::uint32_t stackIndex) noexcept {
        return equip.drawRight(baseFormId, stackIndex);
    }
    api::Status drawWeaponOnLeftGesture(InventoryWeaponEquip& equip,
        std::uint32_t baseFormId, std::uint32_t stackIndex) noexcept {
        return equip.drawLeft(baseFormId, stackIndex);
    }
    // On subsequent frames call equip.poll(result). Confirm removal from your
    // holster state only for Succeeded. On Failed/Cancelled inspect the result
    // flags and actual inventory/hand state before retrying: a failed outgoing
    // reattachment can coexist with a successfully equipped incoming weapon.
}
