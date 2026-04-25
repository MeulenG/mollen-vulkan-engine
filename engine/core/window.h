#ifndef MVE_WINDOW_H
#define MVE_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <string>

namespace mve {

class Window {
public:
    Window(uint32_t width, uint32_t height, const std::string& name);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool ShouldClose() const { return glfwWindowShouldClose(window_); }
    bool WasResized() const { return framebuffer_resized_; }
    void ResetResizedFlag() { framebuffer_resized_ = false; }

    vk::Extent2D GetExtent() const { return {width_, height_}; }
    GLFWwindow* GetGLFWWindow() const { return window_; }

    vk::SurfaceKHR createSurface(vk::Instance instance);

    static std::vector<const char*> GetRequiredInstanceExtensions();

    // Input state
    bool IsMouseButtonDown(int button) const;
    void GetCursorPos(double& x, double& y) const;
    float GetScrollDelta();

private:
    void initWindow();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void scrollCallback(GLFWwindow* window, double x_offset, double y_offset);

    uint32_t width_;
    uint32_t height_;
    bool framebuffer_resized_ = false;

    std::string window_name_;
    GLFWwindow* window_ = nullptr;
    float scroll_delta_ = 0.0f;
};

} // namespace mve

#endif // MVE_WINDOW_H
