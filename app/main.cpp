// Thin host with C++ hot-reload (HOTRELOAD_PLAN.md phase 5).
//
// The host owns nothing but the opaque EngineState pointer and the loaded
// module. It loads the engine DLL explicitly (so it is NOT locked by the
// loader), watches the on-disk DLL for changes, and on a rebuild swaps in the
// new code while keeping the same EngineState. Because the state blob lives on
// the shared heap and holds no module-bound pointers (no vtables/typeid - see
// the ECS rework), the new code picks up exactly where the old left off.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <thread>

namespace mve { struct EngineState; }
namespace fs = std::filesystem;

// Function-pointer types matching engine_api.h's C entry points.
using FnCreate      = mve::EngineState* (*)();
using FnInit        = void (*)(mve::EngineState*);
using FnFrame       = void (*)(mve::EngineState*);
using FnShouldClose = bool (*)(mve::EngineState*);
using FnShutdown    = void (*)(mve::EngineState*);
using FnDestroy     = void (*)(mve::EngineState*);
using FnOnUnload    = void (*)(mve::EngineState*);
using FnOnReload    = void (*)(mve::EngineState*);

struct Module {
    HMODULE       handle       = nullptr;
    FnCreate      create       = nullptr;
    FnInit        init         = nullptr;
    FnFrame       frame        = nullptr;
    FnShouldClose should_close = nullptr;
    FnShutdown    shutdown     = nullptr;
    FnDestroy     destroy      = nullptr;
    FnOnUnload    on_unload    = nullptr;
    FnOnReload    on_reload    = nullptr;
};

// Directory containing this exe. The engine DLL is deployed next to it by the
// engine target's post-build copy.
static fs::path ExeDir() {
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    return fs::path(std::string(buf, n)).parent_path();
}

static bool LoadModule(Module& m, const fs::path& src, const fs::path& live) {
    // Load a COPY so the watched DLL on disk stays unlocked and a rebuild can
    // overwrite it freely.
    std::error_code ec;
    fs::copy_file(src, live, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        std::fprintf(stderr, "[reload] copy failed: %s\n", ec.message().c_str());
        return false;
    }
    m.handle = LoadLibraryW(live.wstring().c_str());
    if (!m.handle) {
        std::fprintf(stderr, "[reload] LoadLibrary failed (%lu)\n", GetLastError());
        return false;
    }
    auto get = [&](const char* name) { return GetProcAddress(m.handle, name); };
    m.create       = reinterpret_cast<FnCreate>(get("engine_create"));
    m.init         = reinterpret_cast<FnInit>(get("engine_init"));
    m.frame        = reinterpret_cast<FnFrame>(get("engine_frame"));
    m.should_close = reinterpret_cast<FnShouldClose>(get("engine_should_close"));
    m.shutdown     = reinterpret_cast<FnShutdown>(get("engine_shutdown"));
    m.destroy      = reinterpret_cast<FnDestroy>(get("engine_destroy"));
    m.on_unload    = reinterpret_cast<FnOnUnload>(get("engine_on_unload"));
    m.on_reload    = reinterpret_cast<FnOnReload>(get("engine_on_reload"));
    return m.create && m.init && m.frame && m.should_close && m.shutdown &&
           m.destroy && m.on_unload && m.on_reload;
}

int main() {
    const fs::path src  = ExeDir() / "mollen-engine.dll";
    const fs::path live = ExeDir() / "mollen-engine_live.dll";

    Module mod;
    if (!LoadModule(mod, src, live)) {
        std::fprintf(stderr, "Failed to load engine module from %s\n",
                     src.string().c_str());
        return EXIT_FAILURE;
    }

    mve::EngineState* state = mod.create();
    mod.init(state);

    std::error_code ec;
    auto last_write = fs::last_write_time(src, ec);

    while (!mod.should_close(state)) {
        mod.frame(state);

        // Watch the on-disk DLL; reload when a rebuild changes its timestamp.
        auto w = fs::last_write_time(src, ec);
        if (!ec && w != last_write) {
            // Let the linker finish writing before we copy + load.
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            mod.on_unload(state);
            FreeLibrary(mod.handle);

            Module next;
            if (LoadModule(next, src, live)) {
                mod = next;
                mod.on_reload(state);
                last_write = fs::last_write_time(src, ec);
                std::fprintf(stderr, "[reload] engine module reloaded\n");
            } else {
                std::fprintf(stderr, "[reload] FAILED; aborting to avoid a "
                                     "half-loaded state\n");
                return EXIT_FAILURE;
            }
        }
    }

    mod.shutdown(state);
    mod.destroy(state);
    FreeLibrary(mod.handle);
    return EXIT_SUCCESS;
}
