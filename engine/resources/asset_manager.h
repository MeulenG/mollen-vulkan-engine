#ifndef MVE_ASSET_MANAGER_H
#define MVE_ASSET_MANAGER_H

#include "../core/device.h"
#include "../scene/mesh.h"
#include "../scene/scene.h"
#include "../resources/image.h"
#include "../resources/descriptor.h"
#include "../formats/m2_loader.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace mve {

using MeshHandle = std::shared_ptr<Mesh>;
using TextureHandle = std::shared_ptr<Image>;

class AssetManager {
public:
    AssetManager(Device& device);

    // Load an M2 model and spawn a fully-wired entity in the scene.
    // Caches M2 data and textures so loading the same model twice shares GPU resources.
    Entity* loadM2IntoScene(const std::string& m2_path, Scene& scene);

    // Set the descriptor layout + pool that entities will use.
    // Must be called before loadM2IntoScene.
    void setDescriptorResources(
        const vk::raii::DescriptorSetLayout& layout,
        DescriptorPool& pool,
        const Buffer& scene_ubo);

    MeshHandle getMesh(const std::string& key) const;
    TextureHandle getTexture(const std::string& key) const;
    TextureHandle getDefaultTexture();

private:
    Device& pm_device;

    // Cached parsed M2 data (skeleton + animations shared across instances)
    struct M2CacheEntry {
        M2Model model;
    };
    std::unordered_map<std::string, std::shared_ptr<M2CacheEntry>> pm_m2_cache;
    std::unordered_map<std::string, MeshHandle> pm_mesh_cache;
    std::unordered_map<std::string, TextureHandle> pm_texture_cache;
    TextureHandle pm_default_texture;

    // Descriptor resources (set externally by RenderSystem or main)
    const vk::raii::DescriptorSetLayout* pm_descriptor_layout = nullptr;
    DescriptorPool* pm_descriptor_pool = nullptr;
    const Buffer* pm_scene_ubo = nullptr;
};

} // namespace mve

#endif // MVE_ASSET_MANAGER_H
