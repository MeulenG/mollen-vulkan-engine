#ifndef MVE_ENTITY_ID_H
#define MVE_ENTITY_ID_H

#include <cstdint>

namespace mve {

// Stable entity handle. Kept in its own header so the component store and
// the Entity class can both depend on it without an include cycle (Entity
// includes the store; the store keys its pools by EntityId).
using EntityId = uint32_t;
constexpr EntityId NULL_ENTITY = 0;

} // namespace mve

#endif // MVE_ENTITY_ID_H
