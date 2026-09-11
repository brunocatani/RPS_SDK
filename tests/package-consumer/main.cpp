#include "PAPERApi.h"
#include "ROCKProviderApi.h"
#include "ROCKConfigurationApi.h"
#include "RPSUIFrameworkApi.h"

#include <cstdint>

int main()
{
    static_assert(paper::api::PAPER_API_VERSION == 1);
    static_assert(paper::api::PAPER_PROVIDER_API_V1_TABLE_BYTES == 400);
    static_assert(rock::provider::ROCK_PROVIDER_API_VERSION == 1);
    static_assert(rock::configuration_api::kVersion == 1);
    static_assert(rpsui::sdk::RPSUI_API_VERSION == 1);
    static_assert(rpsui::sdk::panelContains(0, 0.0f, 0.0f));
    static_assert(!rpsui::sdk::panelContains(
        static_cast<std::uint32_t>(rpsui::sdk::PanelFlagV1::CircularInput), 0.0f, 0.0f));
    return 0;
}
