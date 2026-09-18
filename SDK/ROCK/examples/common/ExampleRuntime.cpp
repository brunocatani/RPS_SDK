#include "ExampleRuntime.h"

#include <F4SE/F4SE.h>
#include <REL/Relocation.h>
#include <Windows.h>

#include <cstdio>
#include <exception>
#include <filesystem>
#include <memory>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

namespace
{
    using rock::sdk::example::hasLifecycleFlag;

    rock::api::Client g_client;
    const rock::api::core::ApiV1* g_core{};
    std::shared_ptr<spdlog::logger> g_logger;
    std::uint64_t g_ownerToken{ 0 };
    std::uint64_t g_callbackToken{ 0 };

    void initializeLogger()
    {
        auto directory = F4SE::log::log_directory();
        if (!directory) {
            return;
        }
        const auto& example = rock::sdk::example::definition();
        *directory /= std::string(example.pluginName) + ".log";
        auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            directory->string(),
            2 * 1024 * 1024,
            2,
            true);
        g_logger = std::make_shared<spdlog::logger>(example.pluginName, std::move(sink));
        g_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] %v");
        g_logger->set_level(spdlog::level::info);
        g_logger->flush_on(spdlog::level::info);
    }

    void disconnectRock() noexcept
    {
        const auto& example = rock::sdk::example::definition();
        if (g_ownerToken != 0 && example.onStop) {
            example.onStop(g_ownerToken);
        }
        const auto result=g_client.close();
        if(result!=rock::api::Status::Ok && result!=rock::api::Status::OwnerNotRegistered) {
            rock::sdk::example::logWarning("ROCK owner cleanup was rejected");return;
        }
        g_callbackToken=0;g_ownerToken=0;g_core=nullptr;
    }

    void ROCK_CALL onRockFrame(
        const rock::api::core::SnapshotV1* snapshot,
        void*)
    {
        if (!snapshot || g_ownerToken == 0) {
            return;
        }
        try {
            const auto& example = rock::sdk::example::definition();
            if (example.onFrame) {
                example.onFrame(g_ownerToken, *snapshot);
            }
        } catch (const std::exception& error) {
            rock::sdk::example::logWarning(error.what());
        } catch (...) {
            rock::sdk::example::logWarning("Unhandled frame callback failure");
        }
    }

    bool connectRock() noexcept
    {
        if (g_ownerToken != 0) {
            return true;
        }

        const auto& example = rock::sdk::example::definition();
        const auto module=GetModuleHandleA("ROCK.dll");
        const auto query=module?reinterpret_cast<rock::api::QueryInterfaceV1>(GetProcAddress(module,rock::api::kQueryExportName)):nullptr;
        if(g_client.connect(query,example.pluginName)!=rock::api::Status::Ok)return false;
        if(g_client.acquire(5,g_core)!=rock::api::Status::Ok ||
           (example.onConnect && !example.onConnect(g_client,query))) {
            (void)g_client.close();
            rock::sdk::example::logWarning("A required ROCK interface major or permission is unavailable");return false;
        }
        g_ownerToken=g_client.owner();
        if (example.onStart && !example.onStart(g_ownerToken)) {
            disconnectRock();
            return false;
        }

        const auto callbackResult =
            g_core->registerFrameCallbackForOwnerV1(
                g_ownerToken,
                &onRockFrame,
                nullptr,
                &g_callbackToken);
        if (callbackResult != rock::api::Status::Ok || g_callbackToken == 0) {
            disconnectRock();
            rock::sdk::example::logWarning("ROCK owner frame callback registration failed");
            return false;
        }

        rock::sdk::example::logInfo("Connected to independently negotiated ROCK providers");
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
            (void)connectRock();
            break;
        case F4SE::MessagingInterface::kPreLoadGame:
            disconnectRock();
            break;
        default:
            break;
        }
    }
}

namespace rock::sdk::example
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
        const auto& example = rock::sdk::example::definition();
        a_info->infoVersion = F4SE::PluginInfo::kVersion;
        a_info->name = example.pluginName;
        a_info->version = example.pluginVersion;

        if (a_f4se->IsEditor()) {
            rock::sdk::example::logWarning("Editor runtime is unsupported");
            return false;
        }
        if (!REL::Module::IsVR()) {
            rock::sdk::example::logWarning("Fallout 4 VR runtime is required");
            return false;
        }

        const auto executableVersion = REL::Module::get().version();
        if (executableVersion != F4SE::RUNTIME_VR_1_2_72) {
            rock::sdk::example::logWarning(
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
        rock::sdk::example::logInfo(message);
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
            rock::sdk::example::logWarning("F4SE messaging registration failed");
            return false;
        }
        rock::sdk::example::logInfo("Plugin loaded; waiting for game data");
        return true;
    } catch (...) {
        return false;
    }
}
