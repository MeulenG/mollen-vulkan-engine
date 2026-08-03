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

    InstallCallbacks();
}

// The engine's Window for callback dispatch. Deliberately NOT the GLFW
// user pointer: ImGui's docking backend stores its ImGuiContext* there and
// dereferences it, so sharing that slot corrupts whichever side reads last.
// A module-local static also self-heals on hot-reload - the new module's
// InstallCallbacks repopulates it.
static Window* s_active_window = nullptr;

void Window::InstallCallbacks() {
    s_active_window = this;
    glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
    glfwSetScrollCallback(window_, scrollCallback);
}

void Window::UninstallCallbacks() {
    // Callbacks point into this module's code; they must be cleared before a
    // hot-reload unloads it, or glfw3.dll dispatches events into freed code.
    glfwSetFramebufferSizeCallback(window_, nullptr);
    glfwSetScrollCallback(window_, nullptr);
}

vk::SurfaceKHR Window::createSurface(vk::Instance instance) {
    VkSurfaceKHR raw_surface;
    if (glfwCreateWindowSurface(instance, window_, nullptr, &raw_surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
    return vk::SurfaceKHR{raw_surface};
}

std::vector<const char*> Window::GetRequiredInstanceExtensions() {
    uint32_t count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    return {extensions, extensions + count};
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    Window* app_window = s_active_window;
    if (!app_window || app_window->window_ != window) return;
    app_window->framebuffer_resized_ = true;
    app_window->width_ = static_cast<uint32_t>(width);
    app_window->height_ = static_cast<uint32_t>(height);
}

bool Window::IsMouseButtonDown(int button) const {
    return glfwGetMouseButton(window_, button) == GLFW_PRESS;
}

void Window::GetCursorPos(double& x, double& y) const {
    glfwGetCursorPos(window_, &x, &y);
}

float Window::GetScrollDelta() {
    float d = scroll_delta_;
    scroll_delta_ = 0.0f;
    return d;
}

void Window::scrollCallback(GLFWwindow* window, double /*x_offset*/, double y_offset) {
    Window* app_window = s_active_window;
    if (!app_window || app_window->window_ != window) return;
    app_window->scroll_delta_ += static_cast<float>(y_offset);
}

} // namespace mve
