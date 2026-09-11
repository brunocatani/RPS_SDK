#include "ExampleTestSupport.h"
#include <Windows.h>

namespace rock::sdk::example {
    void logInfo(std::string_view) noexcept {}
    void logWarning(std::string_view) noexcept {}
}
bool testOffhandLease();
bool testVisualHandOffset();
bool testColliderFocus();
bool testTouchMechanism();
bool testExternalContactSensor();
bool testWeaponPartDriver();
bool testPowerArmorInteraction();

int main(int argc, char** argv)
{
    bool ok = true;
    ok &= testOffhandLease();
    ok &= testVisualHandOffset();
    ok &= testColliderFocus();
    ok &= testTouchMechanism();
    ok &= testExternalContactSensor();
    ok &= testWeaponPartDriver();
    ok &= testPowerArmorInteraction();
    // Inspect exports without running CommonLib static initialization, which
    // requires the game executable. Loader admission still requires FO4VR.
    for (int index = 1; index < argc; ++index) {
        HMODULE module = LoadLibraryExA(argv[index], nullptr, DONT_RESOLVE_DLL_REFERENCES);
        if (!checkExample(module != nullptr, argv[index], "DLL could not load")) { ok = false; continue; }
        using Query = bool (*)(const void*, void*);
        using Load = bool (*)(const void*);
        auto query = reinterpret_cast<Query>(GetProcAddress(module, "F4SEPlugin_Query"));
        auto load = reinterpret_cast<Load>(GetProcAddress(module, "F4SEPlugin_Load"));
        ok &= checkExample(query && load, argv[index], "missing F4SE exports");
        FreeLibrary(module);
    }
    return ok ? 0 : 1;
}
