#include "scene.h"

#include <algorithm>

namespace mve {

Entity* Scene::CreateEntity(const std::string& name) {
    auto entity = std::make_unique<Entity>(pm_next_id++, *this, pm_store, name);
    Entity* raw = entity.get();
    pm_entities.push_back(std::move(entity));
    return raw;
}

void Scene::DestroyEntity(EntityId id) {
    pm_pending_destroy.push_back(id);
}

void Scene::FlushDestroyed() {
    if (pm_pending_destroy.empty()) return;

    for (EntityId id : pm_pending_destroy) {
        // Drop this entity's components first so their GPU resources are
        // released while the owning systems/pools are still alive.
        pm_store.RemoveAll(id);

        pm_entities.erase(
            std::remove_if(pm_entities.begin(), pm_entities.end(),
                [id](const std::unique_ptr<Entity>& e) { return e->Id() == id; }),
            pm_entities.end());

        if (pm_selected == id) {
            pm_selected = NULL_ENTITY;
        }
    }

    pm_pending_destroy.clear();
}

void Scene::Clear() {
    // Destroy all components (and the GPU resources they hold) before the
    // systems that own those GPU pools are torn down. Then drop the entities.
    pm_store.Clear();
    pm_entities.clear();
    pm_pending_destroy.clear();
    pm_selected = NULL_ENTITY;
}

Entity* Scene::FindEntity(EntityId id) {
    for (auto& entity : pm_entities) {
        if (entity->Id() == id) return entity.get();
    }
    return nullptr;
}

Entity* Scene::FindEntityByName(const std::string& name) {
    for (auto& entity : pm_entities) {
        if (entity->Name() == name) return entity.get();
    }
    return nullptr;
}

} // namespace mve
