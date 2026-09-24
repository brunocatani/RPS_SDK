#include "ModularAbiLayoutChecks.h"
#include <ROCK/InputV1_1.h>
static_assert(sizeof(rock::api::input::ApiV1)==80);
static_assert(sizeof(rock::api::input::v1_1::Api)==88);
static_assert(offsetof(rock::api::input::v1_1::Api,getDecorationState)==80);
static_assert(sizeof(rock::api::input::v1_1::DecorationState)==48);
static_assert(offsetof(rock::api::input::v1_1::DecorationState,sample)==16);
int main() {}
