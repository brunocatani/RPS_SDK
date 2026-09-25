#include "ModularAbiLayoutChecks.h"
#include <ROCK/InputV1_1.h>
static_assert(sizeof(rock::api::input::ApiV1)==80);
static_assert(sizeof(rock::api::input::v1_1::Api)==88);
static_assert(offsetof(rock::api::input::v1_1::Api,getPlacementClickState)==80);
static_assert(sizeof(rock::api::input::v1_1::PlacementClickState)==48);
static_assert(offsetof(rock::api::input::v1_1::PlacementClickState,sample)==16);
#include <ROCK/GrabV1_1.h>
static_assert(sizeof(rock::api::grab::ApiV1)==136);
static_assert(sizeof(rock::api::grab::v1_1::Api)==160);
static_assert(offsetof(rock::api::grab::v1_1::Api,getHeldPlacementState)==136);
static_assert(sizeof(rock::api::grab::v1_1::HeldPlacementState)==96);
static_assert(sizeof(rock::api::grab::v1_1::HeldPlacementIntent)==56);
int main() {}
