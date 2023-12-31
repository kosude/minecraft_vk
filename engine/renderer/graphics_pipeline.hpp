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

            VkPipelineLayout layout = nullptr;
            VkRenderPass render_pass = nullptr;
            uint32_t subpass = 0;

            static Config Defaults();
        };

        GraphicsPipeline(const Device &device, const std::vector<ShaderInfo> &shaders, const Config &config);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline &) = delete;
        GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;

    private:
        void _CreateGraphicsPipeline(const Config &config);

        const Device &_device;

        ShaderSet _shader_set;
        VkPipeline _pipeline;
    };
}
