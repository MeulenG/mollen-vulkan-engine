#ifndef MVE_PIPELINE_H
#define MVE_PIPELINE_H

#include "../core/device.h"

#include <string>
#include <vector>

namespace mve {

struct PipelineConfig {
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
    VkPipelineRasterizationStateCreateInfo rasterization_info{};
    VkPipelineMultisampleStateCreateInfo multisample_info{};
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
    VkPipelineViewportStateCreateInfo viewport_info{};
    std::vector<VkDynamicState> dynamic_states;
    VkPipelineDynamicStateCreateInfo dynamic_state_info{};
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkFormat color_attachment_format = VK_FORMAT_UNDEFINED;

    static PipelineConfig defaultConfig();
};

class Pipeline {
public:
    Pipeline(
        Device& device,
        const std::string& vert_path,
        const std::string& frag_path,
        const PipelineConfig& config);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void bind(VkCommandBuffer command_buffer);

    static std::vector<char> readFile(const std::string& filepath);

private:
    void createGraphicsPipeline(
        const std::string& vert_path,
        const std::string& frag_path,
        const PipelineConfig& config);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    Device& device_;
    VkPipeline graphics_pipeline_ = VK_NULL_HANDLE;
};

} // namespace mve

#endif // MVE_PIPELINE_H
