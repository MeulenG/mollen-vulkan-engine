#ifndef MVE_IMAGE_H
#define MVE_IMAGE_H

#include "../core/device.h"

namespace mve {

class Image {
public:
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
};

} // namespace mve

#endif // MVE_IMAGE_H
