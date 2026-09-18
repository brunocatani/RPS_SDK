#pragma pack(push,1)
#include ROCK_MODULAR_HEADER
#include <cstdint>
static_assert(alignof(ROCK_MODULAR_NAMESPACE::ApiV1)==8);
struct CallerPacking { char x; std::uint64_t y; };
static_assert(alignof(CallerPacking)==1);
#pragma pack(pop)
