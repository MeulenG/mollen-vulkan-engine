#include "entity.h"

namespace mve {

Entity::Entity(EntityId id, Scene& scene, ComponentStore& store,
               const std::string& name)
    : pm_id{id}, pm_scene{scene}, pm_store{&store}, pm_name{name} {}

} // namespace mve
