#ifndef MVE_SCENE_H
#define MVE_SCENE_H

#include "entity.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace mve {

class Scene {
public:
    Scene() = default;

    Entity* CreateEntity(const std::string& name = "Entity");
    void DestroyEntity(EntityId id);
    void FlushDestroyed();

    // Tears down every entity in the scene immediately. Used at shutdown
    // to release per-entity GPU resources (descriptor sets, buffers,
    // images) while the systems that own the pools they belong to are
    // still alive. Without this, automatic destruction races: the
    // RenderSystem (and its DescriptorPool) may go away before the
    // scene's entities, leaving vk::raii::DescriptorSet destructors
    // calling vkFreeDescriptorSets against a freed pool handle.
    void Clear();

    Entity* FindEntity(EntityId id);
    Entity* FindEntityByName(const std::string& name);
    const std::vector<std::unique_ptr<Entity>>& Entities() const { return pm_entities; }

    template<typename... Comps, typename Func>
    void Each(Func&& func) {
        for (auto& entity : pm_entities) {
            if ((entity->HasComponent<Comps>() && ...)) {
                func(*entity, *entity->GetComponent<Comps>()...);
            }
        }
    }

    EntityId SelectedEntity() const { return pm_selected; }
    void SelectEntity(EntityId id) { pm_selected = id; }
    void ClearSelection() { pm_selected = NULL_ENTITY; }

private:
    std::vector<std::unique_ptr<Entity>> pm_entities;
    std::vector<EntityId> pm_pending_destroy;
    EntityId pm_next_id = 1;
    EntityId pm_selected = NULL_ENTITY;
};

} // namespace mve

#endif // MVE_SCENE_H
