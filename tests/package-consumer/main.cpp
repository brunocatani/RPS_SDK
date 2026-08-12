#include "PAPERApi.h"
#include "ROCKProviderApi.h"

#include <cstdint>

int main()
{
    static_assert(paper::api::PAPER_API_VERSION == 1);
    static_assert(paper::api::PAPER_PROVIDER_API_V1_TABLE_BYTES == 400);
    static_assert(rock::provider::ROCK_PROVIDER_API_VERSION == 1);
    return 0;
}
