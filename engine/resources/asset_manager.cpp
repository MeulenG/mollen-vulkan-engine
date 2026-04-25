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

#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace mve {

AssetManager::AssetManager(Device& device)
    : pm_device{device} {}

void AssetManager::setDescriptorResources(
    const vk::raii::DescriptorSetLayout& layout,
    DescriptorPool& pool,
    const Buffer& scene_ubo) {
    pm_descriptor_layout = &layout;
    pm_descriptor_pool = &pool;
    pm_scene_ubo = &scene_ubo;
}

TextureHandle AssetManager::getDefaultTexture() {
    if (!pm_default_texture) {
        pm_default_texture = std::make_shared<Image>(Image::createCheckerboard(pm_device));
    }
    return pm_default_texture;
}

MeshHandle AssetManager::getMesh(const std::string& key) const {
    auto it = pm_mesh_cache.find(key);
    return it != pm_mesh_cache.end() ? it->second : nullptr;
}

TextureHandle AssetManager::getTexture(const std::string& key) const {
    auto it = pm_texture_cache.find(key);
    return it != pm_texture_cache.end() ? it->second : nullptr;
}

Entity* AssetManager::loadM2IntoScene(const std::string& m2_path, Scene& scene) {
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
        cache_entry->model = M2Loader::loadFile(m2_path);
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

    // Find a texture: first try referenced paths, then scan for BLPs in same directory
    if (!model.texture_paths.empty() && !model.texture_paths[0].empty()) {
        blp_path = "assets/" + model.texture_paths[0];
    } else {
        // Replaceable texture — scan directory for a BLP
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
                auto blp = BlpLoader::loadFile(blp_path);
                texture = std::make_shared<Image>(pm_device, blp);
                pm_texture_cache[blp_path] = texture;
            } catch (...) {
                texture = getDefaultTexture();
            }
        }
    }

    if (!texture) {
        texture = getDefaultTexture();
    }

    // 4. Create entity
    Entity* entity = scene.createEntity(model.name.empty() ? "M2Model" : model.name);

    // 5. TransformComponent with WoW coordinate conversion
    auto* transform = entity->addComponent<TransformComponent>();
    float ground_offset = -model.bbox_min.z;
    transform->applyWowCoordTransform(ground_offset);

    // 6. MeshComponent
    auto* mesh_comp = entity->addComponent<MeshComponent>();
    mesh_comp->pm_mesh = mesh;

    // 7. MaterialComponent with per-entity bone buffer and descriptor set
    auto* mat = entity->addComponent<MaterialComponent>();

    vk::DeviceSize bone_buffer_size = Skeleton::MAX_BONES * sizeof(glm::mat4);
    mat->pm_bone_buffer = std::make_unique<Buffer>(
        pm_device, bone_buffer_size,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    // Init bone buffer with identity
    std::vector<glm::mat4> identity(Skeleton::MAX_BONES, glm::mat4{1.0f});
    mat->pm_bone_buffer->write(identity.data(), bone_buffer_size);

    // Allocate descriptor set
    auto desc_set = pm_descriptor_pool->allocateSet(*pm_descriptor_layout);

    vk::DescriptorBufferInfo ubo_info{*pm_scene_ubo->buffer(), 0, sizeof(float) * 8}; // SceneUBO size
    auto tex_info = texture->descriptorInfo();
    vk::DescriptorBufferInfo bone_info{*mat->pm_bone_buffer->buffer(), 0, bone_buffer_size};

    DescriptorWriter{}
        .writeBuffer(0, ubo_info)
        .writeImage(1, tex_info)
        .writeBuffer(2, bone_info, vk::DescriptorType::eStorageBuffer)
        .apply(pm_device.device(), desc_set);

    mat->pm_descriptor_set = *desc_set;

    // Store one submesh material entry
    SubmeshMaterial sub_mat;
    sub_mat.pm_texture = texture;
    mat->pm_submesh_materials.push_back(sub_mat);

    // 8. SkeletonComponent
    if (model.skeleton.boneCount() > 0) {
        auto* skel = entity->addComponent<SkeletonComponent>();
        skel->pm_skeleton = &model.skeleton;
        skel->pm_animator = std::make_unique<Animator>(model.skeleton);

        for (auto& clip : model.animations) {
            skel->pm_clips.push_back(clip.get());
        }

        if (!skel->pm_clips.empty()) {
            skel->pm_animator->play(skel->pm_clips[0]);
        }
    }

    // 9. M2InfoComponent
    auto* info = entity->addComponent<M2InfoComponent>();
    info->pm_model_name = model.name;
    info->pm_vertex_count = static_cast<uint32_t>(model.vertices.size());
    info->pm_index_count = static_cast<uint32_t>(model.indices.size());
    info->pm_bone_count = model.skeleton.boneCount();
    info->pm_texture_paths = model.texture_paths;
    info->pm_submeshes = model.submeshes;
    info->pm_bbox_min = model.bbox_min;
    info->pm_bbox_max = model.bbox_max;
    info->pm_ground_offset = ground_offset;

    return entity;
}

} // namespace mve
