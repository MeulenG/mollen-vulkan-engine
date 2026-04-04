#include "core/window.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    try {
        mve::Window window{1280, 720, "Mollen Wow Tools"};

        while (!window.shouldClose()) {
            glfwPollEvents();
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
