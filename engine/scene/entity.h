#ifndef MVE_ENTITY_H
#define MVE_ENTITY_H

#include "entity_id.h"
#include "component_store.h"

#include <string>

namespace mve {

class Scene;

// An Entity is now just an identity (id + name) plus forwarding helpers. It
// no longer owns its components - those live in the Scene's ComponentStore,
// keyed by EntityId. The template API below is unchanged from before, so all
// existing call sites (entity->AddComponent<T>(), GetComponent<T>(), etc.)
// keep working; only the backing storage moved.
class Entity {
public:
    Entity(EntityId id, Scene& scene, ComponentStore& store,
           const std::string& name = "Entity");

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    EntityId Id() const { return pm_id; }
    const std::string& Name() const { return pm_name; }
    void SetName(const std::string& name) { pm_name = name; }
    Scene& GetScene() { return pm_scene; }

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        return pm_store->Add<T>(pm_id, std::forward<Args>(args)...);
    }

    template<typename T>
    T* GetComponent() { return pm_store->Get<T>(pm_id); }

    template<typename T>
    const T* GetComponent() const { return pm_store->Get<T>(pm_id); }

    template<typename T>
    bool HasComponent() const { return pm_store->Has<T>(pm_id); }

    template<typename T>
    void RemoveComponent() { pm_store->Remove<T>(pm_id); }

private:
    EntityId pm_id;
    Scene& pm_scene;
    ComponentStore* pm_store;
    std::string pm_name;
};

} // namespace mve

#endif // MVE_ENTITY_H
