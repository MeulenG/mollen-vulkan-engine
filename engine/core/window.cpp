#include "window.h"

#include <stdexcept>

namespace mve {

Window::Window(uint32_t width, uint32_t height, const std::string& name)
    : width_{width}, height_{height}, window_name_{name} {
    initWindow();
}

Window::~Window() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void Window::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window_ = glfwCreateWindow(
        static_cast<int>(width_),
        static_cast<int>(height_),
        window_name_.c_str(),
        nullptr,
        nullptr);

    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto* app_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app_window->framebuffer_resized_ = true;
    app_window->width_ = static_cast<uint32_t>(width);
    app_window->height_ = static_cast<uint32_t>(height);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
}

} // namespace mve
