#ifndef MVE_ENTITY_H
#define MVE_ENTITY_H

#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace mve {

class Scene;

struct Component {
    virtual ~Component() = default;
};

using EntityId = uint32_t;
constexpr EntityId NULL_ENTITY = 0;

class Entity {
public:
    Entity(EntityId id, Scene& scene, const std::string& name = "Entity");

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;

    EntityId id() const { return pm_id; }
    const std::string& name() const { return pm_name; }
    void SetName(const std::string& name) { pm_name = name; }
    Scene& scene() { return pm_scene; }

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = ptr.get();
        pm_components[std::type_index(typeid(T))] = std::move(ptr);
        return raw;
    }

    template<typename T>
    T* GetComponent() {
        auto it = pm_components.find(std::type_index(typeid(T)));
        return it != pm_components.end() ? static_cast<T*>(it->second.get()) : nullptr;
    }

    template<typename T>
    const T* GetComponent() const {
        auto it = pm_components.find(std::type_index(typeid(T)));
        return it != pm_components.end() ? static_cast<const T*>(it->second.get()) : nullptr;
    }

    template<typename T>
    bool HasComponent() const {
        return pm_components.count(std::type_index(typeid(T))) > 0;
    }

    template<typename T>
    void RemoveComponent() {
        pm_components.erase(std::type_index(typeid(T)));
    }

private:
    EntityId pm_id;
    Scene& pm_scene;
    std::string pm_name;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> pm_components;
};

} // namespace mve

#endif // MVE_ENTITY_H
