#ifndef MVE_RENDERER_H
#define MVE_RENDERER_H

#include "device.h"
#include "swapchain.h"
#include "window.h"

namespace mve {

class Renderer {
public:
    Renderer(Window& window, Device& device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

private:
    Window& window_;
    Device& device_;
};

} // namespace mve

#endif // MVE_RENDERER_H
