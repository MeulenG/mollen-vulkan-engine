#include "pipeline.h"

#include <fstream>
#include <stdexcept>

namespace mve {

PipelineConfig PipelineConfig::defaultConfig() {
    PipelineConfig config{};

    config.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.input_assembly_info.primitiveRestartEnable = VK_FALSE;

    config.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    config.viewport_info.viewportCount = 1;
    config.viewport_info.scissorCount = 1;

    config.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config.rasterization_info.depthClampEnable = VK_FALSE;
    config.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    config.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    config.rasterization_info.lineWidth = 1.0f;
    config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
    config.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    config.rasterization_info.depthBiasEnable = VK_FALSE;

    config.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config.multisample_info.sampleShadingEnable = VK_FALSE;
    config.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    config.color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    config.color_blend_attachment.blendEnable = VK_FALSE;

    config.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config.depth_stencil_info.depthTestEnable = VK_FALSE;
    config.depth_stencil_info.depthWriteEnable = VK_FALSE;
    config.depth_stencil_info.stencilTestEnable = VK_FALSE;

    config.dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    config.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    config.dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(config.dynamic_states.size());
    config.dynamic_state_info.pDynamicStates = config.dynamic_states.data();

    return config;
}

Pipeline::Pipeline(
    Device& device,
    const std::string& vert_path,
    const std::string& frag_path,
    const PipelineConfig& config)
    : device_{device} {
    createGraphicsPipeline(vert_path, frag_path, config);
}

Pipeline::~Pipeline() {
    vkDestroyPipeline(device_.device(), graphics_pipeline_, nullptr);
}

void Pipeline::bind(VkCommandBuffer command_buffer) {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);
}

std::vector<char> Pipeline::readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    return buffer;
}

void Pipeline::createGraphicsPipeline(
    const std::string& vert_path,
    const std::string& frag_path,
    const PipelineConfig& config) {

    auto vert_code = readFile(vert_path);
    auto frag_code = readFile(frag_path);

    VkShaderModule vert_module = createShaderModule(vert_code);
    VkShaderModule frag_module = createShaderModule(frag_code);

    VkPipelineShaderStageCreateInfo shader_stages[2]{};

    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vert_module;
    shader_stages[0].pName = "main";

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = frag_module;
    shader_stages[1].pName = "main";

    // No vertex input for hardcoded triangle
    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.vertexBindingDescriptionCount = 0;

    VkPipelineColorBlendStateCreateInfo color_blend_info{};
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &config.color_blend_attachment;

    // Dynamic rendering — no render pass needed (Vulkan 1.3+ core)
    VkPipelineRenderingCreateInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &config.color_attachment_format;

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = &rendering_info;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &config.input_assembly_info;
    pipeline_info.pViewportState = &config.viewport_info;
    pipeline_info.pRasterizationState = &config.rasterization_info;
    pipeline_info.pMultisampleState = &config.multisample_info;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDepthStencilState = &config.depth_stencil_info;
    pipeline_info.pDynamicState = &config.dynamic_state_info;
    pipeline_info.layout = config.pipeline_layout;
    pipeline_info.renderPass = VK_NULL_HANDLE; // dynamic rendering
    pipeline_info.subpass = 0;

    if (vkCreateGraphicsPipelines(device_.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    vkDestroyShaderModule(device_.device(), vert_module, nullptr);
    vkDestroyShaderModule(device_.device(), frag_module, nullptr);
}

VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device_.device(), &create_info, nullptr, &shader_module) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }

    return shader_module;
}

} // namespace mve
