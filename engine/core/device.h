#ifndef MVE_DEVICE_H
#define MVE_DEVICE_H

#include "window.h"

namespace mve {

class Device {
public:
    Device(Window& window);
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

private:
    Window& window_;
};

} // namespace mve

#endif // MVE_DEVICE_H
