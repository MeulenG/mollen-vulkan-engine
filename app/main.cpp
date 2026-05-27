#include "core/engine_state.h"

#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>

// Thin host. Owns the EngineState blob and the OS event/timing loop; all
// engine logic lives in EngineInit / EngineFrame / EngineShutdown. This split
// is the foundation for the host/module DLL reload (HOTRELOAD_PLAN.md).
int main() {
    try {
        auto state = std::make_unique<mve::EngineState>();
        mve::EngineInit(*state);

        while (!state->window->ShouldClose()) {
            glfwPollEvents();

            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - state->last_time).count();
            state->last_time = now;

            mve::EngineFrame(*state, dt);
        }

        mve::EngineShutdown(*state);

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
