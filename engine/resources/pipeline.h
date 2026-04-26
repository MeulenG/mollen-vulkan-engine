#ifndef MVE_PIPELINE_H
#define MVE_PIPELINE_H

#include "../core/device.h"

#include <string>
#include <vector>

namespace mve {

struct PipelineConfig {
    vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
    vk::PipelineRasterizationStateCreateInfo rasterization_info;
    vk::PipelineMultisampleStateCreateInfo multisample_info;
    vk::PipelineColorBlendAttachmentState color_blend_attachment;
    vk::PipelineDepthStencilStateCreateInfo depth_stencil_info;
    vk::PipelineViewportStateCreateInfo viewport_info;
    std::vector<vk::DynamicState> dynamic_states;
    vk::PipelineDynamicStateCreateInfo dynamic_state_info;
    vk::PipelineLayout pipeline_layout;
    vk::Format color_attachment_format = vk::Format::eUndefined;
    vk::Format depth_attachment_format = vk::Format::eUndefined;

    std::vector<vk::VertexInputBindingDescription> binding_descriptions;
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions;

    static PipelineConfig DefaultConfig();
};

class Pipeline {
public:
    Pipeline(
        Device& device,
        const std::string& vert_path,
        const std::string& frag_path,
        const PipelineConfig& config);

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void Bind(const vk::raii::CommandBuffer& command_buffer);

    static std::vector<char> ReadFile(const std::string& filepath);

private:
    void createGraphicsPipeline(
        const std::string& vert_path,
        const std::string& frag_path,
        const PipelineConfig& config);

    vk::raii::ShaderModule createShaderModule(const std::vector<char>& code);

    Device& device_;
    vk::raii::Pipeline graphics_pipeline_{nullptr};
};

} // namespace mve

#endif // MVE_PIPELINE_H
