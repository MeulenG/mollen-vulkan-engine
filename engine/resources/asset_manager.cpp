#include "asset_manager.h"
#include "buffer.h"
#include "../scene/components/transform_component.h"
#include "../scene/components/mesh_component.h"
#include "../scene/components/material_component.h"
#include "../scene/components/skeleton_component.h"
#include "../scene/components/camera_component.h"
#include "../scene/components/m2_info_component.h"
#include "../formats/blp_loader.h"
#include "../animation/skeleton.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;

namespace mve {

namespace {

// Resolve a WoW-style asset path ("Tileset\\Elwynn\\..." with backslashes)
// against MVE_ASSET_DIR (a compile-time absolute path to the repo's
// assets/ directory). Backslashes become forward slashes; case is
// preserved (Windows is case-insensitive, so this is fine in practice).
std::string ResolveWowAsset(const std::string& wow_path) {
    std::string out = std::string(MVE_ASSET_DIR) + "/" + wow_path;
    std::replace(out.begin(), out.end(), '\\', '/');
    return out;
}

// log2(blp_w / target) - how many mip levels to skip when feeding this
// BLP into an array with target-width slices. Returns -1 if the BLP is
// too small or not a power-of-two multiple.
int BlpMipForTarget(uint32_t blp_w, uint32_t target) {
    if (blp_w < target) return -1;
    uint32_t w = blp_w;
    int skip = 0;
    while (w > target) {
        if (w & 1u) return -1;
        w >>= 1;
        skip++;
    }
    return skip;
}

} // namespace

AssetManager::AssetManager(Device& device)
    : pm_device{device} {}

void AssetManager::SetDescriptorResources(
    const vk::raii::DescriptorSetLayout& layout,
    DescriptorPool& pool,
    const Buffer& scene_ubo) {
    pm_descriptor_layout = &layout;
    pm_descriptor_pool = &pool;
    pm_scene_ubo = &scene_ubo;
}

TextureHandle AssetManager::GetDefaultTexture() {
    if (!pm_default_texture) {
        pm_default_texture = std::make_shared<Image>(Image::CreateCheckerboard(pm_device));
    }
    return pm_default_texture;
}

MeshHandle AssetManager::GetMesh(const std::string& key) const {
    auto it = pm_mesh_cache.find(key);
    return it != pm_mesh_cache.end() ? it->second : nullptr;
}

TextureHandle AssetManager::GetTexture(const std::string& key) const {
    auto it = pm_texture_cache.find(key);
    return it != pm_texture_cache.end() ? it->second : nullptr;
}

Entity* AssetManager::LoadM2IntoScene(const std::string& m2_path, Scene& scene) {
    if (!pm_descriptor_layout || !pm_descriptor_pool || !pm_scene_ubo) {
        return nullptr;
    }

    // Parse M2 (or fetch from cache)
    std::shared_ptr<M2CacheEntry> cache_entry;
    auto cache_it = pm_m2_cache.find(m2_path);
    if (cache_it != pm_m2_cache.end()) {
        cache_entry = cache_it->second;
    } else {
        if (!fs::exists(m2_path)) return nullptr;

        cache_entry = std::make_shared<M2CacheEntry>();
        cache_entry->model = M2Loader::LoadFile(m2_path);
        pm_m2_cache[m2_path] = cache_entry;
    }

    auto& model = cache_entry->model;
    if (model.vertices.empty()) return nullptr;

    // Create or fetch shared mesh
    MeshHandle mesh;
    auto mesh_it = pm_mesh_cache.find(m2_path);
    if (mesh_it != pm_mesh_cache.end()) {
        mesh = mesh_it->second;
    } else {
        mesh = std::make_shared<Mesh>(pm_device, model.vertices, model.indices);
        pm_mesh_cache[m2_path] = mesh;
    }

    // Load texture
    TextureHandle texture;
    std::string blp_path;

    if (!model.texture_paths.empty() && !model.texture_paths[0].empty()) {
        blp_path = "assets/" + model.texture_paths[0];
    } else {
        fs::path m2_dir = fs::path(m2_path).parent_path();
        for (auto& entry : fs::directory_iterator(m2_dir)) {
            auto ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".blp" && entry.path().stem().string().find("Skin") != std::string::npos) {
                blp_path = entry.path().string();
                break;
            }
        }
    }

    if (!blp_path.empty()) {
        auto tex_it = pm_texture_cache.find(blp_path);
        if (tex_it != pm_texture_cache.end()) {
            texture = tex_it->second;
        } else if (fs::exists(blp_path)) {
            try {
                auto blp = BlpLoader::LoadFile(blp_path);
                texture = std::make_shared<Image>(pm_device, blp);
                pm_texture_cache[blp_path] = texture;
            } catch (...) {
                texture = GetDefaultTexture();
            }
        }
    }

    if (!texture) {
        texture = GetDefaultTexture();
    }

    // Create entity
    Entity* entity = scene.CreateEntity(model.name.empty() ? "M2Model" : model.name);

    // TransformComponent with WoW coordinate conversion
    auto* transform = entity->AddComponent<TransformComponent>();
    float ground_offset = -model.bbox_min.z;
    transform->ApplyWowCoordTransform(ground_offset);

    // MeshComponent
    auto* mesh_comp = entity->AddComponent<MeshComponent>();
    mesh_comp->pm_mesh = mesh;

    // MaterialComponent with per-entity bone buffer and descriptor set
    auto* mat = entity->AddComponent<MaterialComponent>();

    vk::DeviceSize bone_buffer_size = Skeleton::MAX_BONES * sizeof(glm::mat4);
    mat->pm_bone_buffer = std::make_unique<Buffer>(
        pm_device, bone_buffer_size,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    std::vector<glm::mat4> identity(Skeleton::MAX_BONES, glm::mat4{1.0f});
    mat->pm_bone_buffer->Write(identity.data(), bone_buffer_size);

    // Move the RAII descriptor set into the component so its lifetime
    // matches the entity's, not this function's stack frame.
    mat->pm_descriptor_set = pm_descriptor_pool->AllocateSet(*pm_descriptor_layout);

    vk::DescriptorBufferInfo ubo_info{*pm_scene_ubo->GetBuffer(), 0, sizeof(float) * 8};
    auto tex_info = texture->DescriptorInfo();
    vk::DescriptorBufferInfo bone_info{*mat->pm_bone_buffer->GetBuffer(), 0, bone_buffer_size};

    DescriptorWriter{}
        .WriteBuffer(0, ubo_info)
        .WriteImage(1, tex_info)
        .WriteBuffer(2, bone_info, vk::DescriptorType::eStorageBuffer)
        .Apply(pm_device.GetDevice(), mat->pm_descriptor_set);

    SubmeshMaterial sub_mat;
    sub_mat.pm_texture = texture;
    mat->pm_submesh_materials.push_back(sub_mat);

    // SkeletonComponent
    if (model.skeleton.BoneCount() > 0) {
        auto* skel = entity->AddComponent<SkeletonComponent>();
        skel->pm_skeleton = &model.skeleton;
        skel->pm_animator = std::make_unique<Animator>(model.skeleton);

        for (auto& clip : model.animations) {
            skel->pm_clips.push_back(clip.get());
        }

        if (!skel->pm_clips.empty()) {
            skel->pm_animator->Play(skel->pm_clips[0]);
        }
    }

    // M2InfoComponent
    auto* info = entity->AddComponent<M2InfoComponent>();
    info->pm_model_name = model.name;
    info->pm_vertex_count = static_cast<uint32_t>(model.vertices.size());
    info->pm_index_count = static_cast<uint32_t>(model.indices.size());
    info->pm_bone_count = model.skeleton.BoneCount();
    info->pm_texture_paths = model.texture_paths;
    info->pm_submeshes = model.submeshes;
    info->pm_bbox_min = model.bbox_min;
    info->pm_bbox_max = model.bbox_max;
    info->pm_ground_offset = ground_offset;

    return entity;
}

AdtTextureSet AssetManager::LoadAdtTextures(const AdtTile& tile,
                                            uint32_t target_size) {
    AdtTextureSet result{};
    result.tile_tex_to_slice.assign(tile.textures.size(), -1);
    if (tile.textures.empty()) return result;

    // Two-pass approach:
    //   Pass 1: parse every BLP, decide on a canonical format (the one
    //           used by the first valid BLP that meets our size and
    //           power-of-two requirements). Skip the rest.
    //   Pass 2: allocate an array of N slices and upload the survivors.
    //
    // WoW terrain BLPs are almost universally BC1 (DXT1) without alpha;
    // we accept BC3 as a fallback so a tile that mixes one BC3 BLP
    // doesn't lose every texture, but only the first-format slices
    // upload successfully. Mismatched BLPs end up at slice -1 ->
    // caller substitutes a checkerboard.
    struct Candidate {
        size_t tile_index;
        std::string path;
        BlpTexture blp;
        bool ok = false;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(tile.textures.size());

    vk::Format chosen_format = vk::Format::eUndefined;

    for (size_t i = 0; i < tile.textures.size(); i++) {
        Candidate c;
        c.tile_index = i;
        c.path = ResolveWowAsset(tile.textures[i]);
        if (!fs::exists(c.path)) {
            std::fprintf(stderr,
                "ADT texture missing: %s\n", c.path.c_str());
            candidates.push_back(std::move(c));
            continue;
        }
        try {
            c.blp = BlpLoader::LoadFile(c.path);
        } catch (const std::exception& e) {
            std::fprintf(stderr,
                "BLP parse failed (%s): %s\n", c.path.c_str(), e.what());
            candidates.push_back(std::move(c));
            continue;
        }

        // We need a BLP whose dimensions are a power-of-two multiple of
        // target_size and whose format is a supported block format.
        // Most WoW terrain BLPs are 256x256 BC1, so this check passes
        // trivially; large 1024x1024 ground BLPs hit the mip-skip path.
        int skip = BlpMipForTarget(c.blp.width, target_size);
        if (skip < 0) {
            std::fprintf(stderr,
                "BLP %s has unsupported size %u for target %u\n",
                c.path.c_str(), c.blp.width, target_size);
            candidates.push_back(std::move(c));
            continue;
        }

        if (chosen_format == vk::Format::eUndefined) {
            chosen_format = c.blp.format;
        }
        c.ok = (c.blp.format == chosen_format);
        if (!c.ok) {
            std::fprintf(stderr,
                "BLP %s format mismatch (got %u, expected %u)\n",
                c.path.c_str(),
                static_cast<unsigned>(c.blp.format),
                static_cast<unsigned>(chosen_format));
        }
        candidates.push_back(std::move(c));
    }

    if (chosen_format == vk::Format::eUndefined) {
        // No usable BLPs at all. Leave result.diffuse null - caller
        // will see the empty array as "all slices missing" and fall
        // back to the per-fragment "magenta missing" path.
        return result;
    }

    // Count successful candidates so we know how many slices to allocate.
    uint32_t slice_count = 0;
    for (const auto& c : candidates) if (c.ok) slice_count++;
    if (slice_count == 0) return result;

    // Mip levels: log2(target_size) - 1 (stop at 4x4 for BC formats).
    uint32_t mip_levels = 1;
    {
        uint32_t s = target_size;
        while (s > 4 && mip_levels < 12) {
            s >>= 1;
            mip_levels++;
        }
    }

    result.diffuse = std::make_unique<TextureArray>(
        pm_device, target_size, target_size,
        slice_count, chosen_format, mip_levels);

    uint32_t next_slice = 0;
    for (const auto& c : candidates) {
        if (!c.ok) continue;
        if (result.diffuse->UploadSliceFromBlp(next_slice, c.blp)) {
            result.tile_tex_to_slice[c.tile_index] = static_cast<int>(next_slice);
            next_slice++;
        }
    }
    result.diffuse->FinalizeForSampling();
    return result;
}

} // namespace mve
