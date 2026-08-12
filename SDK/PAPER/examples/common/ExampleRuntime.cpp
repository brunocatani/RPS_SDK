#include <F4SE/F4SE.h>
#include <REL/Relocation.h>

#include "ExampleRuntime.h"

#include <cstdio>
#include <exception>
#include <memory>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

namespace
{
    using namespace paper::api;

    std::shared_ptr<spdlog::logger> g_logger;
    std::uint64_t g_ownerToken{ 0 };
    std::uint64_t g_callbackToken{ 0 };

    void initializeLogger()
    {
        auto directory = F4SE::log::log_directory();
        if (!directory) {
            return;
        }
        const auto& example = paper::sdk::example::definition();
        *directory /= std::string(example.pluginName) + ".log";
        auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            directory->string(),
            2 * 1024 * 1024,
            2,
            true);
        g_logger = std::make_shared<spdlog::logger>(
            example.pluginName,
            std::move(sink));
        g_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] %v");
        g_logger->set_level(spdlog::level::info);
        g_logger->flush_on(spdlog::level::info);
    }

    void disconnectPaper() noexcept
    {
        const auto& example = paper::sdk::example::definition();
        if (g_ownerToken != 0 && example.onStop) {
            example.onStop(g_ownerToken);
        }
        if (PaperApi::inst && g_ownerToken != 0 && g_callbackToken != 0) {
            (void)PaperApi::inst->unregisterEventCallbackV1(
                g_ownerToken,
                g_callbackToken);
        }
        g_callbackToken = 0;
        if (PaperApi::inst && g_ownerToken != 0) {
            (void)PaperApi::inst->unregisterConsumerV1(g_ownerToken);
        }
        g_ownerToken = 0;
    }

    void PAPER_CALL onPaperEvent(const PaperEventV1* eventData, void*)
    {
        if (!eventData || g_ownerToken == 0) {
            return;
        }
        try {
            const auto& example = paper::sdk::example::definition();
            if (example.onEvent) {
                example.onEvent(g_ownerToken, *eventData);
            }
        } catch (const std::exception& error) {
            paper::sdk::example::logWarning(error.what());
        } catch (...) {
            paper::sdk::example::logWarning("Unhandled PAPER event failure");
        }
    }

    bool connectPaper() noexcept
    {
        if (g_ownerToken != 0) {
            return true;
        }

        const auto& example = paper::sdk::example::definition();
        const auto initializeResult = PaperApi::initialize(
            PAPER_API_VERSION,
            example.minimumTableBytes);
        if (initializeResult != 0 || !PaperApi::inst) {
            char message[160]{};
            std::snprintf(
                message,
                sizeof(message),
                "PAPER API initialization failed with code %d",
                initializeResult);
            paper::sdk::example::logWarning(message);
            return false;
        }

        PaperConsumerRegistrationV1 registration{};
        std::snprintf(
            registration.modName,
            sizeof(registration.modName),
            "%s",
            example.pluginName);
        registration.requestedCapabilities = example.requestedCapabilities;

        PaperConsumerHandleV1 handle{};
        const auto result =
            PaperApi::inst->registerConsumerV1(&registration, &handle);
        if (result != PaperResultV1::Ok || handle.ownerToken == 0 ||
            (handle.grantedCapabilities & example.requestedCapabilities) !=
                example.requestedCapabilities) {
            if (handle.ownerToken != 0) {
                (void)PaperApi::inst->unregisterConsumerV1(handle.ownerToken);
            }
            paper::sdk::example::logWarning(
                "PAPER denied one or more required capabilities");
            return false;
        }

        g_ownerToken = handle.ownerToken;
        if (example.onStart && !example.onStart(g_ownerToken)) {
            disconnectPaper();
            return false;
        }

        const auto callbackResult = PaperApi::inst->registerEventCallbackV1(
            g_ownerToken,
            &onPaperEvent,
            nullptr,
            &g_callbackToken);
        if (callbackResult != PaperResultV1::Ok || g_callbackToken == 0) {
            disconnectPaper();
            paper::sdk::example::logWarning(
                "PAPER event callback registration failed");
            return false;
        }

        paper::sdk::example::logInfo("Connected to PAPER provider API V1");
        return true;
    }

    void onF4SEMessage(F4SE::MessagingInterface::Message* message) noexcept
    {
        if (!message) {
            return;
        }
        switch (message->type) {
        case F4SE::MessagingInterface::kGameLoaded:
        case F4SE::MessagingInterface::kGameDataReady:
        case F4SE::MessagingInterface::kPostLoadGame:
        case F4SE::MessagingInterface::kNewGame:
            (void)connectPaper();
            break;
        case F4SE::MessagingInterface::kPreLoadGame:
            disconnectPaper();
            break;
        default:
            break;
        }
    }
}

namespace paper::sdk::example
{
    void logInfo(const std::string_view message) noexcept
    {
        try {
            if (g_logger) {
                g_logger->info("{}", message);
            }
        } catch (...) {
        }
    }

    void logWarning(const std::string_view message) noexcept
    {
        try {
            if (g_logger) {
                g_logger->warn("{}", message);
            }
        } catch (...) {
        }
    }
}

extern "C" __declspec(dllexport) bool F4SEAPI F4SEPlugin_Query(
    const F4SE::QueryInterface* a_f4se,
    F4SE::PluginInfo* a_info) noexcept
{
    try {
        if (!a_f4se || !a_info) {
            return false;
        }

        initializeLogger();
        const auto& example = paper::sdk::example::definition();
        a_info->infoVersion = F4SE::PluginInfo::kVersion;
        a_info->name = example.pluginName;
        a_info->version = example.pluginVersion;

        if (a_f4se->IsEditor()) {
            paper::sdk::example::logWarning("Editor runtime is unsupported");
            return false;
        }
        if (!REL::Module::IsVR()) {
            paper::sdk::example::logWarning(
                "Fallout 4 VR runtime is required");
            return false;
        }

        const auto executableVersion = REL::Module::get().version();
        if (executableVersion != F4SE::RUNTIME_VR_1_2_72) {
            paper::sdk::example::logWarning(
                "Unsupported Fallout4VR.exe version; 1.2.72 is required");
            return false;
        }

        char message[192]{};
        std::snprintf(
            message,
            sizeof(message),
            "Query passed: F4SE=%s loaderRuntime=%s executable=%s",
            a_f4se->F4SEVersion().string().c_str(),
            a_f4se->RuntimeVersion().string().c_str(),
            executableVersion.string().c_str());
        paper::sdk::example::logInfo(message);
        return true;
    } catch (...) {
        return false;
    }
}

extern "C" __declspec(dllexport) bool F4SEAPI F4SEPlugin_Load(
    const F4SE::LoadInterface* a_f4se) noexcept
{
    try {
        if (!a_f4se) {
            return false;
        }
        F4SE::Init(a_f4se, false);
        const auto* messaging = F4SE::GetMessagingInterface();
        if (!messaging || !messaging->RegisterListener(onF4SEMessage)) {
            paper::sdk::example::logWarning(
                "F4SE messaging registration failed");
            return false;
        }
        paper::sdk::example::logInfo("Plugin loaded; waiting for game data");
        return true;
    } catch (...) {
        return false;
    }
}
