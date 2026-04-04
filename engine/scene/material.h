#ifndef MVE_MATERIAL_H
#define MVE_MATERIAL_H

#include <glm/glm.hpp>

namespace mve {

class Material {
public:
    Material() = default;

private:
    glm::vec3 ambient_{0.1f};
    glm::vec3 diffuse_{0.8f};
};

} // namespace mve

#endif // MVE_MATERIAL_H
