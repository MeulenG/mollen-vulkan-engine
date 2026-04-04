#ifndef MVE_BUFFER_H
#define MVE_BUFFER_H

#include "../core/device.h"

namespace mve {

class Buffer {
public:
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
};

} // namespace mve

#endif // MVE_BUFFER_H
