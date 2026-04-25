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

    Entity* createEntity(const std::string& name = "Entity");
    void destroyEntity(EntityId id);
    void flushDestroyed();

    Entity* findEntity(EntityId id);
    Entity* findEntityByName(const std::string& name);
    const std::vector<std::unique_ptr<Entity>>& entities() const { return pm_entities; }

    // Query: iterate all entities that have ALL of the specified component types.
    // Uses C++17 fold expression to check every type in the pack.
    template<typename... Comps, typename Func>
    void each(Func&& func) {
        for (auto& entity : pm_entities) {
            if ((entity->hasComponent<Comps>() && ...)) {
                func(*entity, *entity->getComponent<Comps>()...);
            }
        }
    }

    EntityId selectedEntity() const { return pm_selected; }
    void selectEntity(EntityId id) { pm_selected = id; }
    void clearSelection() { pm_selected = NULL_ENTITY; }

private:
    std::vector<std::unique_ptr<Entity>> pm_entities;
    std::vector<EntityId> pm_pending_destroy;
    EntityId pm_next_id = 1;
    EntityId pm_selected = NULL_ENTITY;
};

} // namespace mve

#endif // MVE_SCENE_H
