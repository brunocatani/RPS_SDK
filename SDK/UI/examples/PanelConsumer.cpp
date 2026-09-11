#include "RPSUIFrameworkApi.h"

#include <cstring>

namespace status_panel_example
{
    using namespace rpsui::sdk;

    // Integration fragment, with no F4SE entry point and no automatic activation.
    // Call these functions serially from the consumer's control thread. The
    // renderer and userData must remain alive while a host snapshot can use them;
    // unregister is not a render-callback join. Do not hot-unload this consumer.
    struct Panel
    {
        Panel() = default;
        Panel(const Panel&) = delete;
        Panel& operator=(const Panel&) = delete;

        // Call stop explicitly while the host is alive. Automatic destruction
        // cannot establish the external render quiescence needed by userData.
        const ApiV1* api{};
        ConsumerHandleV1 consumer{};
        std::uint64_t handle{};

        [[nodiscard]] ResultV1 start(PanelRenderCallbackV1 render, void* userData) noexcept
        {
            if (handle || consumer.ownerToken || !render) return ResultV1::InvalidArgument;
            api = RequestApiV1();
            if (!api || !api->isFrameworkReady || !api->registerConsumer ||
                !api->unregisterConsumer || !api->registerPanel ||
                !api->submitPanelPresentation || !api->getPanelState ||
                !api->isFrameworkReady()) return ResultV1::FrameworkNotReady;

            ConsumerRegistrationV1 registration{};
            std::memcpy(registration.consumerId, "example.status", sizeof("example.status"));
            std::memcpy(registration.displayName, "Status panel example", sizeof("Status panel example"));
            registration.requestedFeatures = featureMask(FeatureV1::ConsumerRenderCallbacks);
            auto result = api->registerConsumer(&registration, &consumer);
            if (result != ResultV1::Ok) return result;
            if ((consumer.grantedFeatures & registration.requestedFeatures) !=
                registration.requestedFeatures) {
                const auto cleanup = stop();
                return cleanup == ResultV1::Ok ? ResultV1::FrameworkNotReady : cleanup;
            }

            PanelRegistrationV1 panel{};
            std::memcpy(panel.panelId, "example.status.main", sizeof("example.status.main"));
            std::memcpy(panel.displayName, "Status", sizeof("Status"));
            panel.renderCallback = render;
            panel.userData = userData;
            result = api->registerPanel(consumer.ownerToken, &panel, &handle);
            if (result != ResultV1::Ok) {
                const auto cleanup = stop();
                if (cleanup != ResultV1::Ok) return cleanup;
            }
            return result; // Registered closed; the caller chooses when to open.
        }

        [[nodiscard]] ResultV1 present(const PanelPoseV1& pose, bool open) const noexcept
        {
            if (!api || !handle) return ResultV1::PanelNotRegistered;
            PanelPresentationV1 presentation{};
            presentation.pose = pose;
            presentation.open = open ? 1 : 0;
            // Zero asks the host to advance its submission sequence. Submit on
            // open/move/close, not every draw, so physical resizing is retained.
            return api->submitPanelPresentation(consumer.ownerToken, handle, &presentation);
        }

        [[nodiscard]] ResultV1 state(PanelStateV1& out) const noexcept
        {
            out = {};
            return api && handle ? api->getPanelState(consumer.ownerToken, handle, &out) :
                ResultV1::PanelNotRegistered;
        }

        [[nodiscard]] ResultV1 stop() noexcept
        {
            if (!api || !consumer.ownerToken) return ResultV1::Ok;
            // Removes every panel owned by this registration. Keep callback code
            // and userData alive until the host can no longer hold a snapshot.
            const auto result = api->unregisterConsumer(consumer.ownerToken);
            if (result == ResultV1::Ok || result == ResultV1::OwnerNotRegistered) {
                api = nullptr;
                consumer = {};
                handle = 0;
            }
            return result;
        }
    };
}
