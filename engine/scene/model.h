#ifndef MVE_MODEL_H
#define MVE_MODEL_H

#include "mesh.h"
#include "material.h"

#include <glm/glm.hpp>

namespace mve {

class Model {
public:
    Model() = default;

private:
    glm::mat4 transform_{1.0f};
};

} // namespace mve

#endif // MVE_MODEL_H
