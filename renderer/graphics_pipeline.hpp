/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __renderer__graphics_pipeline_hpp
#define __renderer__graphics_pipeline_hpp

#include <volk/volk.h>

#include <vector>

#include "renderer/swap_chain.hpp"

// Note that 'state' and 'stage' are often used interchangeably which is dumb and wrong, but I'm quite lazy and can't be bothered to fix this.
// This could be a result of my inability to read.

namespace MCVK::Renderer {
    /**
     * @brief Vulkan graphics pipeline
     */
    class GraphicsPipeline {
    private:
        VkPipeline _handle;

        VkDevice _logical_device;
        Swapchain _swapchain;

        VkShaderModule _vertex_shader_mod, _fragment_shader_mod;
        VkShaderModule _CreateShaderModule(
            const std::vector<char> &code
        );

        // shader stage (involves shader source code modules) configuration
        VkPipelineShaderStageCreateInfo _vertex_shader_stage_info, _fragment_shader_stage_info;
        std::vector<VkPipelineShaderStageCreateInfo> _shader_stage_info_arr;
        void _CreateShaderStages();

        // pipeline dynamic state specifications
        std::vector<VkDynamicState> _dynamic_states;
        VkPipelineDynamicStateCreateInfo _dynamic_state_info;
        void _CreateDynamicState();

        // vertex input configuration
        VkPipelineVertexInputStateCreateInfo _vertex_input_stage_info;
        void _CreateVertexInputStage();

        // vertex input assembly stage configuration
        VkPipelineInputAssemblyStateCreateInfo _input_assembly_stage_info;
        void _CreateInputAssemblyStage();

        // initial viewport and scissor configuration
        VkViewport _viewport;
        VkRect2D _scissor;
        VkPipelineViewportStateCreateInfo _viewport_state_info;
        void _CreateViewportState();

        // rasterizer stage configuration
        VkPipelineRasterizationStateCreateInfo _rasterizer_info;
        void _CreateRasterizer();

        // multisampler stage configuration
        VkPipelineMultisampleStateCreateInfo _multisampler_info;
        void _CreateMultisampler();

        // colour blending stage/attachment configuration
        VkPipelineColorBlendAttachmentState _colour_blend_attachment_state_info;
        VkPipelineColorBlendStateCreateInfo _colour_blend_state_info;
        void _CreateColourBlendState();

        // layout to hold uniform definitions
        VkPipelineLayout _pipeline_layout;
        void _CreatePipelineLayout();

        // render pass stage
        VkRenderPass _render_pass;
        void _CreateRenderPass();

    public:
        /**
         * @brief Create graphics pipeline
         */
        GraphicsPipeline(
            const VkDevice &logical_device,
            const Swapchain &swapchain
        );

        /**
         * @brief Clean up graphics pipeline
         */
        void Destroy();
    };
}

#endif
