/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"
#include "renderer/shader_set.hpp"

namespace mcvk::Renderer {
    class GraphicsPipeline {
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
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline &) = delete;
        GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;

        inline const VkGraphicsPipelineCreateInfo &GetCreateInfo() const { return _info; }
        inline const VkPipeline &GetPipeline() const { return _pipeline; }

        void CmdBind(VkCommandBuffer cmdbuf);

    private:
        friend class PipelineFactory;

        void _BuildLayout();
        void _BuildCreateInfo();

        const Device &_device;

        ShaderSet _shader_set;
        VkPipelineLayout _layout;
        VkPipeline _pipeline;

        Config _config;

        VkGraphicsPipelineCreateInfo _info{};

        std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;

        std::vector<VkVertexInputBindingDescription> _vertex_bindings;
        std::vector<VkVertexInputAttributeDescription> _vertex_attribs;
        VkPipelineVertexInputStateCreateInfo _vertex_input_info{};
    };
}
