/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/pipeline/pipeline.hpp"

#include "renderer/device.hpp"
#include "renderer/shader_set.hpp"

namespace mcvk::Renderer {
    class GraphicsPipeline : public Pipeline<VkGraphicsPipelineCreateInfo> {
    public:
        struct Config {
            VkPipelineViewportStateCreateInfo viewport_info;

            VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
            VkPipelineRasterizationStateCreateInfo rasterization_info;
            VkPipelineMultisampleStateCreateInfo multisample_info;

            VkPipelineColorBlendAttachmentState color_blend_attachment;
            VkPipelineColorBlendStateCreateInfo color_blend_info;

            VkPipelineDepthStencilStateCreateInfo depth_stencil_info;

            std::vector<VkDynamicState> dynamic_states;
            VkPipelineDynamicStateCreateInfo dynamic_state_info;

            VkRenderPass render_pass = nullptr;
            uint32_t subpass = 0;

            std::vector<VkPushConstantRange> push_constant_ranges;

            static Config Defaults();
        };

        GraphicsPipeline(const Device &device, const std::vector<ShaderInfo> &shaders, const Config &config);

        static void BuildGraphicsPipelines(const Device &device, const std::vector<GraphicsPipeline *> &pipelines);

    private:
        void _BuildLayout() override;
        void _BuildCreateInfo() override;

        Config _config;

        std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;

        std::vector<VkVertexInputBindingDescription> _vertex_bindings;
        std::vector<VkVertexInputAttributeDescription> _vertex_attribs;
        VkPipelineVertexInputStateCreateInfo _vertex_input_info{};
    };
}
