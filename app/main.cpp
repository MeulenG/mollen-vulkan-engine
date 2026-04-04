#include "core/window.h"
#include "core/device.h"
#include "core/renderer.h"
#include "resources/pipeline.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

int main() {
    try {
        mve::Window window{1280, 720, "Mollen Wow Tools"};
        mve::Device device{window};
        mve::Renderer renderer{window, device};

        // Pipeline layout (empty for now — no push constants or descriptors)
        VkPipelineLayout pipeline_layout;
        VkPipelineLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        if (vkCreatePipelineLayout(device.device(), &layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }

        // Create graphics pipeline
        auto pipeline_config = mve::PipelineConfig::defaultConfig();
        pipeline_config.pipeline_layout = pipeline_layout;
        pipeline_config.color_attachment_format = renderer.getSwapchainImageFormat();

        std::string shader_dir = MVE_SHADER_DIR;
        auto pipeline = std::make_unique<mve::Pipeline>(
            device,
            shader_dir + "/basic.vert.spv",
            shader_dir + "/basic.frag.spv",
            pipeline_config);

        while (!window.shouldClose()) {
            glfwPollEvents();

            VkCommandBuffer command_buffer;
            if (!renderer.beginFrame(&command_buffer)) continue;

            renderer.beginRendering(command_buffer);

            pipeline->bind(command_buffer);
            vkCmdDraw(command_buffer, 3, 1, 0, 0);

            renderer.endRendering(command_buffer);
            renderer.endFrame(command_buffer);
        }

        vkDeviceWaitIdle(device.device());

        pipeline.reset();
        vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
