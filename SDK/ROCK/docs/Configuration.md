# ROCK configuration API

`ROCKConfigurationApi.h` defines an independent V1 interface exported by
`ROCK.dll` as `GetROCKConfigurationApi`. It is not an appended
`RockProviderApi` slot and uses no provider owner token. Link `RPS::ROCK` for
the header, discover the export, and validate `version`, `byteSize`, and each
callback before use.

```cpp
#include <Windows.h>
#include <ROCKConfigurationApi.h>

const rock::configuration_api::ApiV1* findRockConfiguration() noexcept
{
    using namespace rock::configuration_api;
    const auto module = GetModuleHandleW(L"ROCK.dll");
    if (!module) return nullptr;
    const auto get = reinterpret_cast<GetApiV1>(GetProcAddress(module, kExportName));
    const auto* api = get ? get(kVersion) : nullptr;
    return api && api->version == kVersion && api->byteSize >= sizeof(ApiV1) &&
        api->revision && api->visit && api->setValue ? api : nullptr;
}
```

The x64 table is 32 bytes: version/byte size plus three function pointers.

| Callback | Meaning and thread |
| --- | --- |
| `revision()` | Applied catalog revision; safe on any thread and performs no I/O. Zero means no applied configuration yet. |
| `visit(Group, VisitorV1, context)` | Enumerate the owning group's compiled settings on the game task thread, outside physics/input callbacks. Returns false when unavailable or invalid. |
| `setValue(Group, section, key, value, error, capacity)` | Save a supported setting through ROCK on the game task thread. Returns success/failure and a bounded error string when supplied. |

`SettingV1` supplies section, key, current/default value, category, description,
`ValueType` (`Boolean`, `Integer`, `Float`, `String`), and `overridden`.
All strings and the record are borrowed only for the visitor call. Copy what
the UI needs into consumer-owned storage. `overridden` means the current value
differs from its compiled default; it does not mean an entry merely exists on disk.
Do not call mutating configuration operations recursively from the visitor.

## Persistence and application

The owning `Group` is `Consumer` or `Developer`, selected by the compiled
catalog. The two authoritative runtime files are beneath the user's Documents
known folder:

```text
My Games/Fallout4VR/Mods_Config/ROCK/ROCK.ini
My Games/Fallout4VR/Mods_Config/ROCK/ROCK_Developer.ini
```

Consumers should obtain sections and keys from `visit`; category labels and
numbered display sections are presentation metadata, not an independent key
catalog. The wheel's ROCK and Developer tabs use this same interface.

ROCK creates a missing consumer file from compiled defaults. Loading an existing
file does not append keys or replace it. Missing keys use compiled defaults.
The developer file is optional: a non-default developer edit creates an override,
resetting an edited key to default removes that key, and an empty developer file
is removed. Existing supplied developer entries load unchanged, including entries
at defaults; editing another setting preserves them.

`setValue` validates key ownership and value syntax and writes through ROCK's
store. Success means saved, not already applied. The normal configuration reload
applies it and then publishes a new revision when loaded state changes. A no-op
edit need not advance revision. Refresh the catalog after application rather
than assuming the value changed synchronously. Both runtime files support hot
reload, including creation/removal of the developer file.

The repository's `ROCK_example.ini` and `ROCK_Developer_example.ini` are human
references. Do not read, copy, embed, install or ship them as runtime defaults.
Mandatory collision/input/haptic behavior has no INI enable switch; render only
supported catalog entries. This interface governs ROCK configuration only.
