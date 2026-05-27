#ifndef MVE_ENGINE_API_H
#define MVE_ENGINE_API_H

// C ABI boundary between the thin host exe and the engine module (DLL).
// The host only ever touches these functions plus an opaque EngineState
// pointer, so it needs none of the engine's C++ types. That is what lets the
// module be rebuilt and swapped without relinking or restarting the host
// (HOTRELOAD_PLAN.md phases 4-5).
#if defined(_WIN32)
  #if defined(MVE_ENGINE_BUILD)
    #define MVE_API extern "C" __declspec(dllexport)
  #else
    #define MVE_API extern "C" __declspec(dllimport)
  #endif
#else
  #define MVE_API extern "C"
#endif

namespace mve { struct EngineState; }

// Allocate + default-construct the state blob on the shared heap. Called once
// by the host; the pointer is retained across reloads.
MVE_API mve::EngineState* engine_create();

// Build window/device/systems and load the world (one-time setup).
MVE_API void engine_init(mve::EngineState* s);

// Pump OS events, advance one frame, render. Computes its own dt from the
// timestamp stored in the state, so the host loop carries no timing logic.
MVE_API void engine_frame(mve::EngineState* s);

// True once the OS window has been asked to close.
MVE_API bool engine_should_close(mve::EngineState* s);

// Release scene GPU resources while the systems are still alive. Called once
// at real exit, before engine_destroy.
MVE_API void engine_shutdown(mve::EngineState* s);

// Destroy the state blob.
MVE_API void engine_destroy(mve::EngineState* s);

#endif // MVE_ENGINE_API_H
