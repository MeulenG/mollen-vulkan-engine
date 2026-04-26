#include "pipeline.h"

#include <fstream>
#include <stdexcept>

namespace mve {

PipelineConfig PipelineConfig::DefaultConfig() {
    PipelineConfig config{};

    config.input_assembly_info.topology = vk::PrimitiveTopology::eTriangleList;
    config.input_assembly_info.primitiveRestartEnable = vk::False;

    config.viewport_info.viewportCount = 1;
    config.viewport_info.scissorCount = 1;

    config.rasterization_info.depthClampEnable = vk::False;
    config.rasterization_info.rasterizerDiscardEnable = vk::False;
    config.rasterization_info.polygonMode = vk::PolygonMode::eFill;
    config.rasterization_info.lineWidth = 1.0f;
    config.rasterization_info.cullMode = vk::CullModeFlagBits::eNone;
    config.rasterization_info.frontFace = vk::FrontFace::eClockwise;
    config.rasterization_info.depthBiasEnable = vk::False;

    config.multisample_info.sampleShadingEnable = vk::False;
    config.multisample_info.rasterizationSamples = vk::SampleCountFlagBits::e1;

    config.color_blend_attachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    config.color_blend_attachment.blendEnable = vk::False;

    config.depth_stencil_info.depthTestEnable = vk::True;
    config.depth_stencil_info.depthWriteEnable = vk::True;
    config.depth_stencil_info.depthCompareOp = vk::CompareOp::eLess;
    config.depth_stencil_info.depthBoundsTestEnable = vk::False;
    config.depth_stencil_info.stencilTestEnable = vk::False;

    config.rasterization_info.cullMode = vk::CullModeFlagBits::eBack;
    config.rasterization_info.frontFace = vk::FrontFace::eCounterClockwise;

    config.dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    config.dynamic_state_info.setDynamicStates(config.dynamic_states);

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

void Pipeline::Bind(const vk::raii::CommandBuffer& command_buffer) {
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphics_pipeline_);
}

std::vector<char> Pipeline::ReadFile(const std::string& filepath) {
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

    auto vert_module = createShaderModule(ReadFile(vert_path));
    auto frag_module = createShaderModule(ReadFile(frag_path));

    std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages{
        vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, *vert_module, "main"},
        vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eFragment, *frag_module, "main"}
    };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.setVertexBindingDescriptions(config.binding_descriptions);
    vertex_input_info.setVertexAttributeDescriptions(config.attribute_descriptions);

    vk::PipelineColorBlendStateCreateInfo color_blend_info{};
    color_blend_info.setAttachments(config.color_blend_attachment);

    // Dynamic rendering (Vulkan 1.3+ core)
    vk::PipelineRenderingCreateInfo rendering_info{};
    rendering_info.setColorAttachmentFormats(config.color_attachment_format);
    if (config.depth_attachment_format != vk::Format::eUndefined) {
        rendering_info.depthAttachmentFormat = config.depth_attachment_format;
    }

    vk::GraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.pNext = &rendering_info;
    pipeline_info.setStages(shader_stages);
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &config.input_assembly_info;
    pipeline_info.pViewportState = &config.viewport_info;
    pipeline_info.pRasterizationState = &config.rasterization_info;
    pipeline_info.pMultisampleState = &config.multisample_info;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDepthStencilState = &config.depth_stencil_info;
    pipeline_info.pDynamicState = &config.dynamic_state_info;
    pipeline_info.layout = config.pipeline_layout;

    graphics_pipeline_ = device_.GetDevice().createGraphicsPipeline(nullptr, pipeline_info);
}

vk::raii::ShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo create_info{
        {},
        code.size(),
        reinterpret_cast<const uint32_t*>(code.data())
    };

    return device_.GetDevice().createShaderModule(create_info);
}

} // namespace mve
