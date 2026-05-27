#include "engine_api.h"
#include "engine_state.h"

// Thin C wrappers over the C++ EngineState lifecycle. These are the only
// symbols the engine DLL exports; everything else stays internal to the
// module so a reload only ever swaps this code, never the host.

MVE_API mve::EngineState* engine_create() {
    return new mve::EngineState();
}

MVE_API void engine_init(mve::EngineState* s) {
    mve::EngineInit(*s);
}

MVE_API void engine_frame(mve::EngineState* s) {
    mve::EngineFrame(*s);
}

MVE_API bool engine_should_close(mve::EngineState* s) {
    return s->window->ShouldClose();
}

MVE_API void engine_shutdown(mve::EngineState* s) {
    mve::EngineShutdown(*s);
}

MVE_API void engine_destroy(mve::EngineState* s) {
    delete s;
}
