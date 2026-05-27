#ifndef MVE_COMPONENT_STORE_H
#define MVE_COMPONENT_STORE_H

#include "entity_id.h"

#include <memory>
#include <tuple>
#include <unordered_map>

// All component types live here. This is the single compile-time list; the
// store holds one pool per type. Adding a new component type = add its header
// here and add it to the pm_pools tuple below.
#include "components/transform_component.h"
#include "components/mesh_component.h"
#include "components/camera_component.h"
#include "components/terrain_component.h"
#include "components/terrain_tile_component.h"
#include "components/wmo_instance_component.h"
#include "components/doodad_instance_component.h"
#include "components/water_component.h"
#include "components/skeleton_component.h"
#include "components/m2_info_component.h"
#include "components/material_component.h"

namespace mve {

// Per-type component pool. Components are stored as unique_ptr<T> (concrete,
// non-polymorphic) keyed by owning EntityId. This keeps per-component heap
// allocation (so component addresses are stable across adds/removes, matching
// the old behaviour) while being hot-reload-safe: no vtable pointers and no
// typeid/type_index keys, both of which would dangle after a module swap.
template<class T>
struct ComponentArray {
    std::unordered_map<EntityId, std::unique_ptr<T>> items;

    template<class... Args>
    T* add(EntityId e, Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = ptr.get();
        items[e] = std::move(ptr);   // replaces any existing component of T
        return raw;
    }
    T* get(EntityId e) {
        auto it = items.find(e);
        return it != items.end() ? it->second.get() : nullptr;
    }
    const T* get(EntityId e) const {
        auto it = items.find(e);
        return it != items.end() ? it->second.get() : nullptr;
    }
    bool has(EntityId e) const { return items.count(e) > 0; }
    void remove(EntityId e) { items.erase(e); }
};

// Owns every component pool. Pool selection is by C++ type at compile time
// (std::get<ComponentArray<T>>), so there is no runtime type lookup - nothing
// here depends on module-bound addresses, which is what makes a DLL reload
// survivable.
class ComponentStore {
public:
    template<class T, class... Args>
    T* Add(EntityId e, Args&&... args) {
        return pool<T>().add(e, std::forward<Args>(args)...);
    }
    template<class T> T* Get(EntityId e) { return pool<T>().get(e); }
    template<class T> const T* Get(EntityId e) const { return pool<T>().get(e); }
    template<class T> bool Has(EntityId e) const { return pool<T>().has(e); }
    template<class T> void Remove(EntityId e) { pool<T>().remove(e); }

    // Drop all of one entity's components (entity destroyed / tile evicted).
    void RemoveAll(EntityId e) {
        std::apply([e](auto&... arr) { (arr.remove(e), ...); }, pm_pools);
    }
    // Destroy every component (scene teardown). Must run while the systems
    // that own the GPU pools these components reference are still alive.
    void Clear() {
        std::apply([](auto&... arr) { (arr.items.clear(), ...); }, pm_pools);
    }

private:
    template<class T>       ComponentArray<T>& pool()       { return std::get<ComponentArray<T>>(pm_pools); }
    template<class T> const ComponentArray<T>& pool() const { return std::get<ComponentArray<T>>(pm_pools); }

    std::tuple<
        ComponentArray<TransformComponent>,
        ComponentArray<MeshComponent>,
        ComponentArray<CameraComponent>,
        ComponentArray<TerrainComponent>,
        ComponentArray<TerrainTileComponent>,
        ComponentArray<WmoInstanceComponent>,
        ComponentArray<DoodadInstanceComponent>,
        ComponentArray<WaterComponent>,
        ComponentArray<SkeletonComponent>,
        ComponentArray<M2InfoComponent>,
        ComponentArray<MaterialComponent>
    > pm_pools;
};

} // namespace mve

#endif // MVE_COMPONENT_STORE_H
