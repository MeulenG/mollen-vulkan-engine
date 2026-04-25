#include "entity.h"

namespace mve {

Entity::Entity(EntityId id, Scene& scene, const std::string& name)
    : pm_id{id}, pm_scene{scene}, pm_name{name} {}

} // namespace mve
