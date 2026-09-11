#include "RPSUICooperationApi.h"

#include <cstring>

namespace status_panel_example
{
    using namespace rpsui::sdk;

    // Integration fragment, with no F4SE entry point and no automatic activation.
    // Call these functions serially from the consumer's control thread. The
    // renderer and userData remain alive until stop() returns Ok. On CallbackBusy,
    // retry on a later control tick; never spin or wait inside a render callback.
    struct Panel
    {
        Panel() = default;
        Panel(const Panel&) = delete;
        Panel& operator=(const Panel&) = delete;

        // The owner calls stop explicitly; a destructor cannot retry CallbackBusy.
        const ApiV1* api{};
        const CooperationApiV1* cooperation{};
        ConsumerHandleV1 consumer{};
        std::uint64_t handle{};

        [[nodiscard]] ResultV1 start(PanelRenderCallbackV1 render, void* userData) noexcept
        {
            if (handle || consumer.ownerToken || !render) return ResultV1::InvalidArgument;
            api = RequestApiV1();
            cooperation = RequestCooperationApiV1();
            if (!api || !api->isFrameworkReady || !api->registerConsumer ||
                !api->unregisterConsumer || !api->registerPanel ||
                !api->submitPanelPresentation || !api->getPanelState ||
                !cooperation || !cooperation->registerPanel || !cooperation->getSnapshot ||
                !cooperation->unregisterConsumerSafely ||
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

            CooperativePanelRegistrationV1 contract{};
            auto& panel = contract.panel;
            std::memcpy(panel.panelId, "example.status.main", sizeof("example.status.main"));
            std::memcpy(panel.displayName, "Status", sizeof("Status"));
            panel.renderCallback = render;
            panel.userData = userData;
            PanelAgreementV1 agreement{};
            result = cooperation->registerPanel(consumer.ownerToken, &contract, &agreement);
            handle = agreement.panelHandle;
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

        // Advisory snapshot only: another mod may move/open/close after this call.
        // Pointer routing and overlap separation remain owned by the framework.
        [[nodiscard]] ResultV1 neighbors(CooperationSnapshotV1& out) const noexcept
        {
            out = {};
            return cooperation ? cooperation->getSnapshot(&out) : ResultV1::FrameworkNotReady;
        }

        [[nodiscard]] ResultV1 stop() noexcept
        {
            if (!api || !consumer.ownerToken) return ResultV1::Ok;
            // Closes callback admission immediately; Ok guarantees no remaining
            // RPS callback can use this owner's code/data. Busy retains ownership.
            const auto result = cooperation->unregisterConsumerSafely(consumer.ownerToken);
            if (result == ResultV1::Ok) {
                api = nullptr;
                cooperation = nullptr;
                consumer = {};
                handle = 0;
            }
            return result;
        }
    };
}
