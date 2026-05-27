#include "core/engine_api.h"

#include <cstdlib>
#include <iostream>

// Thin host. It knows nothing about the engine's C++ types - only the opaque
// EngineState pointer and the C entry points. For now the engine module is
// linked implicitly (import lib); phase 5 swaps this for an explicit
// LoadLibrary so the module can be rebuilt and reloaded while the host runs.
int main() {
    try {
        mve::EngineState* state = engine_create();
        engine_init(state);

        while (!engine_should_close(state)) {
            engine_frame(state);
        }

        engine_shutdown(state);
        engine_destroy(state);

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
