#ifndef MVE_MESH_H
#define MVE_MESH_H

#include <glm/glm.hpp>
#include <vector>

namespace mve {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
};

class Mesh {
public:
    Mesh() = default;

private:
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;
};

} // namespace mve

#endif // MVE_MESH_H
