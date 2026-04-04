#include "core/window.h"
#include "core/device.h"
#include "core/renderer.h"
#include "resources/pipeline.h"
#include "scene/mesh.h"
#include "scene/camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

struct PushConstants {
    glm::mat4 mvp;
    glm::mat4 model;
};

int main() {
    try {
        mve::Window window{1280, 720, "Mollen Wow Tools"};
        mve::Device device{window};
        mve::Renderer renderer{window, device};

        // Pipeline layout with push constants
        vk::PushConstantRange push_range{
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(PushConstants)
        };

        vk::PipelineLayoutCreateInfo layout_info{};
        layout_info.setPushConstantRanges(push_range);
        auto pipeline_layout = device.device().createPipelineLayout(layout_info);

        // Pipeline config with vertex input and depth
        auto pipeline_config = mve::PipelineConfig::defaultConfig();
        pipeline_config.pipeline_layout = *pipeline_layout;
        pipeline_config.color_attachment_format = renderer.getSwapchainImageFormat();
        pipeline_config.depth_attachment_format = renderer.getDepthFormat();
        pipeline_config.binding_descriptions = mve::Vertex::getBindingDescriptions();
        pipeline_config.attribute_descriptions = mve::Vertex::getAttributeDescriptions();

        std::string shader_dir = MVE_SHADER_DIR;
        auto pipeline = std::make_unique<mve::Pipeline>(
            device,
            shader_dir + "/basic.vert.spv",
            shader_dir + "/basic.frag.spv",
            pipeline_config);

        // Create test cube
        auto cube = mve::Mesh::createCube(device, {0.6f, 0.7f, 0.9f});

        // Camera
        mve::Camera camera;
        camera.setOrbit(3.0f, 0.5f, 0.3f);

        double last_x = 0.0, last_y = 0.0;
        bool first_mouse = true;

        while (!window.shouldClose()) {
            glfwPollEvents();

            // Mouse input
            double mx, my;
            window.getCursorPos(mx, my);

            if (first_mouse) {
                last_x = mx;
                last_y = my;
                first_mouse = false;
            }

            double dx = mx - last_x;
            double dy = my - last_y;
            last_x = mx;
            last_y = my;

            // Left mouse: orbit
            if (window.isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
                camera.rotate(static_cast<float>(-dx) * 0.005f,
                              static_cast<float>(dy) * 0.005f);
            }

            // Middle mouse: pan
            if (window.isMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
                camera.pan(static_cast<float>(-dx) * 0.005f,
                           static_cast<float>(dy) * 0.005f);
            }

            // Scroll: zoom
            float scroll = window.getScrollDelta();
            if (scroll != 0.0f) {
                camera.zoom(scroll * 0.3f);
            }

            // Update projection for current aspect ratio
            auto extent = renderer.getSwapchainExtent();
            if (extent.width > 0 && extent.height > 0) {
                float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);
                camera.setPerspective(45.0f, aspect, 0.1f, 100.0f);
            }

            // Render
            vk::raii::CommandBuffer* command_buffer;
            if (!renderer.beginFrame(&command_buffer)) continue;

            renderer.beginRendering(*command_buffer);

            pipeline->bind(*command_buffer);

            // Push MVP
            glm::mat4 model = glm::mat4{1.0f};
            PushConstants push{};
            push.model = model;
            push.mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * model;

            command_buffer->pushConstants<PushConstants>(
                *pipeline_layout,
                vk::ShaderStageFlagBits::eVertex,
                0,
                push);

            cube.bind(*command_buffer);
            cube.draw(*command_buffer);

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
