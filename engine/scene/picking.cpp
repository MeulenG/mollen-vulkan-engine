#include "picking.h"

#include "scene.h"
#include "camera.h"
#include "components/transform_component.h"
#include "components/m2_info_component.h"
#include "components/wmo_instance_component.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <algorithm>
#include <limits>

namespace mve {

namespace {

// Slab-method ray vs axis-aligned box. Writes the entry parameter to t_hit
// if there's a forward intersection.
bool RayAabb(const glm::vec3& orig, const glm::vec3& dir,
             const glm::vec3& bmin, const glm::vec3& bmax, float& t_hit) {
    float t_near = -std::numeric_limits<float>::infinity();
    float t_far  =  std::numeric_limits<float>::infinity();
    for (int i = 0; i < 3; ++i) {
        if (std::abs(dir[i]) < 1e-8f) {
            // Ray parallel to slab: must be inside the slab to hit.
            if (orig[i] < bmin[i] || orig[i] > bmax[i]) return false;
        } else {
            float inv = 1.0f / dir[i];
            float t1 = (bmin[i] - orig[i]) * inv;
            float t2 = (bmax[i] - orig[i]) * inv;
            if (t1 > t2) std::swap(t1, t2);
            t_near = std::max(t_near, t1);
            t_far  = std::min(t_far,  t2);
            if (t_near > t_far) return false;
        }
    }
    if (t_far < 0.0f) return false;            // box entirely behind ray
    t_hit = t_near > 0.0f ? t_near : t_far;    // first hit in front
    return true;
}

// Transform an axis-aligned box by a 4x4 matrix and return the AABB of the
// transformed 8 corners. Conservative when M includes rotation; for our
// editor pick that's fine.
void TransformAabb(const glm::mat4& m, const glm::vec3& bmin,
                   const glm::vec3& bmax, glm::vec3& out_min,
                   glm::vec3& out_max) {
    glm::vec3 corners[8] = {
        {bmin.x, bmin.y, bmin.z}, {bmax.x, bmin.y, bmin.z},
        {bmin.x, bmax.y, bmin.z}, {bmax.x, bmax.y, bmin.z},
        {bmin.x, bmin.y, bmax.z}, {bmax.x, bmin.y, bmax.z},
        {bmin.x, bmax.y, bmax.z}, {bmax.x, bmax.y, bmax.z},
    };
    out_min = glm::vec3{ std::numeric_limits<float>::infinity()};
    out_max = glm::vec3{-std::numeric_limits<float>::infinity()};
    for (const auto& c : corners) {
        glm::vec3 w = glm::vec3(m * glm::vec4(c, 1.0f));
        out_min = glm::min(out_min, w);
        out_max = glm::max(out_max, w);
    }
}

} // namespace

EntityId PickEntity(Scene& scene, const Camera& cam,
                    float screen_x, float screen_y,
                    float image_w, float image_h) {
    if (image_w <= 0.0f || image_h <= 0.0f) return NULL_ENTITY;

    // Convert pixel -> NDC. Vulkan NDC after the projection's Y-flip already
    // matches screen-Y-down semantics, so the same `2*sy/h - 1` formula
    // unprojects correctly.
    float ndc_x = 2.0f * (screen_x / image_w) - 1.0f;
    float ndc_y = 2.0f * (screen_y / image_h) - 1.0f;

    glm::mat4 view_proj = cam.GetProjectionMatrix() * cam.GetViewMatrix();
    glm::mat4 inv_vp    = glm::inverse(view_proj);

    glm::vec4 p_near = inv_vp * glm::vec4(ndc_x, ndc_y, 0.0f, 1.0f);
    glm::vec4 p_far  = inv_vp * glm::vec4(ndc_x, ndc_y, 1.0f, 1.0f);
    p_near /= p_near.w;
    p_far  /= p_far.w;

    glm::vec3 ray_o = glm::vec3(p_near);
    glm::vec3 ray_d = glm::normalize(glm::vec3(p_far) - ray_o);

    EntityId best_id = NULL_ENTITY;
    float    best_t  = std::numeric_limits<float>::infinity();

    // M2 entities: TransformComponent + M2InfoComponent. World AABB =
    // ModelMatrix * local bbox (corner-transformed).
    scene.Each<TransformComponent, M2InfoComponent>(
        [&](Entity& e, TransformComponent& tf, M2InfoComponent& info) {
            // Skip M2s with no bbox loaded (degenerate or missing).
            if (info.pm_bbox_min == info.pm_bbox_max) return;
            glm::vec3 wmin, wmax;
            TransformAabb(tf.ModelMatrix(), info.pm_bbox_min,
                          info.pm_bbox_max, wmin, wmax);
            float t;
            if (RayAabb(ray_o, ray_d, wmin, wmax, t) && t < best_t) {
                best_t  = t;
                best_id = e.Id();
            }
        });

    // WMO instances: aggregate per-group bboxes in WMO-local space, then
    // transform by the stored model_matrix.
    scene.Each<WmoInstanceComponent>(
        [&](Entity& e, WmoInstanceComponent& wi) {
            if (wi.group_bbox_min.empty()) return;
            glm::vec3 lmin{ std::numeric_limits<float>::infinity()};
            glm::vec3 lmax{-std::numeric_limits<float>::infinity()};
            for (size_t i = 0; i < wi.group_bbox_min.size(); ++i) {
                lmin = glm::min(lmin, glm::vec3(wi.group_bbox_min[i]));
                lmax = glm::max(lmax, glm::vec3(wi.group_bbox_max[i]));
            }
            glm::vec3 wmin, wmax;
            TransformAabb(wi.model_matrix, lmin, lmax, wmin, wmax);
            float t;
            if (RayAabb(ray_o, ray_d, wmin, wmax, t) && t < best_t) {
                best_t  = t;
                best_id = e.Id();
            }
        });

    return best_id;
}

} // namespace mve
