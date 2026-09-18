#pragma once
#include <ROCK/Weapon.h>
#include <ROCK/Core.h>
namespace rock::sdk::example {
inline api::weapon::EquippedWeaponStateV1 weaponIdentity(api::OwnerToken owner,const api::weapon::ApiV1* weapon,const api::core::SnapshotV1& frame) noexcept {
    api::weapon::EquippedWeaponStateV1 state{};
    if(!weapon || weapon->getEquippedWeaponStateV1(owner,&state)!=api::Status::Ok ||
       state.frameIndex!=frame.frameIndex || state.worldGeneration!=frame.worldGeneration ||
       state.skeletonGeneration!=frame.skeletonGeneration || state.providerGeneration!=frame.providerGeneration) return {};
    return state;
}
}
