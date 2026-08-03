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

// Last-resort crash reporter: prints the faulting module + offset so a
// reload crash is diagnosable from stderr alone (WER stays silent for us).
static LONG WINAPI CrashFilter(EXCEPTION_POINTERS* ep) {
    void* addr = ep->ExceptionRecord->ExceptionAddress;
    HMODULE m = nullptr;
    char name[MAX_PATH] = "<unknown>";
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           static_cast<LPCWSTR>(addr), &m) && m) {
        GetModuleFileNameA(m, name, MAX_PATH);
    }
    std::fprintf(stderr, "[crash] code=0x%08lX addr=%p module=%s rva=0x%llX\n",
                 ep->ExceptionRecord->ExceptionCode, addr, name,
                 static_cast<unsigned long long>(
                     reinterpret_cast<uintptr_t>(addr) -
                     reinterpret_cast<uintptr_t>(m)));
    std::fflush(stderr);
    return EXCEPTION_CONTINUE_SEARCH;
}

int main() {
    SetUnhandledExceptionFilter(CrashFilter);

    const fs::path src = ExeDir() / "mollen-engine.dll";
    // Two alternating shadow-copy names: the active module keeps its copy
    // locked, so the incoming one loads from the other slot. Overlapping
    // old + new loads is what keeps shared dependencies (vulkan-1.dll,
    // glfw3.dll, pqxx.dll) referenced at all times - if the old module
    // were freed first, Windows would unload them mid-swap and tear down
    // live driver state under us.
    const fs::path live_slot[2] = {
        ExeDir() / "mollen-engine_live0.dll",
        ExeDir() / "mollen-engine_live1.dll",
    };
    int slot = 0;

    // Pin glfw3.dll for the whole process anyway: its window state and
    // WndProc must survive even a pathological swap sequence.
    if (!LoadLibraryW((ExeDir() / L"glfw3.dll").wstring().c_str())) {
        std::fprintf(stderr, "failed to pin glfw3.dll (%lu)\n", GetLastError());
        return EXIT_FAILURE;
    }

    Module mod;
    if (!LoadModule(mod, src, live_slot[slot])) {
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
            // Debounce: wait until the timestamp stops moving so we don't
            // copy a half-written DLL or double-reload one rebuild.
            for (;;) {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
                auto now_w = fs::last_write_time(src, ec);
                if (ec || now_w == w) break;
                w = now_w;
            }
            last_write = w;

            // Load the new module FIRST (from the other slot), then detach
            // the old one. Load failure = keep running the old module.
            std::fprintf(stderr, "[reload] loading new module\n"); std::fflush(stderr);
            Module next;
            if (!LoadModule(next, src, live_slot[1 - slot])) {
                std::fprintf(stderr, "[reload] new module failed to load; "
                                     "keeping the old one\n");
                continue;
            }
            slot = 1 - slot;

            std::fprintf(stderr, "[reload] on_unload\n"); std::fflush(stderr);
            mod.on_unload(state);
            std::fprintf(stderr, "[reload] freeing old module\n"); std::fflush(stderr);
            FreeLibrary(mod.handle);
            mod = next;
            std::fprintf(stderr, "[reload] on_reload\n"); std::fflush(stderr);
            mod.on_reload(state);
            std::fprintf(stderr, "[reload] engine module reloaded\n"); std::fflush(stderr);
        }
    }

    mod.shutdown(state);
    mod.destroy(state);
    FreeLibrary(mod.handle);
    return EXIT_SUCCESS;
}
