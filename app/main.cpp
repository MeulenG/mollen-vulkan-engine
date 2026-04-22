#include "core/window.h"
#include "core/device.h"
#include "core/renderer.h"
#include "core/offscreen_pass.h"
#include "core/imgui_context.h"
#include "resources/pipeline.h"
#include "resources/buffer.h"
#include "resources/descriptor.h"
#include "resources/image.h"
#include "scene/mesh.h"
#include "scene/camera.h"
#include "animation/skeleton.h"
#include "animation/animation_clip.h"
#include "animation/animator.h"
#include "formats/blp_loader.h"
#include "formats/m2_loader.h"

#include <imgui.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

namespace fs = std::filesystem;

struct PushConstants {
    glm::mat4 mvp;
    glm::mat4 model;
};

struct SceneUBO {
    alignas(16) glm::vec3 light_dir;
    float ambient;
    alignas(16) glm::vec3 light_color;
    float light_intensity;
};

int main() {
    try {
        mve::Window window{1280, 720, "Mollen Wow Tools"};
        mve::Device device{window};
        mve::Renderer renderer{window, device};
        mve::ImGuiContext imgui_ctx{window, device, renderer.getSwapchainImageFormat()};

        std::string asset_dir = "assets";
        std::string m2_path = asset_dir + "/Creature/bear/Bear.M2";

        mve::M2Model bear_model;
        bool m2_loaded = false;

        if (fs::exists(m2_path)) {
            try {
                bear_model = mve::M2Loader::loadFile(m2_path);
                m2_loaded = true;
                printf("Loaded M2: %s (%zu vertices, %zu indices, %u bones, %zu submeshes, %zu anims)\n",
                    bear_model.name.c_str(),
                    bear_model.vertices.size(),
                    bear_model.indices.size(),
                    bear_model.skeleton.boneCount(),
                    bear_model.submeshes.size(),
                    bear_model.animations.size());
            } catch (const std::exception& e) {
                printf("Failed to load M2: %s\n", e.what());
            }
        } else {
            printf("Bear M2 not found at %s - run asset-extract first.\n", m2_path.c_str());
        }

        // Create mesh from M2 data (or fallback cube)
        std::unique_ptr<mve::Mesh> mesh;
        if (m2_loaded && !bear_model.vertices.empty()) {
            mesh = std::make_unique<mve::Mesh>(device, bear_model.vertices, bear_model.indices);
        } else {
            std::cout << "Rendering Pyramid" << std::endl;
            auto pyramid = mve::Mesh::CreatePyramid(device);
            mesh = std::make_unique<mve::Mesh>(std::move(pyramid));
        }

        std::unique_ptr<mve::Image> texture;
        std::string blp_path = asset_dir + "/Creature/bear/BearSkinBrown.blp";

        if (fs::exists(blp_path)) {
            try {
                auto blp = mve::BlpLoader::loadFile(blp_path);
                texture = std::make_unique<mve::Image>(device, blp);
                printf("Loaded BLP: %ux%u, %u mips, format=%d\n",
                    blp.width, blp.height, blp.mip_count, (int)blp.format);
            } catch (const std::exception& e) {
                printf("Failed to load BLP: %s - using checkerboard\n", e.what());
                texture = std::make_unique<mve::Image>(mve::Image::createCheckerboard(device));
            }
        } else {
            printf("BLP not found at %s - using checkerboard\n", blp_path.c_str());
            texture = std::make_unique<mve::Image>(mve::Image::createCheckerboard(device));
        }

        std::unique_ptr<mve::Animator> animator;
        int current_anim = 0;

        if (m2_loaded && bear_model.skeleton.boneCount() > 0) {
            animator = std::make_unique<mve::Animator>(bear_model.skeleton);
            if (!bear_model.animations.empty()) {
                animator->play(bear_model.animations[0].get());
                printf("Playing animation: %s (%.2fs)\n",
                    bear_model.animations[0]->name().c_str(),
                    bear_model.animations[0]->duration());
            }
        }

        auto offscreen = std::make_unique<mve::OffscreenPass>(
            device, 800, 600,
            renderer.getSwapchainImageFormat(),
            renderer.getDepthFormat());

        ImTextureID viewport_tex = imgui_ctx.registerTexture(
            *offscreen->sampler(), *offscreen->colorImageView());

        auto descriptor_layout = mve::DescriptorSetLayoutBuilder{device}
            .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
            .addBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addBinding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
            .build();

        mve::DescriptorPool descriptor_pool{device, 10, {
            {vk::DescriptorType::eUniformBuffer, 10},
            {vk::DescriptorType::eCombinedImageSampler, 10},
            {vk::DescriptorType::eStorageBuffer, 10},
        }};

        mve::Buffer ubo_buffer{device, sizeof(SceneUBO),
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

        vk::DeviceSize bone_buffer_size = mve::Skeleton::MAX_BONES * sizeof(glm::mat4);
        mve::Buffer bone_buffer{device, bone_buffer_size,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

        // Init bone buffer
        {
            std::vector<glm::mat4> identity(mve::Skeleton::MAX_BONES, glm::mat4{1.0f});
            bone_buffer.write(identity.data(), bone_buffer_size);
        }

        auto descriptor_set = descriptor_pool.allocateSet(descriptor_layout);
        vk::DescriptorBufferInfo ubo_info{*ubo_buffer.buffer(), 0, sizeof(SceneUBO)};
        auto tex_info = texture->descriptorInfo();
        vk::DescriptorBufferInfo bone_info{*bone_buffer.buffer(), 0, bone_buffer_size};

        mve::DescriptorWriter{}
            .writeBuffer(0, ubo_info)
            .writeImage(1, tex_info)
            .writeBuffer(2, bone_info, vk::DescriptorType::eStorageBuffer)
            .apply(device.device(), descriptor_set);

        vk::PushConstantRange push_range{vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants)};
        vk::DescriptorSetLayout layouts[] = {*descriptor_layout};
        vk::PipelineLayoutCreateInfo layout_info{};
        layout_info.setPushConstantRanges(push_range);
        layout_info.setSetLayouts(layouts);
        auto pipeline_layout = device.device().createPipelineLayout(layout_info);

        auto pipeline_config = mve::PipelineConfig::defaultConfig();
        pipeline_config.pipeline_layout = *pipeline_layout;
        pipeline_config.color_attachment_format = offscreen->colorFormat();
        pipeline_config.depth_attachment_format = offscreen->depthFormat();
        pipeline_config.binding_descriptions = mve::Vertex::getBindingDescriptions();
        pipeline_config.attribute_descriptions = mve::Vertex::getAttributeDescriptions();

        std::string shader_dir = MVE_SHADER_DIR;
        auto pipeline = std::make_unique<mve::Pipeline>(
            device, shader_dir + "/basic.vert.spv", shader_dir + "/basic.frag.spv",
            pipeline_config);

        auto bg_pipeline_layout = device.device().createPipelineLayout({});

        auto bg_config = mve::PipelineConfig::defaultConfig();
        bg_config.pipeline_layout = *bg_pipeline_layout;
        bg_config.color_attachment_format = offscreen->colorFormat();
        bg_config.depth_attachment_format = offscreen->depthFormat();
        bg_config.depth_stencil_info.depthTestEnable = vk::False;
        bg_config.depth_stencil_info.depthWriteEnable = vk::False;
        bg_config.rasterization_info.cullMode = vk::CullModeFlagBits::eNone;
        bg_config.binding_descriptions.clear();
        bg_config.attribute_descriptions.clear();

        auto bg_pipeline = std::make_unique<mve::Pipeline>(
            device, shader_dir + "/background.vert.spv", shader_dir + "/background.frag.spv",
            bg_config);

        vk::PushConstantRange ground_push{vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants)};
        vk::PipelineLayoutCreateInfo ground_layout_info{};
        ground_layout_info.setPushConstantRanges(ground_push);
        auto ground_pipeline_layout = device.device().createPipelineLayout(ground_layout_info);

        auto ground_config = mve::PipelineConfig::defaultConfig();
        ground_config.pipeline_layout = *ground_pipeline_layout;
        ground_config.color_attachment_format = offscreen->colorFormat();
        ground_config.depth_attachment_format = offscreen->depthFormat();
        ground_config.binding_descriptions = mve::Vertex::getBindingDescriptions();
        ground_config.attribute_descriptions = mve::Vertex::getAttributeDescriptions();
        ground_config.rasterization_info.cullMode = vk::CullModeFlagBits::eNone;
        // Enable alpha blending for fade-out
        ground_config.color_blend_attachment.blendEnable = vk::True;
        ground_config.color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        ground_config.color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        ground_config.color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
        ground_config.color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        ground_config.color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        ground_config.color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;

        auto ground_pipeline = std::make_unique<mve::Pipeline>(
            device, shader_dir + "/ground.vert.spv", shader_dir + "/ground.frag.spv",
            ground_config);

        // auto ground_mesh = mve::Mesh::createGroundPlane(device, 30.0f);

        // Position camera to see model centered on ground
        mve::Camera camera;
        float model_ground_offset = 0.0f; // Y offset to place model feet on ground
        if (m2_loaded) {
            // In WoW coords (before our -90° X rotation):
            // Z is up, so bbox_min.z is the bottom of the model
            // After rotation, Y is up, so we offset by -bbox_min.z to place feet at Y=0
            model_ground_offset = -bear_model.bbox_min.z;

            glm::vec3 center = (bear_model.bbox_min + bear_model.bbox_max) * 0.5f;
            float height = bear_model.bbox_max.z - bear_model.bbox_min.z;
            float extent = glm::length(bear_model.bbox_max - bear_model.bbox_min);
            // Camera targets the center of the model (accounting for ground offset)
            camera.setTarget({0.0f, height * 0.4f, 0.0f});
            camera.setOrbit(extent * 1.8f, 0.5f, 0.2f);
        } else {
            camera.setOrbit(8.0f, 0.25f, 0.3f);
            camera.setTarget({0.0f, 0.15f, 0.0f});
        }

        float light_dir[3] = {0.5f, 1.0f, 0.3f};
        float ambient = 0.15f;
        float light_intensity = 0.85f;
        bool animate = true;
        float anim_speed = 1.0f;

        double last_x = 0.0, last_y = 0.0;
        bool first_mouse = true;
        auto last_time = std::chrono::high_resolution_clock::now();

        while (!window.shouldClose()) {
            glfwPollEvents();

            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last_time).count();
            last_time = now;

            imgui_ctx.newFrame();
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

            ImGui::Begin("Viewport");
            ImVec2 viewport_size = ImGui::GetContentRegionAvail();

            if (viewport_size.x > 0 && viewport_size.y > 0) {
                uint32_t vp_w = static_cast<uint32_t>(viewport_size.x);
                uint32_t vp_h = static_cast<uint32_t>(viewport_size.y);

                if (vp_w != offscreen->width() || vp_h != offscreen->height()) {
                    offscreen->resize(vp_w, vp_h);
                    viewport_tex = imgui_ctx.registerTexture(
                        *offscreen->sampler(), *offscreen->colorImageView());
                }

                ImGui::Image(viewport_tex, viewport_size);

                if (ImGui::IsItemHovered()) {
                    double mx, my;
                    window.getCursorPos(mx, my);
                    if (first_mouse) { last_x = mx; last_y = my; first_mouse = false; }
                    double dx = mx - last_x, dy = my - last_y;
                    last_x = mx; last_y = my;

                    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                        camera.rotate(float(-dx) * 0.005f, float(dy) * 0.005f);
                    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
                        camera.pan(float(-dx) * 0.005f, float(dy) * 0.005f);

                    float scroll = window.getScrollDelta();
                    if (scroll != 0.0f) camera.zoom(scroll * 0.3f);
                } else {
                    double mx, my;
                    window.getCursorPos(mx, my);
                    last_x = mx; last_y = my;
                    window.getScrollDelta();
                }

                float aspect = viewport_size.x / viewport_size.y;
                camera.setPerspective(45.0f, aspect, 0.1f, 1000.0f);
            }
            ImGui::End();

            ImGui::Begin("Properties");
            ImGui::SeparatorText("Lighting");
            ImGui::SliderFloat3("Light Direction", light_dir, -1.0f, 1.0f);
            ImGui::SliderFloat("Ambient", &ambient, 0.0f, 1.0f);
            ImGui::SliderFloat("Intensity", &light_intensity, 0.0f, 2.0f);

            ImGui::SeparatorText("Animation");
            ImGui::Checkbox("Animate", &animate);
            ImGui::SliderFloat("Speed", &anim_speed, 0.0f, 3.0f);

            if (m2_loaded && !bear_model.animations.empty()) {
                if (ImGui::BeginCombo("Animation", bear_model.animations[current_anim]->name().c_str())) {
                    for (int i = 0; i < static_cast<int>(bear_model.animations.size()); i++) {
                        bool selected = (i == current_anim);
                        if (ImGui::Selectable(bear_model.animations[i]->name().c_str(), selected)) {
                            current_anim = i;
                            if (animator) animator->play(bear_model.animations[i].get());
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::End();

            ImGui::Begin("Model");
            if (m2_loaded) {
                ImGui::Text("Name: %s", bear_model.name.c_str());
                ImGui::Text("Vertices: %zu", bear_model.vertices.size());
                ImGui::Text("Indices: %zu", bear_model.indices.size());
                ImGui::Text("Bones: %u", bear_model.skeleton.boneCount());
                ImGui::Text("Animations: %zu", bear_model.animations.size());
                ImGui::Text("Submeshes: %zu", bear_model.submeshes.size());
                ImGui::Text("Textures: %zu", bear_model.texture_paths.size());

                ImGui::SeparatorText("Bounding Box");
                ImGui::Text("Min: (%.1f, %.1f, %.1f)", bear_model.bbox_min.x, bear_model.bbox_min.y, bear_model.bbox_min.z);
                ImGui::Text("Max: (%.1f, %.1f, %.1f)", bear_model.bbox_max.x, bear_model.bbox_max.y, bear_model.bbox_max.z);

                if (ImGui::CollapsingHeader("Texture Paths")) {
                    for (size_t i = 0; i < bear_model.texture_paths.size(); i++) {
                        const auto& path = bear_model.texture_paths[i];
                        ImGui::Text("[%zu] %s", i, path.empty() ? "(replaceable)" : path.c_str());
                    }
                }

                if (ImGui::CollapsingHeader("Bones")) {
                    for (uint32_t i = 0; i < bear_model.skeleton.boneCount(); i++) {
                        const auto& bone = bear_model.skeleton.getBone(i);
                        ImGui::Text("[%u] %s (parent: %d)", i, bone.name.c_str(), bone.parent_index);
                    }
                }
            } else {
                ImGui::Text("No M2 model loaded. Run asset-extract first.");
            }
            ImGui::End();

            if (animate && animator) {
                animator->update(dt * anim_speed);
            }

            {
                std::vector<glm::mat4> matrices;
                if (animator) {
                    matrices = animator->boneMatrices();
                }
                matrices.resize(mve::Skeleton::MAX_BONES, glm::mat4{1.0f});
                bone_buffer.write(matrices.data(), bone_buffer_size);
            }

            SceneUBO scene{};
            scene.light_dir = glm::normalize(glm::vec3{light_dir[0], light_dir[1], light_dir[2]});
            scene.ambient = ambient;
            scene.light_color = glm::vec3{1.0f};
            scene.light_intensity = light_intensity;
            ubo_buffer.write(&scene, sizeof(SceneUBO));

            vk::raii::CommandBuffer* command_buffer;
            if (!renderer.beginFrame(&command_buffer)) continue;

            offscreen->beginRendering(*command_buffer);

            bg_pipeline->bind(*command_buffer);
            command_buffer->draw(3, 1, 0, 0);

            {
                ground_pipeline->bind(*command_buffer);

                glm::mat4 ground_model{1.0f};
                PushConstants ground_push_data{};
                ground_push_data.model = ground_model;
                ground_push_data.mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * ground_model;
                command_buffer->pushConstants<PushConstants>(
                    *ground_pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, ground_push_data);

                // ground_mesh.bind(*command_buffer);
                // ground_mesh.draw(*command_buffer);
            }

            {
                pipeline->bind(*command_buffer);
                command_buffer->bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics, *pipeline_layout, 0, *descriptor_set, nullptr);

                // Convert WoW Z-up to our Y-up, and offset so feet sit on ground (Y=0)
                glm::mat4 model = glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, model_ground_offset, 0.0f});
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3{1, 0, 0});

                PushConstants push{};
                push.model = model;
                push.mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * model;
                command_buffer->pushConstants<PushConstants>(
                    *pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, push);

                mesh->bind(*command_buffer);
                mesh->draw(*command_buffer);
            }

            offscreen->endRendering(*command_buffer);

            renderer.beginRendering(*command_buffer, false);
            imgui_ctx.render(*command_buffer);
            renderer.endRendering(*command_buffer);

            renderer.endFrame(*command_buffer);
        }

        device.device().waitIdle();

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
