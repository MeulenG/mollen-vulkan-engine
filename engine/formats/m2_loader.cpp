#include "m2_loader.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace mve {

static constexpr uint32_t M2_MAGIC = 0x3032444D; // "MD20"

M2Model M2Loader::load(
    const uint8_t* m2_data, uint32_t m2_size,
    const uint8_t* skin_data, uint32_t skin_size) {

    M2Model model;
    parseHeader(m2_data, m2_size, model);
    const auto& header = *at<m2::M2Header>(m2_data, 0);

    parseVertices(m2_data, header, model);
    parseBones(m2_data, header, model);
    parseTextures(m2_data, header, model);
    parseMaterials(m2_data, header, model);
    parseLookups(m2_data, header, model);
    parseAnimations(m2_data, m2_size, header, model);

    if (skin_data && skin_size > 0) {
        parseSkin(skin_data, skin_size, m2_data, header, model);
    }

    return model;
}

M2Model M2Loader::LoadFile(const std::string& m2_path) {
    // Read M2 file
    std::ifstream m2_file(m2_path, std::ios::binary | std::ios::ate);
    if (!m2_file.is_open()) {
        throw std::runtime_error("Failed to open M2 file: " + m2_path);
    }

    auto m2_size = m2_file.tellg();
    m2_file.seekg(0);
    std::vector<uint8_t> m2_data(static_cast<size_t>(m2_size));
    m2_file.read(reinterpret_cast<char*>(m2_data.data()), m2_size);

    // Try to find the .skin file (ModelName00.skin)
    namespace fs = std::filesystem;
    fs::path m2_fs_path{m2_path};
    std::string stem = m2_fs_path.stem().string();
    fs::path skin_path = m2_fs_path.parent_path() / (stem + "00.skin");

    std::vector<uint8_t> skin_data;
    if (fs::exists(skin_path)) {
        std::ifstream skin_file(skin_path, std::ios::binary | std::ios::ate);
        auto skin_size = skin_file.tellg();
        skin_file.seekg(0);
        skin_data.resize(static_cast<size_t>(skin_size));
        skin_file.read(reinterpret_cast<char*>(skin_data.data()), skin_size);
    }

    return load(
        m2_data.data(), static_cast<uint32_t>(m2_data.size()),
        skin_data.empty() ? nullptr : skin_data.data(),
        static_cast<uint32_t>(skin_data.size()));
}

void M2Loader::parseHeader(const uint8_t* data, uint32_t size, M2Model& model) {
    if (size < sizeof(m2::M2Header)) {
        throw std::runtime_error("M2 file too small for header");
    }

    const auto& header = *at<m2::M2Header>(data, 0);

    if (header.magic != M2_MAGIC) {
        throw std::runtime_error("Invalid M2 magic number");
    }

    // Read model name
    if (header.name.count > 0 && header.name.offset > 0) {
        model.name = std::string(
            reinterpret_cast<const char*>(data + header.name.offset),
            header.name.count - 1); // -1 for null terminator
    }

    model.bbox_min = {header.bounding_box_min.x, header.bounding_box_min.y, header.bounding_box_min.z};
    model.bbox_max = {header.bounding_box_max.x, header.bounding_box_max.y, header.bounding_box_max.z};
}

void M2Loader::parseVertices(const uint8_t* data, const m2::M2Header& header, M2Model& model) {
    // M2 vertices are stored in the main M2 file.
    // We DON'T convert them to our Vertex format here — that happens in parseSkin()
    // because the skin file remaps vertex indices.
    // Just validate they exist.
    if (header.vertices.count == 0) return;
}

void M2Loader::parseBones(const uint8_t* data, const m2::M2Header& header, M2Model& model) {
    if (header.bones.count == 0) return;

    const auto* bones = at<m2::M2CompBone>(data, header.bones.offset);

    for (uint32_t i = 0; i < header.bones.count; i++) {
        const auto& src = bones[i];

        Bone bone;
        bone.name = "bone_" + std::to_string(i);
        bone.parent_index = src.parent_bone;

        // For M2, bind position/rotation/scale are identity —
        // the pivots handle positioning.
        bone.bind_position = glm::vec3{0.0f};
        bone.bind_rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
        bone.bind_scale = glm::vec3{1.0f};

        model.skeleton.AddBone(bone);
        model.skeleton.SetPivot(i, {src.pivot.x, src.pivot.y, src.pivot.z});
    }

    // For M2 models, inverse bind matrices are not used —
    // the pivot-based computation handles everything.
    model.skeleton.ComputeInverseBindMatrices();
}

void M2Loader::parseTextures(const uint8_t* data, const m2::M2Header& header, M2Model& model) {
    if (header.textures.count == 0) return;

    const auto* textures = at<m2::M2Texture>(data, header.textures.offset);

    for (uint32_t i = 0; i < header.textures.count; i++) {
        if (textures[i].filename.count > 0 && textures[i].filename.offset > 0) {
            std::string path(
                reinterpret_cast<const char*>(data + textures[i].filename.offset),
                textures[i].filename.count - 1);

            // Normalize path separators
            std::replace(path.begin(), path.end(), '\\', '/');
            model.texture_paths.push_back(path);
        } else {
            model.texture_paths.push_back(""); // procedural texture
        }
    }
}

void M2Loader::parseMaterials(const uint8_t* data, const m2::M2Header& header, M2Model& model) {
    if (header.render_flags.count == 0) return;

    const auto* materials = at<m2::M2Material>(data, header.render_flags.offset);

    model.materials.resize(header.render_flags.count);
    std::memcpy(model.materials.data(), materials, header.render_flags.count * sizeof(m2::M2Material));
}

void M2Loader::parseLookups(const uint8_t* data, const m2::M2Header& header, M2Model& model) {
    // Texture lookup
    if (header.texture_lookup.count > 0) {
        const auto* lut = at<uint16_t>(data, header.texture_lookup.offset);
        model.texture_lookup.assign(lut, lut + header.texture_lookup.count);
    }

    // Bone lookup
    if (header.bone_lookup.count > 0) {
        const auto* lut = at<uint16_t>(data, header.bone_lookup.offset);
        model.bone_lookup.assign(lut, lut + header.bone_lookup.count);
    }
}

// Helper: check if a byte range is within the file
static bool boundsOk(uint32_t offset, uint32_t count, uint32_t elem_size, uint32_t file_size) {
    if (offset == 0 || count == 0) return false;
    uint64_t end = static_cast<uint64_t>(offset) + static_cast<uint64_t>(count) * elem_size;
    return end <= file_size;
}

void M2Loader::parseAnimations(const uint8_t* data, uint32_t data_size,
                                const m2::M2Header& header, M2Model& model) {
    if (header.animations.count == 0 || header.bones.count == 0) return;

    if (!boundsOk(header.animations.offset, header.animations.count, sizeof(m2::M2Sequence), data_size))
        return;

    const auto* sequences = at<m2::M2Sequence>(data, header.animations.offset);
    const auto* bones = at<m2::M2CompBone>(data, header.bones.offset);

    for (uint32_t a = 0; a < header.animations.count; a++) {
        const auto& seq = sequences[a];
        float duration_sec = seq.duration / 1000.0f;
        if (duration_sec <= 0.0f) continue;

        auto clip = std::make_unique<AnimationClip>(
            "anim_" + std::to_string(seq.id) + "_" + std::to_string(seq.variation_index),
            duration_sec);

        for (uint32_t b = 0; b < header.bones.count; b++) {
            const auto& bone = bones[b];
            BoneTrack track;

            // --- Translation (vec3 keyframes) ---
            if (bone.translation.timestamps.count > a &&
                boundsOk(bone.translation.timestamps.offset, bone.translation.timestamps.count, 8, data_size) &&
                boundsOk(bone.translation.values.offset, bone.translation.values.count, 8, data_size)) {

                auto& ts_inner = at<m2::M2Array<>>(data, bone.translation.timestamps.offset)[a];
                auto& val_inner = at<m2::M2Array<>>(data, bone.translation.values.offset)[a];

                if (ts_inner.count > 0 && val_inner.count > 0 &&
                    boundsOk(ts_inner.offset, ts_inner.count, 4, data_size) &&
                    boundsOk(val_inner.offset, val_inner.count, 12, data_size)) {

                    const auto* timestamps = at<uint32_t>(data, ts_inner.offset);
                    const auto* values = at<m2::C3Vector>(data, val_inner.offset);
                    uint32_t count = std::min(ts_inner.count, val_inner.count);

                    for (uint32_t k = 0; k < count; k++) {
                        float t = timestamps[k] / 1000.0f;
                        track.position_keys.push_back({t, {values[k].x, values[k].y, values[k].z}});
                    }
                }
            }

            // --- Rotation (compressed quaternion keyframes) ---
            // M2 stores rotations as uint16[4] (x, y, z, w).
            // Conversion: float = (uint16 / 65535.0) * 2.0 - 1.0
            // This maps [0, 65535] → [-1.0, 1.0]
            if (bone.rotation.timestamps.count > a &&
                boundsOk(bone.rotation.timestamps.offset, bone.rotation.timestamps.count, 8, data_size) &&
                boundsOk(bone.rotation.values.offset, bone.rotation.values.count, 8, data_size)) {

                auto& ts_inner = at<m2::M2Array<>>(data, bone.rotation.timestamps.offset)[a];
                auto& val_inner = at<m2::M2Array<>>(data, bone.rotation.values.offset)[a];

                if (ts_inner.count > 0 && val_inner.count > 0 &&
                    boundsOk(ts_inner.offset, ts_inner.count, 4, data_size) &&
                    boundsOk(val_inner.offset, val_inner.count, 8, data_size)) {

                    const auto* timestamps = at<uint32_t>(data, ts_inner.offset);
                    const auto* raw = at<uint16_t>(data, val_inner.offset);
                    uint32_t count = std::min(ts_inner.count, val_inner.count);

                    for (uint32_t k = 0; k < count; k++) {
                        float t = timestamps[k] / 1000.0f;

                        // Decompress: uint16 → float [-1, 1]
                        float x = (raw[k * 4 + 0] / 65535.0f) * 2.0f - 1.0f;
                        float y = (raw[k * 4 + 1] / 65535.0f) * 2.0f - 1.0f;
                        float z = (raw[k * 4 + 2] / 65535.0f) * 2.0f - 1.0f;
                        float w = (raw[k * 4 + 3] / 65535.0f) * 2.0f - 1.0f;

                        glm::quat q{w, x, y, z};
                        q = glm::normalize(q);
                        track.rotation_keys.push_back({t, q});
                    }
                }
            }

            // --- Scale (vec3 keyframes) ---
            if (bone.scaling.timestamps.count > a &&
                boundsOk(bone.scaling.timestamps.offset, bone.scaling.timestamps.count, 8, data_size) &&
                boundsOk(bone.scaling.values.offset, bone.scaling.values.count, 8, data_size)) {

                auto& ts_inner = at<m2::M2Array<>>(data, bone.scaling.timestamps.offset)[a];
                auto& val_inner = at<m2::M2Array<>>(data, bone.scaling.values.offset)[a];

                if (ts_inner.count > 0 && val_inner.count > 0 &&
                    boundsOk(ts_inner.offset, ts_inner.count, 4, data_size) &&
                    boundsOk(val_inner.offset, val_inner.count, 12, data_size)) {

                    const auto* timestamps = at<uint32_t>(data, ts_inner.offset);
                    const auto* values = at<m2::C3Vector>(data, val_inner.offset);
                    uint32_t count = std::min(ts_inner.count, val_inner.count);

                    for (uint32_t k = 0; k < count; k++) {
                        float t = timestamps[k] / 1000.0f;
                        track.scale_keys.push_back({t, {values[k].x, values[k].y, values[k].z}});
                    }
                }
            }

            bool has_data = !track.position_keys.empty() ||
                            !track.rotation_keys.empty() ||
                            !track.scale_keys.empty();

            if (has_data) {
                clip->SetBoneTrack(b, track);
            }
        }

        model.animations.push_back(std::move(clip));
    }
}

void M2Loader::parseSkin(const uint8_t* skin_data, uint32_t skin_size,
                          const uint8_t* m2_data, const m2::M2Header& header,
                          M2Model& model) {

    if (skin_size < sizeof(m2::M2SkinHeader)) {
        throw std::runtime_error("Skin file too small");
    }

    const auto& skin = *at<m2::M2SkinHeader>(skin_data, 0);

    if (skin.magic != m2::SKIN_MAGIC) {
        throw std::runtime_error("Invalid skin file magic");
    }

    // Skin file has its own vertex index list that maps into the M2's global vertex array
    const auto* skin_vertices = at<uint16_t>(skin_data, skin.vertices.offset);
    const auto* skin_indices = at<uint16_t>(skin_data, skin.indices.offset);
    const auto* m2_vertices = at<m2::M2Vertex>(m2_data, header.vertices.offset);

    // Convert M2 vertices referenced by this skin into our Vertex format
    model.vertices.resize(skin.vertices.count);

    for (uint32_t i = 0; i < skin.vertices.count; i++) {
        uint16_t global_idx = skin_vertices[i];
        const auto& src = m2_vertices[global_idx];

        Vertex& dst = model.vertices[i];
        dst.position = {src.position.x, src.position.y, src.position.z};
        dst.normal = {src.normal.x, src.normal.y, src.normal.z};
        dst.color = {1.0f, 1.0f, 1.0f};
        dst.uv = {src.tex_coords[0].x, src.tex_coords[0].y};

        // M2 vertex bone_indices are direct indices into the bone array
        // (NOT through the bone lookup table — that's for the skin's submesh batches)
        dst.bone_indices = {
            static_cast<uint32_t>(src.bone_indices[0]),
            static_cast<uint32_t>(src.bone_indices[1]),
            static_cast<uint32_t>(src.bone_indices[2]),
            static_cast<uint32_t>(src.bone_indices[3])
        };

        // Bone weights: M2 stores as uint8 (0-255), we normalize to float (0.0-1.0)
        dst.bone_weights = {
            src.bone_weights[0] / 255.0f,
            src.bone_weights[1] / 255.0f,
            src.bone_weights[2] / 255.0f,
            src.bone_weights[3] / 255.0f
        };
    }

    // Triangle indices
    model.indices.resize(skin.indices.count);
    for (uint32_t i = 0; i < skin.indices.count; i++) {
        model.indices[i] = skin_indices[i];
    }

    // Parse submeshes and batches
    const auto* submeshes = at<m2::M2SkinSection>(skin_data, skin.submeshes.offset);
    const auto* batches = at<m2::M2Batch>(skin_data, skin.batches.offset);

    for (uint32_t i = 0; i < skin.batches.count; i++) {
        const auto& batch = batches[i];
        const auto& section = submeshes[batch.skin_section_index];

        M2Submesh sub{};
        sub.index_start = section.index_start;
        sub.index_count = section.index_count;
        sub.material_index = batch.material_index;
        sub.shader_id = batch.shader_id;
        sub.texture_combo_index = batch.texture_combo_index;
        sub.texture_count = batch.texture_count;

        model.submeshes.push_back(sub);
    }
}

} // namespace mve
