#ifndef MVE_WINDOW_H
#define MVE_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

namespace mve {

class Window {
public:
    Window(uint32_t width, uint32_t height, const std::string& name);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const { return glfwWindowShouldClose(window_); }
    bool wasResized() const { return framebuffer_resized_; }
    void resetResizedFlag() { framebuffer_resized_ = false; }

    VkExtent2D getExtent() const { return {width_, height_}; }
    GLFWwindow* getGLFWWindow() const { return window_; }

private:
    void initWindow();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    uint32_t width_;
    uint32_t height_;
    bool framebuffer_resized_ = false;

    std::string window_name_;
    GLFWwindow* window_ = nullptr;
};

} // namespace mve

#endif // MVE_WINDOW_H
