#ifndef MVE_PICKING_H
#define MVE_PICKING_H

#include "entity_id.h"

namespace mve {

class Scene;
class Camera;

// Pick the entity nearest the camera along a ray cast from the given pixel
// of the viewport. screen_x/y are pixel coordinates within the rendered
// viewport image (top-left origin); image_w/h are the image's size.
// Candidates: any entity with TransformComponent + M2InfoComponent (an M2
// model) or any entity with WmoInstanceComponent. Terrain tiles and instanced
// doodad groups are excluded for v1 (their per-instance picking needs
// separate work). Returns NULL_ENTITY when the ray misses everything.
EntityId PickEntity(Scene& scene, const Camera& cam,
                    float screen_x, float screen_y,
                    float image_w, float image_h);

} // namespace mve

#endif // MVE_PICKING_H
