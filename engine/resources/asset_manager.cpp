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
#include "../scene/terrain_mesh.h"
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

        // WoW MDDF rotation is Euler degrees applied YXZ (yaw, pitch,
        // roll about the WoW axes). Build the quaternion in WoW frame
        // then compose with the -90 X-axis rotation that converts WoW
        // Z-up to engine Y-up.
        glm::quat q_wow =
              glm::angleAxis(glm::radians(d.rot_y_deg), glm::vec3{0, 1, 0})
            * glm::angleAxis(glm::radians(d.rot_x_deg), glm::vec3{1, 0, 0})
            * glm::angleAxis(glm::radians(d.rot_z_deg), glm::vec3{0, 0, 1});
        glm::quat q_axis = glm::angleAxis(glm::radians(-90.0f), glm::vec3{1, 0, 0});
        glm::quat q_final = q_axis * q_wow;

        glm::vec3 scale_vec(d.scale);

        // Build the per-instance model matrix and park it in the
        // pending list. The shader will multiply this against the
        // skin-bind-pose position to land each vertex in world space.
        //   M = T(engine_pos) * R(q_final) * S(scale)
        glm::mat4 m = glm::translate(glm::mat4{1.0f}, engine_pos);
        m = m * glm::mat4_cast(q_final);
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

    return entity;
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

} // namespace mve
