#ifndef MVE_M2_LOADER_H
#define MVE_M2_LOADER_H

#include "m2_types.h"
#include "../scene/mesh.h"
#include "../animation/skeleton.h"
#include "../animation/animation_clip.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <string>
#include <vector>

namespace mve {

// Parsed M2 submesh — one draw call with one material
struct M2Submesh {
    uint32_t index_start;
    uint32_t index_count;
    uint16_t material_index;
    int16_t  shader_id;
    uint16_t texture_combo_index;
    uint16_t texture_count;
};

// Parsed M2 model — ready to be uploaded to GPU
struct M2Model {
    std::string name;

    // Geometry (combined from .skin file)
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<M2Submesh> submeshes;

    // Skeleton
    Skeleton skeleton;

    // Animations
    std::vector<std::unique_ptr<AnimationClip>> animations;

    // Textures (BLP file paths referenced by this model)
    std::vector<std::string> texture_paths;

    // Materials (blend modes, render flags)
    std::vector<m2::M2Material> materials;

    // Lookup tables
    std::vector<uint16_t> texture_lookup;
    std::vector<uint16_t> bone_lookup;

    // Bounding box
    glm::vec3 bbox_min;
    glm::vec3 bbox_max;
};

class M2Loader {
public:
    // Load an M2 model from raw bytes (M2 file + skin file)
    // skin_data can be null if you only want the header info
    static M2Model load(
        const uint8_t* m2_data, uint32_t m2_size,
        const uint8_t* skin_data = nullptr, uint32_t skin_size = 0);

    // Load from file paths
    static M2Model LoadFile(const std::string& m2_path);

private:
    static void parseHeader(const uint8_t* data, uint32_t size, M2Model& model);
    static void parseVertices(const uint8_t* data, const m2::M2Header& header, M2Model& model);
    static void parseBones(const uint8_t* data, const m2::M2Header& header, M2Model& model);
    static void parseTextures(const uint8_t* data, const m2::M2Header& header, M2Model& model);
    static void parseMaterials(const uint8_t* data, const m2::M2Header& header, M2Model& model);
    static void parseLookups(const uint8_t* data, const m2::M2Header& header, M2Model& model);
    static void parseAnimations(const uint8_t* data, uint32_t data_size, const m2::M2Header& header, M2Model& model);

    static void parseSkin(const uint8_t* skin_data, uint32_t skin_size,
                          const uint8_t* m2_data, const m2::M2Header& header,
                          M2Model& model);

    // Read an M2Track of vec3 or quat values from binary
    template<typename T>
    static std::vector<Keyframe<T>> readTrackKeyframes(
        const uint8_t* data,
        uint16_t interpolation_type,
        const m2::M2Array<>& timestamps_array,
        const m2::M2Array<>& values_array,
        uint32_t anim_index);

    // Helper to read data at an offset
    template<typename T>
    static const T* at(const uint8_t* data, uint32_t offset) {
        return reinterpret_cast<const T*>(data + offset);
    }
};

} // namespace mve

#endif // MVE_M2_LOADER_H
