#include "asset_manager.h"
#include "buffer.h"
#include "../scene/components/transform_component.h"
#include "../scene/components/mesh_component.h"
#include "../scene/components/material_component.h"
#include "../scene/components/skeleton_component.h"
#include "../scene/components/camera_component.h"
#include "../scene/components/m2_info_component.h"
#include "../scene/components/terrain_component.h"
#include "../scene/components/terrain_tile_component.h"
#include "../scene/components/doodad_instance_component.h"
#include "../scene/components/water_component.h"
#include "../scene/components/wmo_instance_component.h"
#include "../scene/terrain_mesh.h"
#include "../scene/water_mesh.h"
#include "../scene/wmo_mesh.h"
#include "../formats/wmo_types.h"
#include "../systems/render_system.h"
#include "../formats/blp_loader.h"
#include "../formats/adt_types.h"
#include "../animation/skeleton.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;

namespace mve {

namespace {

// Resolve a WoW-style asset path ("Tileset\\Elwynn\\..." with backslashes)
// against MVE_ASSET_DIR (a compile-time absolute path to the repo's
// assets/ directory). Backslashes become forward slashes; case is
// preserved (Windows is case-insensitive, so this is fine in practice).
std::string ResolveWowAsset(const std::string& wow_path) {
    std::string out = std::string(MVE_ASSET_DIR) + "/" + wow_path;
    std::replace(out.begin(), out.end(), '\\', '/');
    return out;
}

// log2(blp_w / target) - how many mip levels to skip when feeding this
// BLP into an array with target-width slices. Returns -1 if the BLP is
// too small or not a power-of-two multiple.
int BlpMipForTarget(uint32_t blp_w, uint32_t target) {
    if (blp_w < target) return -1;
    uint32_t w = blp_w;
    int skip = 0;
    while (w > target) {
        if (w & 1u) return -1;
        w >>= 1;
        skip++;
    }
    return skip;
}

// MMDX path strings often end in the legacy ".mdx" extension even
// though the file actually extracted from MPQ is ".m2". If the
// requested .mdx doesn't exist, try the .m2 variant transparently
// so the user doesn't have to rewrite every doodad path.
std::string ResolveM2Extension(const std::string& path) {
    if (fs::exists(path)) return path;
    if (path.size() < 4) return path;
    std::string ext = path.substr(path.size() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".mdx") {
        std::string m2 = path.substr(0, path.size() - 4) + ".m2";
        if (fs::exists(m2)) return m2;
    }
    return path;
}

// Find a usable BLP texture for an M2 model. Tries (in order):
//   1. Every entry in model.texture_paths that's non-empty, resolved
//      via ResolveWowAsset.
//   2. The M2's filename with .blp extension (e.g. OakTree.m2 -> OakTree.blp).
//   3. Any *.blp in the M2's directory (alphabetical first match).
//
// Returns empty string if none of these resolve to an existing file.
//
// Why we need all three: WoW M2 textures have a `type` field. type 0
// means "hardcoded path" (texture_paths[i] non-empty). Types 1+ are
// "replaceable" textures (character skin, item, etc.) where the engine
// is supposed to substitute. For doodads we don't have a substitution
// system, so we fall back to the filename heuristic which works for
// most prop M2s that ship with a single-texture-per-folder convention.
std::string FindM2BlpPath(const std::string& m2_fs_path,
                          const M2Model& model) {
    // 1. Try texture_paths entries via MVE_ASSET_DIR
    for (const auto& wow_path : model.texture_paths) {
        if (wow_path.empty()) continue;
        std::string fs_path = ResolveWowAsset(wow_path);
        if (fs::exists(fs_path)) return fs_path;
    }

    // 2. Try the M2's filename stem with .blp extension
    fs::path m2_path{m2_fs_path};
    fs::path candidate = m2_path.parent_path() / (m2_path.stem().string() + ".blp");
    if (fs::exists(candidate)) return candidate.string();

    // 3. Walk the M2's directory and return the first .blp we find.
    // Alphabetical order via directory_iterator + manual sort would be
    // ideal but slow; for v1 we accept whatever iteration order returns.
    fs::path m2_dir = m2_path.parent_path();
    if (fs::exists(m2_dir)) {
        for (auto& entry : fs::directory_iterator(m2_dir)) {
            auto ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".blp") return entry.path().string();
        }
    }

    return {};
}

} // namespace

AssetManager::AssetManager(Device& device)
    : pm_device{device} {}

void AssetManager::SetDescriptorResources(
    const vk::raii::DescriptorSetLayout& layout,
    DescriptorPool& pool,
    const Buffer& scene_ubo) {
    pm_descriptor_layout = &layout;
    pm_descriptor_pool = &pool;
    pm_scene_ubo = &scene_ubo;
}

TextureHandle AssetManager::GetDefaultTexture() {
    if (!pm_default_texture) {
        pm_default_texture = std::make_shared<Image>(Image::CreateCheckerboard(pm_device));
    }
    return pm_default_texture;
}

MeshHandle AssetManager::GetMesh(const std::string& key) const {
    auto it = pm_mesh_cache.find(key);
    return it != pm_mesh_cache.end() ? it->second : nullptr;
}

TextureHandle AssetManager::GetTexture(const std::string& key) const {
    auto it = pm_texture_cache.find(key);
    return it != pm_texture_cache.end() ? it->second : nullptr;
}

Entity* AssetManager::LoadM2IntoScene(const std::string& m2_path, Scene& scene) {
    return LoadM2IntoScene(m2_path, scene,
                            glm::vec3{0.0f},
                            glm::quat{1.0f, 0.0f, 0.0f, 0.0f},
                            glm::vec3{1.0f},
                            "");
}

Entity* AssetManager::LoadM2IntoScene(
    const std::string& m2_path_in, Scene& scene,
    const glm::vec3& position,
    const glm::quat& rotation,
    const glm::vec3& scale,
    const std::string& name_hint) {

    if (!pm_descriptor_layout || !pm_descriptor_pool || !pm_scene_ubo) {
        return nullptr;
    }

    // MMDX paths in WoW are often the legacy ".mdx" extension; the
    // actual file extracted from MPQ is ".m2". Try the swap before
    // bailing on a missing file.
    std::string m2_path = ResolveM2Extension(m2_path_in);

    // Parse M2 (or fetch from cache). The cache is keyed by the
    // resolved path so .m2 / .mdx variants share one entry.
    std::shared_ptr<M2CacheEntry> cache_entry;
    auto cache_it = pm_m2_cache.find(m2_path);
    if (cache_it != pm_m2_cache.end()) {
        cache_entry = cache_it->second;
    } else {
        if (!fs::exists(m2_path)) return nullptr;

        try {
            cache_entry = std::make_shared<M2CacheEntry>();
            cache_entry->model = M2Loader::LoadFile(m2_path);
        } catch (...) {
            // Corrupt M2 or unsupported version. Don't insert into
            // cache so a retry might succeed if the file is fixed.
            return nullptr;
        }
        pm_m2_cache[m2_path] = cache_entry;
    }

    auto& model = cache_entry->model;
    if (model.vertices.empty()) return nullptr;

    // Create or fetch shared mesh
    MeshHandle mesh;
    auto mesh_it = pm_mesh_cache.find(m2_path);
    if (mesh_it != pm_mesh_cache.end()) {
        mesh = mesh_it->second;
    } else {
        mesh = std::make_shared<Mesh>(pm_device, model.vertices, model.indices);
        pm_mesh_cache[m2_path] = mesh;
    }

    // Load texture via the multi-strategy helper - tries M2-declared
    // paths first, then filename stem .blp, then any .blp in the dir.
    TextureHandle texture;
    std::string blp_path = FindM2BlpPath(m2_path, model);

    if (!blp_path.empty()) {
        auto tex_it = pm_texture_cache.find(blp_path);
        if (tex_it != pm_texture_cache.end()) {
            texture = tex_it->second;
        } else {
            try {
                auto blp = BlpLoader::LoadFile(blp_path);
                texture = std::make_shared<Image>(pm_device, blp);
                pm_texture_cache[blp_path] = texture;
            } catch (...) {
                texture = GetDefaultTexture();
            }
        }
    }

    if (!texture) {
        texture = GetDefaultTexture();
    }

    // Create entity. Use name_hint when provided (doodads name themselves
    // "Doodad#<unique_id>") so the editor's entity panel can identify
    // each instance; otherwise fall back to the model's internal name.
    std::string entity_name = !name_hint.empty()
        ? name_hint
        : (model.name.empty() ? "M2Model" : model.name);
    Entity* entity = scene.CreateEntity(entity_name);

    // TransformComponent - use the caller-supplied TRS directly. The
    // default overload above passes pos=0, rot=identity, scale=1 +
    // calls ApplyWowCoordTransform afterward; the doodad overload
    // passes the MDDF-derived TRS with no further mutation.
    auto* transform = entity->AddComponent<TransformComponent>();
    float ground_offset = -model.bbox_min.z;

    // Heuristic: zero position + identity rotation + unit scale means
    // the caller is the legacy single-arg overload, so apply the
    // WoW-coord hoist that R0 expected. Any non-default TRS came from
    // the explicit overload (doodads), which sets exact values.
    bool is_legacy = (position == glm::vec3{0.0f}) &&
                     (rotation == glm::quat{1.0f, 0.0f, 0.0f, 0.0f}) &&
                     (scale == glm::vec3{1.0f}) &&
                     name_hint.empty();
    if (is_legacy) {
        transform->ApplyWowCoordTransform(ground_offset);
    } else {
        transform->pm_position = position;
        transform->pm_rotation = rotation;
        transform->pm_scale    = scale;
    }

    // MeshComponent
    auto* mesh_comp = entity->AddComponent<MeshComponent>();
    mesh_comp->pm_mesh = mesh;

    // MaterialComponent + per-entity bone buffer + descriptor set.
    // R4.5 collapses the previous two-path (legacy vs doodad) into one:
    // doodads no longer pass through LoadM2IntoScene at all (they go
    // through FlushDoodadInstances), so every call here is the legacy
    // spell-editor preview path. Per-entity bone buffer is needed so
    // skeletal animation in the preview can write its own matrices
    // without trampling other entities.
    auto* mat = entity->AddComponent<MaterialComponent>();

    vk::DeviceSize bone_buffer_size = Skeleton::MAX_BONES * sizeof(glm::mat4);

    mat->pm_bone_buffer = std::make_unique<Buffer>(
        pm_device, bone_buffer_size,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    std::vector<glm::mat4> identity(Skeleton::MAX_BONES, glm::mat4{1.0f});
    mat->pm_bone_buffer->Write(identity.data(), bone_buffer_size);

    mat->pm_descriptor_set = pm_descriptor_pool->AllocateSetRaw(*pm_descriptor_layout);

    // Binding 3: 1-entry instance buffer with the identity matrix. The
    // shader reads gl_InstanceIndex=0 and gets identity, so the math
    // collapses to (mvp * identity * skin * pos) - same as pre-R4.5.
    // Cached on the AssetManager because every legacy entity wants the
    // exact same buffer.
    if (!pm_identity_instance_buffer) {
        vk::DeviceSize inst_size = sizeof(glm::mat4);
        pm_identity_instance_buffer = std::make_unique<Buffer>(
            pm_device, inst_size,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent);
        glm::mat4 id_mat{1.0f};
        pm_identity_instance_buffer->Write(&id_mat, inst_size);
    }

    vk::DescriptorBufferInfo ubo_info{*pm_scene_ubo->GetBuffer(), 0, sizeof(SceneUBO)};
    auto tex_info = texture->DescriptorInfo();
    vk::DescriptorBufferInfo bone_info{
        *mat->pm_bone_buffer->GetBuffer(), 0, bone_buffer_size};
    vk::DescriptorBufferInfo inst_info{
        *pm_identity_instance_buffer->GetBuffer(), 0, sizeof(glm::mat4)};

    DescriptorWriter{}
        .WriteBuffer(0, ubo_info)
        .WriteImage(1, tex_info)
        .WriteBuffer(2, bone_info, vk::DescriptorType::eStorageBuffer)
        .WriteBuffer(3, inst_info, vk::DescriptorType::eStorageBuffer)
        .Apply(pm_device.GetDevice(), mat->pm_descriptor_set);

    SubmeshMaterial sub_mat;
    sub_mat.pm_texture = texture;
    mat->pm_submesh_materials.push_back(sub_mat);

    // SkeletonComponent only on the legacy single-arg path. The R4.5
    // doodad instancing path doesn't go through here at all (see
    // FlushDoodadInstances), but the multi-arg LoadM2IntoScene is still
    // exposed for callers that want manual placement without skeletal
    // animation (e.g. one-off props the editor may add interactively).
    if (is_legacy && model.skeleton.BoneCount() > 0) {
        auto* skel = entity->AddComponent<SkeletonComponent>();
        skel->pm_skeleton = &model.skeleton;
        skel->pm_animator = std::make_unique<Animator>(model.skeleton);

        for (auto& clip : model.animations) {
            skel->pm_clips.push_back(clip.get());
        }

        if (!skel->pm_clips.empty()) {
            skel->pm_animator->Play(skel->pm_clips[0]);
        }
    }

    // M2InfoComponent
    auto* info = entity->AddComponent<M2InfoComponent>();
    info->pm_model_name = model.name;
    info->pm_vertex_count = static_cast<uint32_t>(model.vertices.size());
    info->pm_index_count = static_cast<uint32_t>(model.indices.size());
    info->pm_bone_count = model.skeleton.BoneCount();
    info->pm_texture_paths = model.texture_paths;
    info->pm_submeshes = model.submeshes;
    info->pm_bbox_min = model.bbox_min;
    info->pm_bbox_max = model.bbox_max;
    info->pm_ground_offset = ground_offset;

    return entity;
}

AdtTextureSet AssetManager::LoadAdtTextures(const AdtTile& tile,
                                            uint32_t target_size) {
    AdtTextureSet result{};
    result.tile_tex_to_slice.assign(tile.textures.size(), -1);
    if (tile.textures.empty()) return result;

    // Two-pass approach:
    //   Pass 1: parse every BLP, decide on a canonical format (the one
    //           used by the first valid BLP that meets our size and
    //           power-of-two requirements). Skip the rest.
    //   Pass 2: allocate an array of N slices and upload the survivors.
    //
    // WoW terrain BLPs are almost universally BC1 (DXT1) without alpha;
    // we accept BC3 as a fallback so a tile that mixes one BC3 BLP
    // doesn't lose every texture, but only the first-format slices
    // upload successfully. Mismatched BLPs end up at slice -1 ->
    // caller substitutes a checkerboard.
    struct Candidate {
        size_t tile_index;
        std::string path;
        BlpTexture blp;
        bool ok = false;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(tile.textures.size());

    vk::Format chosen_format = vk::Format::eUndefined;

    for (size_t i = 0; i < tile.textures.size(); i++) {
        Candidate c;
        c.tile_index = i;
        c.path = ResolveWowAsset(tile.textures[i]);
        if (!fs::exists(c.path)) {
            std::fprintf(stderr,
                "ADT texture missing: %s\n", c.path.c_str());
            candidates.push_back(std::move(c));
            continue;
        }
        try {
            c.blp = BlpLoader::LoadFile(c.path);
        } catch (const std::exception& e) {
            std::fprintf(stderr,
                "BLP parse failed (%s): %s\n", c.path.c_str(), e.what());
            candidates.push_back(std::move(c));
            continue;
        }

        // We need a BLP whose dimensions are a power-of-two multiple of
        // target_size and whose format is a supported block format.
        // Most WoW terrain BLPs are 256x256 BC1, so this check passes
        // trivially; large 1024x1024 ground BLPs hit the mip-skip path.
        // Tiny 8x8 placeholder BLPs (used for "no border" markers on
        // many tiles) fail silently here - the shader's magenta path
        // covers them and they don't represent a real asset.
        int skip = BlpMipForTarget(c.blp.width, target_size);
        if (skip < 0) {
            candidates.push_back(std::move(c));
            continue;
        }

        if (chosen_format == vk::Format::eUndefined) {
            chosen_format = c.blp.format;
        }
        c.ok = (c.blp.format == chosen_format);
        if (!c.ok) {
            std::fprintf(stderr,
                "BLP %s format mismatch (got %u, expected %u)\n",
                c.path.c_str(),
                static_cast<unsigned>(c.blp.format),
                static_cast<unsigned>(chosen_format));
        }
        candidates.push_back(std::move(c));
    }

    if (chosen_format == vk::Format::eUndefined) {
        // No usable BLPs at all. Leave result.diffuse null - caller
        // will see the empty array as "all slices missing" and fall
        // back to the per-fragment "magenta missing" path.
        return result;
    }

    // Count successful candidates so we know how many slices to allocate.
    uint32_t slice_count = 0;
    for (const auto& c : candidates) if (c.ok) slice_count++;
    if (slice_count == 0) return result;

    // Mip levels: log2(target_size) - 1 (stop at 4x4 for BC formats).
    uint32_t mip_levels = 1;
    {
        uint32_t s = target_size;
        while (s > 4 && mip_levels < 12) {
            s >>= 1;
            mip_levels++;
        }
    }

    result.diffuse = std::make_unique<TextureArray>(
        pm_device, target_size, target_size,
        slice_count, chosen_format, mip_levels);

    uint32_t next_slice = 0;
    for (const auto& c : candidates) {
        if (!c.ok) continue;
        if (result.diffuse->UploadSliceFromBlp(next_slice, c.blp)) {
            result.tile_tex_to_slice[c.tile_index] = static_cast<int>(next_slice);
            next_slice++;
        }
    }
    result.diffuse->FinalizeForSampling();
    return result;
}

Entity* AssetManager::LoadAdtTileIntoScene(
    int tile_x, int tile_y, Scene& scene,
    const vk::raii::DescriptorSetLayout& terrain_layout) {

    if (!pm_descriptor_pool || !pm_scene_ubo) return nullptr;
    if (tile_x < 0 || tile_x > 63 || tile_y < 0 || tile_y > 63) return nullptr;

    // Build the WoW-style path. Backslashes mirror what asset_extract
    // produces and what ResolveWowAsset normalizes.
    char rel_path[96];
    std::snprintf(rel_path, sizeof(rel_path),
        "World/Maps/Azeroth/Azeroth_%d_%d.adt", tile_x, tile_y);
    std::string fs_path = ResolveWowAsset(rel_path);

    if (!fs::exists(fs_path)) {
        std::fprintf(stderr, "ADT missing: %s\n", fs_path.c_str());
        return nullptr;
    }

    // AdtTile is ~5 MB (256 chunks * ~19 KB each); must be heap-allocated.
    auto tile = std::make_unique<AdtTile>();
    if (!AdtLoader::LoadFile(fs_path, *tile)) {
        std::fprintf(stderr, "ADT parse failed: %s\n", fs_path.c_str());
        return nullptr;
    }
    tile->tile_x = tile_x;
    tile->tile_y = tile_y;

    // Phase 2E stage 1: capture every MODF WMO placement BEFORE the
    // AdtTile drops at end of this function. Dedup by unique_id - the
    // same WMO (e.g. Stormwind) is listed in every ADT it overlaps, so
    // without dedup we'd spawn a dozen copies. Convert from MDDF-frame
    // WoW coords to engine space using the same axis flip the MDDF
    // doodad code uses.
    for (const auto& w : tile->wmos) {
        if (pm_wmo_seen_unique_ids.count(w.unique_id)) continue;
        pm_wmo_seen_unique_ids.insert(w.unique_id);

        // MDDF -> MCNK frame -> engine frame (same convention as doodads).
        float mcnk_x = kAdtMaxCoord - w.pos_x;
        float mcnk_y = kAdtMaxCoord - w.pos_z;
        float mcnk_z =                w.pos_y;
        glm::vec3 engine_pos(mcnk_y, mcnk_z, mcnk_x);

        // Bbox extents from MODF bbox_lo/hi. The bbox is in WoW MDDF
        // coords too, so the extents (hi - lo) are valid yard sizes
        // regardless of axis swap. Clamp tiny/huge values defensively.
        float ex = std::abs(w.bbox_hi[0] - w.bbox_lo[0]);
        float ey = std::abs(w.bbox_hi[1] - w.bbox_lo[1]);
        float ez = std::abs(w.bbox_hi[2] - w.bbox_lo[2]);
        if (ex < 1.0f) ex = 5.0f;
        if (ey < 1.0f) ey = 5.0f;
        if (ez < 1.0f) ez = 5.0f;
        // Swap Y and Z extents to match the WoW->engine axis remap
        // (WoW Z up -> engine Y up; WoW Y east -> engine X east).
        glm::vec3 extents{ex, ez, ey};

        std::string wmo_path;
        if (w.name_id < tile->wmo_paths.size()) {
            wmo_path = tile->wmo_paths[w.name_id];
        }

        WmoPlacement p{};
        p.wow_path     = wmo_path;
        p.engine_pos   = engine_pos;
        p.bbox_extents = extents;
        p.rot_deg      = glm::vec3{w.rot_x_deg, w.rot_y_deg, w.rot_z_deg};
        p.unique_id    = w.unique_id;
        pm_pending_wmo_placements.push_back(std::move(p));

        // Diagnostic: dump raw MODF + computed positions for the first
        // few placements per session. Goal: prove (or disprove) that
        // (a) engine_pos is correctly derived, (b) bbox center in
        // engine frame coincides with engine_pos for centered models,
        // and (c) figure out the local-origin offset for asymmetric
        // ones (like the footbridge).
        static int diag_count = 0;
        if (diag_count < 12) {
            ++diag_count;
            float bc_x_wow = 0.5f * (w.bbox_lo[0] + w.bbox_hi[0]);
            float bc_y_wow = 0.5f * (w.bbox_lo[1] + w.bbox_hi[1]);
            float bc_z_wow = 0.5f * (w.bbox_lo[2] + w.bbox_hi[2]);
            // Note: MODF bbox per wowdev is "the WMO bbox already in
            // server-space coords, NOT in MDDF on-disk coords". So we
            // do NOT apply the kAdtMaxCoord flip to the bbox - the
            // raw bbox values ARE the world-space AABB. Just apply
            // WowToEngine axis swap.
            glm::vec3 bbox_center_engine_servercoord{
                bc_y_wow, bc_z_wow, bc_x_wow};
            // Also compute "treating bbox like MDDF on-disk" for
            // comparison. Whichever interpretation makes bbox_center
            // match engine_pos for symmetric WMOs is the right one.
            glm::vec3 bbox_center_engine_mddfcoord{
                kAdtMaxCoord - bc_z_wow,
                bc_y_wow,
                kAdtMaxCoord - bc_x_wow};
            glm::vec3 delta_server = bbox_center_engine_servercoord - engine_pos;
            glm::vec3 delta_mddf   = bbox_center_engine_mddfcoord - engine_pos;
            std::fprintf(stderr,
                "WMO_DIAG [%s] u=%u\n"
                "  raw_pos = (%.2f, %.2f, %.2f)  rot_deg = (%.2f, %.2f, %.2f)\n"
                "  raw_bbox lo=(%.2f, %.2f, %.2f) hi=(%.2f, %.2f, %.2f)\n"
                "  engine_pos                = (%.2f, %.2f, %.2f)\n"
                "  bbox_center_engine_server = (%.2f, %.2f, %.2f)  delta=(%.2f, %.2f, %.2f) |%.2f|\n"
                "  bbox_center_engine_mddf   = (%.2f, %.2f, %.2f)  delta=(%.2f, %.2f, %.2f) |%.2f|\n",
                wmo_path.c_str(), w.unique_id,
                w.pos_x, w.pos_y, w.pos_z,
                w.rot_x_deg, w.rot_y_deg, w.rot_z_deg,
                w.bbox_lo[0], w.bbox_lo[1], w.bbox_lo[2],
                w.bbox_hi[0], w.bbox_hi[1], w.bbox_hi[2],
                engine_pos.x, engine_pos.y, engine_pos.z,
                bbox_center_engine_servercoord.x, bbox_center_engine_servercoord.y, bbox_center_engine_servercoord.z,
                delta_server.x, delta_server.y, delta_server.z, glm::length(delta_server),
                bbox_center_engine_mddfcoord.x, bbox_center_engine_mddfcoord.y, bbox_center_engine_mddfcoord.z,
                delta_mddf.x, delta_mddf.y, delta_mddf.z, glm::length(delta_mddf));
        }
    }

    // Cache the heightmap before the AdtTile gets dropped at end of
    // this function. PlayerController queries this via GetGroundY to
    // make the player follow terrain contour. Stored sparse - tiles
    // evicted by the streamer aren't currently removed from this
    // cache (memory leak in the strict sense, but ~80 KB/tile means
    // even a full Azeroth = 64*64*80 = 320 MB which is fine).
    {
        const uint32_t key = static_cast<uint32_t>(tile_x) * 64u +
                              static_cast<uint32_t>(tile_y);
        TileHeightCache cache{};
        for (int i = 0; i < kAdtChunksPerTile; ++i) {
            const auto& src = tile->chunks[i];
            auto& dst = cache.chunks[i];
            dst.wow_x = src.wow_x;
            dst.wow_y = src.wow_y;
            for (int j = 0; j < 81; ++j) {
                dst.y_outer[j] = src.heights.y_outer[j];
            }
        }
        pm_tile_height_cache[key] = cache;
    }

    // Build (or look up cached) mesh + alpha + chunk_meta.
    char mesh_key[32];
    std::snprintf(mesh_key, sizeof(mesh_key), "adt:%d_%d", tile_x, tile_y);

    // Diffuse atlas isn't cached across tiles for now (each tile gets its
    // own array). Adjacent tiles often share BLPs - a follow-up
    // optimization will share the underlying TextureArray slices.
    auto tex_set = LoadAdtTextures(*tile);
    auto terrain_build = TerrainMesh::Build(
        pm_device, *tile, tex_set.tile_tex_to_slice);

    // Per-chunk alpha array: 256 slices of 64x64 RGBA8.
    auto alpha_array = std::make_shared<TextureArray>(
        pm_device, 64, 64, kAdtChunksPerTile,
        vk::Format::eR8G8B8A8Unorm, 1);
    for (int i = 0; i < kAdtChunksPerTile; i++) {
        const uint8_t* slice = terrain_build.alpha_pixels.data() + i * 64 * 64 * 4;
        alpha_array->UploadSlicePixels(static_cast<uint32_t>(i), 0,
                                        slice, 64 * 64 * 4);
    }
    alpha_array->FinalizeForSampling();

    // Chunk-meta SSBO.
    vk::DeviceSize meta_size =
        sizeof(TerrainChunkMeta) * terrain_build.chunk_meta.size();
    auto chunk_meta_buf = std::make_unique<Buffer>(
        pm_device, meta_size,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);
    chunk_meta_buf->Write(terrain_build.chunk_meta.data(), meta_size);

    // Compute the tile's centroid in engine space + diagonal radius. Used
    // by the streamer to decide which tile a camera position belongs to
    // and to prioritize eviction.
    glm::dvec3 sum_wow{0.0};
    int counted = 0;
    glm::vec3 vmin{ std::numeric_limits<float>::max() };
    glm::vec3 vmax{ -std::numeric_limits<float>::max() };
    for (const auto& ch : tile->chunks) {
        if (ch.wow_x != 0.0f || ch.wow_y != 0.0f ||
            ch.heights.y_outer[0] != 0.0f) {
            sum_wow.x += ch.wow_x;
            sum_wow.y += ch.wow_y;
            sum_wow.z += ch.wow_z_base;
            counted++;
            glm::vec3 corner(ch.wow_y, ch.wow_z_base, ch.wow_x);
            vmin = glm::min(vmin, corner);
            vmax = glm::max(vmax, corner);
        }
    }
    glm::vec3 centroid{0.0f};
    if (counted > 0) {
        glm::dvec3 avg = sum_wow / double(counted);
        // WowToEngine: (wow_x, wow_y, wow_z) -> (wow_y, wow_z, wow_x)
        centroid = glm::vec3(avg.y, avg.z, avg.x);
    }
    float radius = glm::length(vmax - vmin) * 0.5f;

    // Spawn entity.
    char entity_name[48];
    std::snprintf(entity_name, sizeof(entity_name),
                  "Tile_%d_%d", tile_x, tile_y);
    Entity* entity = scene.CreateEntity(entity_name);
    entity->AddComponent<TransformComponent>();

    auto* mesh_comp = entity->AddComponent<MeshComponent>();
    auto shared_mesh = std::shared_ptr<Mesh>(std::move(terrain_build.mesh));
    pm_mesh_cache[mesh_key] = shared_mesh;
    mesh_comp->pm_mesh = shared_mesh;

    auto* terrain_comp = entity->AddComponent<TerrainComponent>();
    terrain_comp->pm_alpha_array = alpha_array;
    terrain_comp->pm_chunk_meta_ssbo = std::move(chunk_meta_buf);
    if (tex_set.diffuse) {
        terrain_comp->pm_diffuse_array =
            std::shared_ptr<TextureArray>(tex_set.diffuse.release());
    }

    terrain_comp->pm_descriptor_set =
        pm_descriptor_pool->AllocateSetRaw(terrain_layout);

    vk::DescriptorBufferInfo ubo_info{
        *pm_scene_ubo->GetBuffer(), 0, sizeof(SceneUBO)};
    vk::DescriptorBufferInfo meta_info{
        *terrain_comp->pm_chunk_meta_ssbo->GetBuffer(), 0, meta_size};
    auto alpha_info = terrain_comp->pm_alpha_array->DescriptorInfo();

    DescriptorWriter writer{};
    writer.WriteBuffer(0, ubo_info);
    writer.WriteBuffer(1, meta_info, vk::DescriptorType::eStorageBuffer);
    if (terrain_comp->pm_diffuse_array) {
        auto diffuse_info = terrain_comp->pm_diffuse_array->DescriptorInfo();
        writer.WriteImage(2, diffuse_info);
    } else {
        // No diffuse: bind the alpha array to keep the layout happy. The
        // shader's all-slots-unused branch then paints magenta so the
        // missing-asset state is obvious.
        writer.WriteImage(2, alpha_info);
    }
    writer.WriteImage(3, alpha_info);
    writer.Apply(pm_device.GetDevice(), terrain_comp->pm_descriptor_set);

    auto* tile_comp = entity->AddComponent<TerrainTileComponent>();
    tile_comp->pm_tile_x = tile_x;
    tile_comp->pm_tile_y = tile_y;
    tile_comp->pm_centroid_engine = centroid;
    tile_comp->pm_radius = radius;

    // Build per-instance water meshes. Each AdtLiquidInstance becomes
    // one Mesh; tiles with no liquids skip the component entirely.
    // Meshes are owned by the tile entity, so eviction drops them.
    if (!tile->liquids.empty()) {
        auto* water_comp = entity->AddComponent<WaterComponent>();
        water_comp->instances.reserve(tile->liquids.size());
        for (const auto& li : tile->liquids) {
            auto m = WaterMesh::Build(pm_device, *tile, li);
            if (!m) continue;
            WaterInstanceMesh wm{};
            wm.mesh = std::move(m);
            wm.liquid_type = li.liquid_type;
            water_comp->instances.push_back(std::move(wm));
        }
    }

    // Spawn MDDF doodads. Each entry references a path via name_id ->
    // tile->doodad_paths. Dedupe by unique_id across tiles so a prop
    // listed in two neighboring tiles doesn't render twice (typical
    // for forest edges and roads that cross tile boundaries).
    size_t spawn_attempts = 0;
    size_t spawn_failures = 0;
    size_t spawn_duplicates = 0;
    for (const auto& d : tile->doodads) {
        if (!pm_spawned_doodad_ids.insert(d.unique_id).second) {
            spawn_duplicates++;
            continue;
        }
        if (d.name_id >= tile->doodad_paths.size()) {
            spawn_failures++;
            continue;
        }
        const std::string& wow_path = tile->doodad_paths[d.name_id];
        if (wow_path.empty()) {
            spawn_failures++;
            continue;
        }

        std::string fs_path = ResolveWowAsset(wow_path);
        // Also resolve .mdx -> .m2 here so the key in
        // pm_pending_doodad_instances matches whatever path the M2
        // loader will actually use. Without this, two MDDF paths that
        // differ only by extension produce two map entries (and two
        // identical mesh loads).
        fs_path = ResolveM2Extension(fs_path);

        // MDDF stores positions in a different coordinate origin than
        // MCNK. MCNK puts the world origin at the map center (tile 32,
        // 32); MDDF puts it at the south-west corner of the 64x64 grid
        // (so values are 0..17066). Convert MDDF -> MCNK first, then
        // do the WoW->engine axis remap.
        //
        // The flip math: in MCNK coords, X is south-positive and Y is
        // east-positive but both axes grow toward NEGATIVE values when
        // you move toward higher tile indices (because tile 32 is at
        // origin and indices grow with WoW negation). MDDF values count
        // up from the SW corner the same direction, so the conversion
        // is just (kAdtMaxCoord - mddf_value) on each horizontal axis.
        float mcnk_x = kAdtMaxCoord - d.pos_x;   // south axis
        float mcnk_y = kAdtMaxCoord - d.pos_z;   // east axis
        float mcnk_z =                d.pos_y;   // up (unchanged)
        // WoW -> engine: (east, up, south).
        glm::vec3 engine_pos(mcnk_y, mcnk_z, mcnk_x);

        // MDDF on-disk frame is (X=south, Y=UP, Z=east); M2 vertices are
        // stored Z-up. The MDDF Euler is applied in WORLD frame after
        // the model has been axis-swapped from M2-local to engine basis.
        // The axis correspondences MDDF -> engine are:
        //   MDDF Y (up)    -> engine Y (up)    -> yaw axis  (0,1,0)
        //   MDDF X (south) -> engine Z (south) -> pitch axis (0,0,1)
        //   MDDF Z (east)  -> engine X (east)  -> roll axis  (1,0,0)
        // YXZ application order matches the MDDF spec.
        glm::quat q_yaw   = glm::angleAxis(glm::radians(d.rot_y_deg), glm::vec3{0, 1, 0});
        glm::quat q_pitch = glm::angleAxis(glm::radians(d.rot_x_deg), glm::vec3{0, 0, 1});
        glm::quat q_roll  = glm::angleAxis(glm::radians(d.rot_z_deg), glm::vec3{1, 0, 0});
        glm::quat q_world = q_yaw * q_pitch * q_roll;
        // M2 axis swap: -90 around engine X brings M2 +Z (up) to engine
        // +Y (up). Applied as the INNERMOST rotation so the placement
        // rotation operates on an already-engine-oriented model.
        glm::quat q_m2_axis = glm::angleAxis(glm::radians(-90.0f), glm::vec3{1, 0, 0});

        glm::vec3 scale_vec(d.scale);

        // M = T(engine_pos) * R(q_world) * R(q_m2_axis) * S(scale)
        glm::mat4 m = glm::translate(glm::mat4{1.0f}, engine_pos);
        m = m * glm::mat4_cast(q_world * q_m2_axis);
        m = glm::scale(m, scale_vec);

        PendingDoodadInstance pending{};
        pending.model_matrix = m;
        pm_pending_doodad_instances[fs_path].push_back(pending);
        spawn_attempts++;
    }
    if (spawn_attempts > 0 || spawn_duplicates > 0 || spawn_failures > 0) {
        std::fprintf(stderr,
            "Tile (%d, %d): %zu doodads, queued %zu, missing %zu, dedup %zu\n",
            tile_x, tile_y, tile->doodads.size(),
            spawn_attempts, spawn_failures, spawn_duplicates);
    }

    // R4-grass: walk the MCNK / MCLY effect_ids and scatter grass M2s
    // from the GroundEffect DBC tables. This is a separate pass from
    // MDDF because grass is procedurally placed at runtime - it never
    // appears in the ADT MDDF list. Skipped silently when the tables
    // weren't loaded (LoadGroundEffectTables() failed or wasn't
    // called).
    ScatterGrassForTile(*tile);

    return entity;
}

bool AssetManager::LoadGroundEffectTables() {
    // Asset layout: assets/dbc/GroundEffectTexture.dbc and
    // assets/dbc/GroundEffectDoodad.dbc. Both ship with the WoW
    // 3.3.5a client and were extracted at asset-prep time. Failure
    // here is non-fatal - the rest of the engine renders without
    // detail grass.
    std::string tex_path =
        std::string(MVE_ASSET_DIR) + "/dbc/GroundEffectTexture.dbc";
    std::string doo_path =
        std::string(MVE_ASSET_DIR) + "/dbc/GroundEffectDoodad.dbc";
    pm_ground_effects_loaded =
        pm_ground_effects.Load(tex_path, doo_path);
    return pm_ground_effects_loaded;
}

bool AssetManager::GetGroundY(const glm::vec3& engine_pos,
                                float* out_y) const {
    // Engine -> WoW frame:
    //   engine.x = wow_y (east-axis)
    //   engine.z = wow_x (south-axis)
    //   engine.y = wow_z (up)
    const float wow_x = engine_pos.z;
    const float wow_y = engine_pos.x;

    // Tile coords. Tile (32, 32) sits at WoW origin; both axes count
    // up TOWARD the southern/eastern edge with each tile spanning
    // 533.333 yards. The +0.5 offset puts wow=0 in the centre of
    // tile 32 not at its boundary.
    const float kTileSize = kAdtTileSize;
    int tile_x = 32 - static_cast<int>(std::floor(wow_x / kTileSize + 0.5f));
    int tile_y = 32 - static_cast<int>(std::floor(wow_y / kTileSize + 0.5f));
    // Try the analytic tile first, plus the 8 neighbours, since the
    // analytic tile math can land just outside the chunk grid at
    // tile borders.
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int tx = tile_x + dx;
            int ty = tile_y + dy;
            if (tx < 0 || tx > 63 || ty < 0 || ty > 63) continue;
            uint32_t key = static_cast<uint32_t>(tx) * 64u +
                            static_cast<uint32_t>(ty);
            auto it = pm_tile_height_cache.find(key);
            if (it == pm_tile_height_cache.end()) continue;

            const TileHeightCache& cache = it->second;
            for (int i = 0; i < kAdtChunksPerTile; ++i) {
                const auto& ch = cache.chunks[i];
                // Chunk footprint: wow_x in [ch.wow_x - kChunkSize,
                // ch.wow_x] (the chunk's NW corner has the LARGEST
                // wow_x; moving south decreases it). Same for wow_y
                // (east axis). A tiny epsilon avoids missing edges
                // where the position sits exactly on a chunk seam.
                const float kChunk = kAdtChunkSize;
                const float eps = 1e-3f;
                if (wow_x > ch.wow_x + eps) continue;
                if (wow_x < ch.wow_x - kChunk - eps) continue;
                if (wow_y > ch.wow_y + eps) continue;
                if (wow_y < ch.wow_y - kChunk - eps) continue;

                // Position within chunk, normalised to [0, 1]:
                //   col_norm = south-axis fraction (0 = north edge)
                //   row_norm = east-axis fraction  (0 = east edge)
                float col_norm = (ch.wow_x - wow_x) / kChunk;
                float row_norm = (ch.wow_y - wow_y) / kChunk;
                col_norm = std::clamp(col_norm, 0.0f, 1.0f);
                row_norm = std::clamp(row_norm, 0.0f, 1.0f);

                // Bilerp the 9x9 outer-vertex grid. Same convention
                // as grass_scatter.cpp's SampleHeight - the indexing
                // is y_outer[r * 9 + c] where r is east-axis index
                // and c is south-axis index.
                float fr = row_norm * 8.0f;
                float fc = col_norm * 8.0f;
                int r0 = static_cast<int>(std::floor(fr));
                int c0 = static_cast<int>(std::floor(fc));
                int r1 = std::min(r0 + 1, 8);
                int c1 = std::min(c0 + 1, 8);
                float tr = fr - r0;
                float tc = fc - c0;
                float h00 = ch.y_outer[r0 * 9 + c0];
                float h10 = ch.y_outer[r1 * 9 + c0];
                float h01 = ch.y_outer[r0 * 9 + c1];
                float h11 = ch.y_outer[r1 * 9 + c1];
                float h_c0 = h00 + (h10 - h00) * tr;
                float h_c1 = h01 + (h11 - h01) * tr;
                *out_y = h_c0 + (h_c1 - h_c0) * tc;
                return true;
            }
        }
    }
    return false;
}

bool AssetManager::LoadLightTables() {
    // Asset layout: assets/dbc/{Light,LightParams,LightIntBand,
    // LightFloatBand}.dbc. All four ship with the WoW 3.3.5a client.
    // Failure is non-fatal: the RenderSystem's LightCycle will
    // produce default snapshots if these aren't loaded, which means
    // the engine renders with the hardcoded "Elwynn noon" colors
    // baked into the LightSnapshot defaults.
    return pm_light_tables.Load(std::string(MVE_ASSET_DIR));
}

void AssetManager::EnqueueDetailGrassInstance(
    const std::string& wow_m2_path, const glm::mat4& model_matrix) {
    if (wow_m2_path.empty()) return;
    // Resolve identically to MDDF: ResolveWowAsset then ResolveM2Extension
    // so the cache key matches whatever the M2 loader will use. Without
    // this, two grass placements that differ only by .mdx vs .m2 in
    // their source DBC produce two separate entities.
    std::string fs_path = ResolveWowAsset(wow_m2_path);
    fs_path = ResolveM2Extension(fs_path);

    pm_detail_grass_paths.insert(fs_path);
    PendingDoodadInstance pending{};
    pending.model_matrix = model_matrix;
    pm_pending_doodad_instances[fs_path].push_back(pending);
}

void AssetManager::ScatterGrassForTile(const AdtTile& tile) {
    if (!pm_ground_effects_loaded) return;

    // Conservative per-sub-cell cap so a 5x5 preload doesn't blow the
    // descriptor pool. WoW's GroundEffectTexture.Density values reach
    // 12+ in dense forest tiles; combined with 256 chunks * 64 sub-
    // cells that's >150k placements per tile if we let it run free.
    // 1 per sub-cell per layer means at most 4 * 64 * 256 = ~65k per
    // tile, but in practice most sub-cells fall under the 10% alpha
    // threshold or have effect_id=0 so real counts are ~5-15k.
    constexpr int kMaxPerSubcell = 1;

    std::vector<GrassPlacement> placements;
    placements.reserve(8192);
    mve::ScatterGrassForTile(tile, pm_ground_effects, kMaxPerSubcell, placements);

    for (const auto& p : placements) {
        EnqueueDetailGrassInstance(p.wow_m2_path, p.model_matrix);
    }

    if (!placements.empty()) {
        std::fprintf(stderr,
            "ScatterGrassForTile (%d, %d): %zu grass placements queued\n",
            tile.tile_x, tile.tile_y, placements.size());
    }
}

void AssetManager::FlushDoodadInstances(Scene& scene) {
    if (!pm_descriptor_layout || !pm_descriptor_pool || !pm_scene_ubo) {
        pm_pending_doodad_instances.clear();
        return;
    }
    if (pm_pending_doodad_instances.empty()) return;

    size_t new_instances = 0;
    size_t new_entities  = 0;
    size_t extended_entities = 0;

    // Stable cmd-buffer fence: any rewrite of a descriptor set that's
    // currently bound in an in-flight command buffer is a validation
    // error and undefined behaviour. The streaming path flushes from
    // the per-frame Update(), so we may be looking at a set that the
    // previous frame just used. waitIdle here is the simplest correct
    // sync barrier - flushes are rare (one per tile-set load), so the
    // perf cost is negligible.
    pm_device.GetDevice().waitIdle();

    for (auto& [m2_path, placements] : pm_pending_doodad_instances) {
        if (placements.empty()) continue;

        // 1. Parse + cache the M2 model (skip path if file missing or
        //    corrupt - the placement set for it just gets dropped).
        auto cache_it = pm_m2_cache.find(m2_path);
        std::shared_ptr<M2CacheEntry> cache_entry;
        if (cache_it != pm_m2_cache.end()) {
            cache_entry = cache_it->second;
        } else {
            if (!fs::exists(m2_path)) continue;
            try {
                cache_entry = std::make_shared<M2CacheEntry>();
                cache_entry->model = M2Loader::LoadFile(m2_path);
            } catch (...) {
                continue;
            }
            pm_m2_cache[m2_path] = cache_entry;
        }
        auto& model = cache_entry->model;
        if (model.vertices.empty()) continue;

        // 2. Mesh (shared)
        MeshHandle mesh;
        auto mesh_it = pm_mesh_cache.find(m2_path);
        if (mesh_it != pm_mesh_cache.end()) {
            mesh = mesh_it->second;
        } else {
            mesh = std::make_shared<Mesh>(
                pm_device, model.vertices, model.indices);
            pm_mesh_cache[m2_path] = mesh;
        }

        // 3. Texture per submesh. WoW M2s store one or more textures
        //    in texture_paths[]; a "texture_lookup" array picks which
        //    one each batch uses. The old code grabbed [0] for every
        //    submesh, which is why all batches drew with the trunk
        //    texture even on the leaf submesh. Now we resolve once
        //    per submesh.
        auto load_blp = [&](const std::string& wow_path) -> TextureHandle {
            if (wow_path.empty()) return nullptr;
            std::string fs_path = ResolveWowAsset(wow_path);
            if (!fs::exists(fs_path)) return nullptr;
            auto cached = pm_texture_cache.find(fs_path);
            if (cached != pm_texture_cache.end()) return cached->second;
            try {
                auto blp = BlpLoader::LoadFile(fs_path);
                auto tex = std::make_shared<Image>(pm_device, blp);
                pm_texture_cache[fs_path] = tex;
                return tex;
            } catch (...) {
                return nullptr;
            }
        };

        // Helper: resolve the texture for a specific submesh's batch.
        // Falls back through every layer of the M2 texture indirection
        // before giving up and using the FindM2BlpPath heuristic.
        auto submesh_texture = [&](const M2Submesh& sub) -> TextureHandle {
            // texture_lookup[texture_combo_index] -> texture_paths index
            if (sub.texture_combo_index < model.texture_lookup.size()) {
                uint16_t ti = model.texture_lookup[sub.texture_combo_index];
                if (ti < model.texture_paths.size()) {
                    auto t = load_blp(model.texture_paths[ti]);
                    if (t) return t;
                }
            }
            // Fallback A: any texture_paths entry, in order.
            for (const auto& p : model.texture_paths) {
                auto t = load_blp(p);
                if (t) return t;
            }
            // Fallback B: stem-based / dir-scan heuristic.
            std::string fs_path = FindM2BlpPath(m2_path, model);
            if (!fs_path.empty()) {
                auto cached = pm_texture_cache.find(fs_path);
                if (cached != pm_texture_cache.end()) return cached->second;
                try {
                    auto blp = BlpLoader::LoadFile(fs_path);
                    auto tex = std::make_shared<Image>(pm_device, blp);
                    pm_texture_cache[fs_path] = tex;
                    return tex;
                } catch (...) {}
            }
            return GetDefaultTexture();
        };

        // 4. Shared material - per-submesh records cached the first
        //    time we see this M2 path. Bone buffer is identity, shared
        //    across all submeshes.
        auto sh_it = pm_shared_m2_material.find(m2_path);
        std::shared_ptr<M2SharedMaterial> shared;
        if (sh_it != pm_shared_m2_material.end()) {
            shared = sh_it->second;
        } else {
            shared = std::make_shared<M2SharedMaterial>();

            vk::DeviceSize bone_size =
                Skeleton::MAX_BONES * sizeof(glm::mat4);
            shared->pm_bone_buffer = std::make_unique<Buffer>(
                pm_device, bone_size,
                vk::BufferUsageFlagBits::eStorageBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent);
            std::vector<glm::mat4> bone_ids(
                Skeleton::MAX_BONES, glm::mat4{1.0f});
            shared->pm_bone_buffer->Write(bone_ids.data(), bone_size);

            // Build per-submesh material records. One descriptor set per
            // submesh, each with its own texture binding.
            shared->pm_submeshes.reserve(model.submeshes.size());
            for (const auto& sub : model.submeshes) {
                M2SharedSubmesh ssm{};
                ssm.pm_texture     = submesh_texture(sub);
                ssm.pm_index_start = sub.index_start;
                ssm.pm_index_count = sub.index_count;
                if (sub.material_index < model.materials.size()) {
                    const auto& mat = model.materials[sub.material_index];
                    ssm.pm_blend_mode   = mat.blend_mode;
                    ssm.pm_render_flags = mat.flags;
                }
                ssm.pm_descriptor_set =
                    pm_descriptor_pool->AllocateSetRaw(*pm_descriptor_layout);
                shared->pm_submeshes.push_back(std::move(ssm));
            }

            // Back-compat slot for legacy LoadM2IntoScene callers.
            // (DoodadInstanceComponent uses pm_submeshes instead.)
            shared->pm_texture = !shared->pm_submeshes.empty()
                ? shared->pm_submeshes[0].pm_texture
                : GetDefaultTexture();
            shared->pm_descriptor_set = !shared->pm_submeshes.empty()
                ? shared->pm_submeshes[0].pm_descriptor_set
                : VK_NULL_HANDLE;

            pm_shared_m2_material[m2_path] = shared;
        }

        // 5. Append the new placements to the persistent matrix list
        //    for this path. Order matters - existing entities already
        //    drew the first N matrices in this list; new placements go
        //    at the end.
        auto& all_matrices = pm_flushed_doodad_matrices[m2_path];
        size_t prev_count = all_matrices.size();
        all_matrices.reserve(prev_count + placements.size());
        for (const auto& p : placements) {
            all_matrices.push_back(p.model_matrix);
        }
        size_t total_count = all_matrices.size();
        vk::DeviceSize total_bytes = total_count * sizeof(glm::mat4);

        // 6. (Re-)allocate the instance SSBO. We always rebuild even
        //    if just appending - the buffer's size is fixed at create
        //    time, so growing requires a new allocation.
        auto inst_buf = std::make_unique<Buffer>(
            pm_device, total_bytes,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent);
        inst_buf->Write(all_matrices.data(), total_bytes);

        // 7. Write each submesh's descriptor set. Bindings 0/2/3 are the
        //    same shared resources (scene UBO, bone buffer, instance SSBO);
        //    binding 1 is the submesh-specific texture.
        vk::DescriptorBufferInfo ubo_info{
            *pm_scene_ubo->GetBuffer(), 0, sizeof(SceneUBO)};
        vk::DescriptorBufferInfo bone_info{
            *shared->pm_bone_buffer->GetBuffer(), 0,
            Skeleton::MAX_BONES * sizeof(glm::mat4)};
        vk::DescriptorBufferInfo inst_info{
            *inst_buf->GetBuffer(), 0, total_bytes};

        for (auto& ssm : shared->pm_submeshes) {
            auto tex_info = ssm.pm_texture->DescriptorInfo();
            DescriptorWriter{}
                .WriteBuffer(0, ubo_info)
                .WriteImage(1, tex_info)
                .WriteBuffer(2, bone_info, vk::DescriptorType::eStorageBuffer)
                .WriteBuffer(3, inst_info, vk::DescriptorType::eStorageBuffer)
                .Apply(pm_device.GetDevice(), ssm.pm_descriptor_set);
        }

        // 8. Materialize the entity. If one already exists for this
        //    M2 path (later streamed-in tile added more placements),
        //    update its DoodadInstanceComponent in place; otherwise
        //    create a fresh entity.
        Entity* entity = nullptr;
        auto en_it = pm_doodad_entity_for_path.find(m2_path);
        if (en_it != pm_doodad_entity_for_path.end()) {
            entity = scene.FindEntity(en_it->second);
        }

        if (!entity) {
            std::string entity_name = "DoodadGroup:";
            size_t slash = m2_path.find_last_of("/\\");
            entity_name += (slash != std::string::npos)
                ? m2_path.substr(slash + 1) : m2_path;

            entity = scene.CreateEntity(entity_name);
            entity->AddComponent<TransformComponent>();
            auto* mc = entity->AddComponent<MeshComponent>();
            mc->pm_mesh = mesh;
            entity->AddComponent<DoodadInstanceComponent>();

            // M2InfoComponent (surfaces metadata to the editor panel)
            auto* info = entity->AddComponent<M2InfoComponent>();
            info->pm_model_name    = model.name;
            info->pm_vertex_count  = static_cast<uint32_t>(model.vertices.size());
            info->pm_index_count   = static_cast<uint32_t>(model.indices.size());
            info->pm_bone_count    = model.skeleton.BoneCount();
            info->pm_texture_paths = model.texture_paths;
            info->pm_submeshes     = model.submeshes;
            info->pm_bbox_min      = model.bbox_min;
            info->pm_bbox_max      = model.bbox_max;
            info->pm_ground_offset = -model.bbox_min.z;

            pm_doodad_entity_for_path[m2_path] = entity->Id();
            new_entities++;
        } else {
            extended_entities++;
        }

        auto* dic = entity->GetComponent<DoodadInstanceComponent>();
        dic->pm_instance_buffer = std::move(inst_buf);
        dic->pm_instance_count  = static_cast<uint32_t>(total_count);
        // Tag the group as detail grass if any placement for this M2
        // path came from the GroundEffect scatter (the set membership
        // is sticky - once tagged, always tagged - since grass paths
        // never alias to MDDF paths in practice).
        dic->pm_is_detail_grass =
            (pm_detail_grass_paths.find(m2_path) !=
             pm_detail_grass_paths.end());
        dic->pm_submeshes.clear();
        dic->pm_submeshes.reserve(shared->pm_submeshes.size());
        for (const auto& ssm : shared->pm_submeshes) {
            DoodadSubmesh ds{};
            ds.pm_descriptor_set = ssm.pm_descriptor_set;
            ds.pm_index_start    = ssm.pm_index_start;
            ds.pm_index_count    = ssm.pm_index_count;
            ds.pm_blend_mode     = ssm.pm_blend_mode;
            ds.pm_render_flags   = ssm.pm_render_flags;
            dic->pm_submeshes.push_back(ds);
        }

        // Coarse bounding sphere for the whole group. Center = average
        // of all instance positions, radius = max distance from center
        // plus the per-instance model bbox extent (so the test covers
        // an oak tree's leaves, not just its root point).
        glm::vec3 center{0.0f};
        for (const auto& m : all_matrices) {
            center += glm::vec3(m[3]);  // translation column
        }
        center /= static_cast<float>(all_matrices.size());
        float radius_sq = 0.0f;
        for (const auto& m : all_matrices) {
            glm::vec3 p = glm::vec3(m[3]) - center;
            radius_sq = glm::max(radius_sq, glm::dot(p, p));
        }
        float model_extent = glm::length(model.bbox_max - model.bbox_min);
        dic->pm_bbox_center = center;
        dic->pm_bbox_radius = std::sqrt(radius_sq) + model_extent;

        new_instances += placements.size();
    }

    pm_pending_doodad_instances.clear();

    if (new_instances > 0) {
        std::fprintf(stderr,
            "FlushDoodadInstances: +%zu instances (new entities %zu, "
            "extended %zu)\n",
            new_instances, new_entities, extended_entities);
    }
}

Entity* AssetManager::LoadWmoPlacement(
    const WmoPlacement& p, Scene& scene,
    const vk::raii::DescriptorSetLayout& wmo_descriptor_layout) {
    if (p.wow_path.empty()) return nullptr;

    std::string root_path = ResolveWowAsset(p.wow_path);
    auto root = WmoLoader::LoadRoot(root_path);
    if (!root) return nullptr;

    // Group files are <stem>_NNN.wmo where NNN is 3-digit zero-padded.
    // Stem = root path minus the .wmo extension.
    std::string stem = root_path;
    size_t dot = stem.find_last_of('.');
    if (dot != std::string::npos) stem.resize(dot);

    std::vector<WmoGroupGpu> group_gpus;
    group_gpus.reserve(root->header.n_groups);
    std::vector<glm::vec4> bbox_mins;
    std::vector<glm::vec4> bbox_maxs;
    bbox_mins.reserve(root->header.n_groups);
    bbox_maxs.reserve(root->header.n_groups);

    size_t grp_load_fail = 0, grp_build_empty = 0;
    for (uint32_t gi = 0; gi < root->header.n_groups; ++gi) {
        char suffix[16];
        std::snprintf(suffix, sizeof(suffix), "_%03u.wmo", gi);
        std::string gpath = stem + suffix;
        auto grp = WmoLoader::LoadGroup(gpath);
        if (!grp) { ++grp_load_fail; continue; }

        auto gpu = WmoMesh::Build(pm_device, *grp);
        if (gpu.mesh) {
            group_gpus.push_back(std::move(gpu));
            const auto& gi_info = (gi < root->group_infos.size())
                                      ? root->group_infos[gi]
                                      : WmoMogi{};
            bbox_mins.emplace_back(gi_info.bbox_min[0],
                                    gi_info.bbox_min[1],
                                    gi_info.bbox_min[2], 0.0f);
            bbox_maxs.emplace_back(gi_info.bbox_max[0],
                                    gi_info.bbox_max[1],
                                    gi_info.bbox_max[2], 0.0f);
        } else {
            ++grp_build_empty;
        }
    }
    if (group_gpus.empty()) {
        std::fprintf(stderr,
            "  WMO %s: n_groups=%u, loaded=%zu, build_empty=%zu, "
            "load_fail=%zu -> bbox fallback\n",
            p.wow_path.c_str(),
            root->header.n_groups, group_gpus.size(),
            grp_build_empty, grp_load_fail);
        return nullptr;
    }

    // Build the WMO model matrix. Three components, applied right to
    // left (so vertex transforms as: B then R_world then T):
    //
    //   B = 3-cycle basis permutation taking WMO-local WoW frame
    //       (X=south, Y=east, Z=up) to engine (X=east, Y=up, Z=south).
    //       Per WowToEngine: column 0 = (0,0,1), column 1 = (1,0,0),
    //       column 2 = (0,1,0). Same swap used by terrain + water.
    //
    //   R_world = MODF Euler applied in engine-frame axes, using the
    //       same MDDF axis mapping as M2 doodads but WITHOUT the -90 X
    //       innermost rotation (WMO MOVT is Y-up post-basis-swap, not
    //       Z-up like M2 vertices).
    //         yaw   (rot_y) -> rotate about engine Y = (0,1,0)
    //         pitch (rot_x) -> rotate about engine Z = (0,0,1)
    //         roll  (rot_z) -> rotate about engine X = (1,0,0)
    //         order: YXZ
    //
    //   T = translate to engine_pos
    //
    glm::mat4 B{0.0f};
    B[0] = glm::vec4{0, 0, 1, 0};
    B[1] = glm::vec4{1, 0, 0, 0};
    B[2] = glm::vec4{0, 1, 0, 0};
    B[3] = glm::vec4{0, 0, 0, 1};

    glm::quat q_yaw   = glm::angleAxis(glm::radians(p.rot_deg.y), glm::vec3{0, 1, 0});
    glm::quat q_pitch = glm::angleAxis(glm::radians(p.rot_deg.x), glm::vec3{0, 0, 1});
    glm::quat q_roll  = glm::angleAxis(glm::radians(p.rot_deg.z), glm::vec3{1, 0, 0});
    glm::mat4 R = glm::mat4_cast(q_yaw * q_pitch * q_roll);

    glm::mat4 T = glm::translate(glm::mat4{1.0f}, p.engine_pos);
    glm::mat4 model = T * R * B;

    char entity_name[96];
    std::snprintf(entity_name, sizeof(entity_name), "WMO_%u", p.unique_id);
    Entity* entity = scene.CreateEntity(entity_name);
    auto* tx = entity->AddComponent<TransformComponent>();
    tx->pm_position = p.engine_pos;
    auto* comp = entity->AddComponent<WmoInstanceComponent>();
    auto root_shared = std::shared_ptr<WmoRoot>(root.release());
    comp->groups         = std::move(group_gpus);
    comp->group_bbox_min = std::move(bbox_mins);
    comp->group_bbox_max = std::move(bbox_maxs);
    comp->model_matrix   = model;

    // Allocate one descriptor set per material with SceneUBO + diffuse
    // sampler. Texture loading mirrors the M2 load_blp pattern -
    // resolve through ResolveWowAsset, load via BlpLoader, cache in
    // pm_texture_cache, wrap in an Image. Missing textures land a
    // null vk::DescriptorSet so the draw loop falls back gracefully.
    if (pm_descriptor_pool && pm_scene_ubo) {
        const auto& wmo_layout = wmo_descriptor_layout;
        comp->material_sets.resize(root_shared->materials.size(),
                                    VK_NULL_HANDLE);
        comp->material_textures.resize(root_shared->materials.size());

        size_t tex_ok = 0, tex_missing_path = 0, tex_missing_file = 0,
               tex_load_fail = 0;
        for (size_t mi = 0; mi < root_shared->materials.size(); ++mi) {
            const std::string& wow_tex = root_shared->texture_paths[mi];
            if (wow_tex.empty()) { ++tex_missing_path; continue; }
            std::string fs_path = ResolveWowAsset(wow_tex);
            if (!fs::exists(fs_path)) {
                ++tex_missing_file;
                // Track unique top-level dirs across the whole session
                // by emitting one line per WMO listing all distinct
                // texture roots that are missing (caller can sort -u).
                std::fprintf(stderr,
                    "  WMO tex missing: %s\n", wow_tex.c_str());
                continue;
            }

            TextureHandle tex;
            auto cached = pm_texture_cache.find(fs_path);
            if (cached != pm_texture_cache.end()) {
                tex = cached->second;
            } else {
                try {
                    auto blp = BlpLoader::LoadFile(fs_path);
                    tex = std::make_shared<Image>(pm_device, blp);
                    pm_texture_cache[fs_path] = tex;
                } catch (...) {
                    ++tex_load_fail;
                    if (tex_load_fail <= 3) {
                        std::fprintf(stderr,
                            "  WMO BLP load threw: %s\n", fs_path.c_str());
                    }
                    continue;
                }
            }
            if (!tex) { ++tex_load_fail; continue; }
            ++tex_ok;

            vk::DescriptorSet ds =
                pm_descriptor_pool->AllocateSetRaw(wmo_layout);
            if (!ds) continue;

            vk::DescriptorBufferInfo ubo_info{
                *pm_scene_ubo->GetBuffer(), 0, sizeof(SceneUBO)};
            vk::DescriptorImageInfo img_info{
                *tex->GetSampler(),
                *tex->GetImageView(),
                vk::ImageLayout::eShaderReadOnlyOptimal};
            DescriptorWriter{}
                .WriteBuffer(0, ubo_info, vk::DescriptorType::eUniformBuffer)
                .WriteImage(1, img_info, vk::DescriptorType::eCombinedImageSampler)
                .Apply(pm_device.GetDevice(), ds);

            comp->material_sets[mi]     = ds;
            comp->material_textures[mi] = tex;
        }
        std::fprintf(stderr,
            "  WMO %s: %zu materials, %zu textured, %zu missing-path, "
            "%zu missing-file, %zu load-fail\n",
            p.wow_path.c_str(), root_shared->materials.size(),
            tex_ok, tex_missing_path, tex_missing_file, tex_load_fail);
    }

    comp->root = std::move(root_shared);

    return entity;
}

} // namespace mve
