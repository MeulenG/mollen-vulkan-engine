#include "core/window.h"
#include "core/device.h"
#include "core/renderer.h"
#include "core/offscreen_pass.h"
#include "core/imgui_context.h"
#include "scene/scene.h"
#include "scene/components/camera_component.h"
#include "scene/components/m2_info_component.h"
#include "scene/components/transform_component.h"
#include "scene/components/mesh_component.h"
#include "scene/components/material_component.h"
#include "scene/components/terrain_component.h"
#include "scene/terrain_mesh.h"
#include "resources/asset_manager.h"
#include "resources/buffer.h"
#include "resources/descriptor.h"
#include "resources/image.h"
#include "animation/skeleton.h"
#include "formats/wdt_loader.h"
#include "formats/adt_types.h"
#include "systems/render_system.h"
#include "systems/animation_system.h"
#include "systems/editor_ui_system.h"

#include <imgui.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>

int main() {
    try {
        mve::Window window{1280, 720, "Mollen Wow Tools"};
        mve::Device device{window};
        mve::Renderer renderer{window, device};
        mve::ImGuiContext imgui_ctx{window, device, renderer.GetSwapchainImageFormat()};

        auto offscreen = std::make_unique<mve::OffscreenPass>(
            device, 800, 600,
            renderer.GetSwapchainImageFormat(),
            renderer.GetDepthFormat());

        // Systems
        mve::Scene scene;
        mve::RenderSystem render_system{device, *offscreen};
        render_system.Init();

        mve::AssetManager assets{device};
        assets.SetDescriptorResources(
            render_system.DescriptorLayout(),
            render_system.GetDescriptorPool(),
            render_system.SceneUBOBuffer());

        mve::AnimationSystem animation_system;
        mve::EditorUISystem editor_ui{window, imgui_ctx, *offscreen};

        // Editor camera
        auto* cam_entity = scene.CreateEntity("EditorCamera");
        auto* cam = cam_entity->AddComponent<mve::CameraComponent>();
        cam->pm_is_active = true;
        cam->pm_camera.SetOrbit(8.0f, 0.5f, 0.3f);
        cam->pm_camera.SetTarget({0.0f, 0.5f, 0.0f});

        // Load an Elwynn Forest ADT tile and render it as terrain. The tile
        // chosen (32_48) covers the Northshire / Stormwind northern area.
        // This is the R1 milestone deliverable: a single ADT heightmap on
        // screen with the existing camera + lighting pipeline.
        //
        // Path resolution uses MVE_ASSET_DIR (compile-time absolute path to
        // the repo's assets/ dir) so the binary works regardless of CWD.
        // Without this, launching from VS debugger or build/app/Debug failed
        // silently because the relative "assets/..." path didn't resolve.
        {
            std::string adt_path = std::string(MVE_ASSET_DIR)
                + "/World/Maps/Azeroth/Azeroth_32_48.adt";
            mve::AdtTile tile{};
            if (!mve::AdtLoader::LoadFile(adt_path, tile)) {
                std::cerr << "Failed to load ADT: " << adt_path << "\n";
                return EXIT_FAILURE;
            } else {
                std::cout << "Loaded ADT with " << tile.textures.size()
                          << " textures and 256 chunks\n";

                // Step 1: load the tile's diffuse BLPs into one 2D-array
                // image. tile_tex_to_slice[i] tells the mesh builder
                // which slice the i-th MTEX entry landed on, so it can
                // bake per-chunk layer-slot indices into the chunk_meta
                // SSBO. A -1 entry means the BLP failed to load, and
                // the shader's slot=0xFFFFFFFF fallback substitutes.
                auto tex_set = assets.LoadAdtTextures(tile);
                if (!tex_set.diffuse) {
                    std::cerr << "Failed to load any ADT diffuse textures - "
                                 "terrain will render magenta\n";
                }

                // Step 2: build the terrain mesh. The result owns the
                // GPU Mesh, the per-chunk SSBO data, and the per-chunk
                // alpha pixel data we still need to upload to a 2D-array.
                auto terrain_build = mve::TerrainMesh::Build(
                    device, tile, tex_set.tile_tex_to_slice);

                // Step 3: upload the 256 alpha-map slices to a 64x64 RGBA8
                // 2D-array. One mip; no need for chained mips when the
                // texture is sampled at constant resolution per chunk_uv.
                auto alpha_array = std::make_shared<mve::TextureArray>(
                    device, 64, 64, mve::kAdtChunksPerTile,
                    vk::Format::eR8G8B8A8Unorm, 1);
                for (int i = 0; i < mve::kAdtChunksPerTile; i++) {
                    const uint8_t* slice = terrain_build.alpha_pixels.data()
                                          + i * 64 * 64 * 4;
                    alpha_array->UploadSlicePixels(
                        static_cast<uint32_t>(i), 0, slice, 64 * 64 * 4);
                }
                alpha_array->FinalizeForSampling();

                // Step 4: per-chunk metadata SSBO (256 * 16 bytes).
                vk::DeviceSize meta_size =
                    sizeof(mve::TerrainChunkMeta) * terrain_build.chunk_meta.size();
                auto chunk_meta_buf = std::make_unique<mve::Buffer>(
                    device, meta_size,
                    vk::BufferUsageFlagBits::eStorageBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible |
                        vk::MemoryPropertyFlagBits::eHostCoherent);
                chunk_meta_buf->Write(terrain_build.chunk_meta.data(), meta_size);

                // Step 5: spawn entity. Transform is identity because
                // the terrain mesh is already baked in world coords.
                auto* terrain_entity = scene.CreateEntity("Elwynn_32_48");
                terrain_entity->AddComponent<mve::TransformComponent>();
                auto* mesh_comp = terrain_entity->AddComponent<mve::MeshComponent>();
                mesh_comp->pm_mesh = std::move(terrain_build.mesh);

                auto* terrain_comp =
                    terrain_entity->AddComponent<mve::TerrainComponent>();
                terrain_comp->pm_alpha_array = alpha_array;
                terrain_comp->pm_chunk_meta_ssbo = std::move(chunk_meta_buf);
                if (tex_set.diffuse) {
                    terrain_comp->pm_diffuse_array =
                        std::shared_ptr<mve::TextureArray>(tex_set.diffuse.release());
                }

                // Step 6: descriptor set with the four bindings the
                // terrain pipeline consumes.
                terrain_comp->pm_descriptor_set =
                    render_system.GetDescriptorPool().AllocateSet(
                        render_system.TerrainDescriptorLayout());

                vk::DescriptorBufferInfo ubo_info{
                    *render_system.SceneUBOBuffer().GetBuffer(), 0, sizeof(mve::SceneUBO)};
                vk::DescriptorBufferInfo meta_info{
                    *terrain_comp->pm_chunk_meta_ssbo->GetBuffer(), 0, meta_size};
                auto alpha_info = terrain_comp->pm_alpha_array->DescriptorInfo();

                mve::DescriptorWriter writer{};
                writer.WriteBuffer(0, ubo_info);
                writer.WriteBuffer(1, meta_info, vk::DescriptorType::eStorageBuffer);
                if (terrain_comp->pm_diffuse_array) {
                    auto diffuse_info = terrain_comp->pm_diffuse_array->DescriptorInfo();
                    writer.WriteImage(2, diffuse_info);
                } else {
                    // No diffuse: bind the alpha array slice 0 just to satisfy
                    // the layout. The shader's all-slots-unused branch will
                    // paint magenta everywhere.
                    writer.WriteImage(2, alpha_info);
                }
                writer.WriteImage(3, alpha_info);
                writer.Apply(device.GetDevice(), terrain_comp->pm_descriptor_set);

                // Center the camera roughly on the tile. The tile spans
                // ~533 yards. The terrain's vertex world-positions come
                // straight from the ADT (WoW coords), so the tile sits
                // somewhere in world space dictated by its (x, y) index.
                //
                // We average the WoW positions across all parsed chunks
                // and convert to engine coords once. Using chunks[0] and
                // chunks[255] alone is fragile: if either corner failed
                // to parse, that entry is zero-initialized and the
                // midpoint lands halfway to world origin (far from the
                // tile, which sits at ~+/- 9000 yards in WoW absolute coords).
                glm::dvec3 sum_wow{0.0};
                int counted = 0;
                for (const auto& ch : tile.chunks) {
                    // A parsed chunk has at least one non-zero outer height
                    // or non-zero base position. Pure-zero entries are
                    // either failed parses or default-constructed.
                    if (ch.wow_x != 0.0f || ch.wow_y != 0.0f ||
                        ch.heights.y_outer[0] != 0.0f) {
                        sum_wow.x += ch.wow_x;
                        sum_wow.y += ch.wow_y;
                        sum_wow.z += ch.wow_z_base;
                        counted++;
                    }
                }
                glm::vec3 center{0.0f, 0.5f, 0.0f};
                if (counted > 0) {
                    glm::dvec3 avg = sum_wow / double(counted);
                    // WowToEngine: (wow_x, wow_y, wow_z) -> (wow_y, wow_z, wow_x)
                    center = glm::vec3(avg.y, avg.z, avg.x);
                }
                cam->pm_camera.SetTarget(center);
                cam->pm_camera.SetOrbit(800.0f, 0.5f, 0.6f);
            }
        }

        auto last_time = std::chrono::high_resolution_clock::now();

        while (!window.ShouldClose()) {
            glfwPollEvents();
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last_time).count();
            last_time = now;

            // ImGui frame
            imgui_ctx.NewFrame();
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

            // Systems
            editor_ui.Update(scene, render_system, dt);
            animation_system.Update(scene, dt);
            render_system.UpdateSceneUBO();
            scene.FlushDestroyed();

            // Render
            vk::raii::CommandBuffer* cmd;
            if (!renderer.BeginFrame(&cmd)) continue;

            // Find active camera
            mve::Camera* active_cam = nullptr;
            scene.Each<mve::CameraComponent>([&](mve::Entity&, mve::CameraComponent& cc) {
                if (cc.pm_is_active) active_cam = &cc.pm_camera;
            });

            if (active_cam) {
                render_system.Render(scene, *active_cam, *cmd);
            }

            // ImGui to swapchain
            renderer.BeginRendering(*cmd, false);
            imgui_ctx.Render(*cmd);
            renderer.EndRendering(*cmd);
            renderer.EndFrame(*cmd);
        }

        device.GetDevice().waitIdle();

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
