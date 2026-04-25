#include "scene.h"

#include <algorithm>

namespace mve {

Entity* Scene::createEntity(const std::string& name) {
    auto entity = std::make_unique<Entity>(pm_next_id++, *this, name);
    Entity* raw = entity.get();
    pm_entities.push_back(std::move(entity));
    return raw;
}

void Scene::destroyEntity(EntityId id) {
    pm_pending_destroy.push_back(id);
}

void Scene::flushDestroyed() {
    if (pm_pending_destroy.empty()) return;

    for (EntityId id : pm_pending_destroy) {
        pm_entities.erase(
            std::remove_if(pm_entities.begin(), pm_entities.end(),
                [id](const std::unique_ptr<Entity>& e) { return e->id() == id; }),
            pm_entities.end());

        if (pm_selected == id) {
            pm_selected = NULL_ENTITY;
        }
    }

    pm_pending_destroy.clear();
}

Entity* Scene::findEntity(EntityId id) {
    for (auto& entity : pm_entities) {
        if (entity->id() == id) return entity.get();
    }
    return nullptr;
}

Entity* Scene::findEntityByName(const std::string& name) {
    for (auto& entity : pm_entities) {
        if (entity->name() == name) return entity.get();
    }
    return nullptr;
}

} // namespace mve
