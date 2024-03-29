/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "graphics_pipeline.hpp"

#include "renderer/data/model.hpp"

#include "utils/log.hpp"

namespace mcvk::Renderer {
    GraphicsPipeline::Config GraphicsPipeline::Config::Defaults() {
        Config config{};

        config.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        config.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        config.input_assembly_info.primitiveRestartEnable = VK_FALSE;

        config.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        config.viewport_info.viewportCount = 1;
        config.viewport_info.pViewports = nullptr;
        config.viewport_info.scissorCount = 1;
        config.viewport_info.pScissors = nullptr;

        config.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        config.rasterization_info.depthClampEnable = VK_FALSE;
        config.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
        config.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
        config.rasterization_info.lineWidth = 1.0f;
        config.rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
        config.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        config.rasterization_info.depthBiasEnable = VK_FALSE;
        config.rasterization_info.depthBiasConstantFactor = 0.0f;
        config.rasterization_info.depthBiasClamp = 0.0f;
        config.rasterization_info.depthBiasSlopeFactor = 0.0f;

        config.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        config.multisample_info.sampleShadingEnable = VK_FALSE;
        config.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        config.multisample_info.minSampleShading = 1.0f;
        config.multisample_info.pSampleMask = nullptr;
        config.multisample_info.alphaToCoverageEnable = VK_FALSE;
        config.multisample_info.alphaToOneEnable = VK_FALSE;

        config.color_blend_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        config.color_blend_attachment.blendEnable = VK_FALSE;
        config.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        config.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        config.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        config.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        config.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        config.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

        config.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        config.color_blend_info.logicOpEnable = VK_FALSE;
        config.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
        config.color_blend_info.attachmentCount = 1;
        config.color_blend_info.pAttachments = &config.color_blend_attachment;
        config.color_blend_info.blendConstants[0] = 0.0f;
        config.color_blend_info.blendConstants[1] = 0.0f;
        config.color_blend_info.blendConstants[2] = 0.0f;
        config.color_blend_info.blendConstants[3] = 0.0f;

        config.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        config.depth_stencil_info.depthTestEnable = VK_TRUE;
        config.depth_stencil_info.depthWriteEnable = VK_TRUE;
        config.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
        config.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
        config.depth_stencil_info.minDepthBounds = 0.0f;
        config.depth_stencil_info.maxDepthBounds = 1.0f;
        config.depth_stencil_info.stencilTestEnable = VK_FALSE;
        config.depth_stencil_info.front = {};
        config.depth_stencil_info.back = {};

        config.dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        config.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        config.dynamic_state_info.pDynamicStates = config.dynamic_states.data();
        config.dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(config.dynamic_states.size());
        config.dynamic_state_info.flags = 0;

        config.set_layouts = {};
        config.push_constant_ranges = {};

        return config;
    }

    GraphicsPipeline::GraphicsPipeline(const Device &device, const std::vector<ShaderInfo> &shaders, const Config &config)
        : _config{config}, Pipeline{device, shaders}{
        _BuildLayout();
        _BuildCreateInfo();
    }

    void GraphicsPipeline::BuildGraphicsPipelines(const Device &device, const std::vector<GraphicsPipeline *> &pipelines) {
        Utils::Info("Building " + std::to_string(pipelines.size()) + " graphics pipelines");

        std::vector<VkGraphicsPipelineCreateInfo> pipeline_infos(pipelines.size());
        for (size_t i = 0; i < pipeline_infos.size(); i++) {
            pipeline_infos[i] = pipelines[i]->_info;
        }

        std::vector<VkPipeline> vk_pipelines(pipelines.size());

        if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, static_cast<uint32_t>(pipeline_infos.size()),
            pipeline_infos.data(), nullptr, vk_pipelines.data()) != VK_SUCCESS) {
            Utils::Fatal("Failed to create graphics pipeline");
        }

        // store new pipeline objects in each pipeline abstraction
        for (size_t i = 0; i < vk_pipelines.size(); i++) {
            pipelines[i]->_pipeline = vk_pipelines[i];
        }
    }

    void GraphicsPipeline::_BuildLayout() {
        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount = static_cast<uint32_t>(_config.set_layouts.size());
        info.pSetLayouts = _config.set_layouts.data();
        info.pushConstantRangeCount = static_cast<uint32_t>(_config.push_constant_ranges.size());
        info.pPushConstantRanges = _config.push_constant_ranges.data();

        if (vkCreatePipelineLayout(_device.GetDevice(), &info, nullptr, &_layout) != VK_SUCCESS) {
            Utils::Fatal("Failed to create graphics pipeline layout");
        }
    }

    void GraphicsPipeline::_BuildCreateInfo() {
        _shader_stages = _shader_set.BuildShaderStageInfos();

        _vertex_bindings = Model::Vertex::GetBindingDescriptions();
        _vertex_attribs = Model::Vertex::GetAttributeDescriptions();
        _vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        _vertex_input_info.pNext = nullptr;
        _vertex_input_info.flags = 0;
        _vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(_vertex_bindings.size());
        _vertex_input_info.pVertexBindingDescriptions = _vertex_bindings.data();
        _vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(_vertex_attribs.size());
        _vertex_input_info.pVertexAttributeDescriptions = _vertex_attribs.data();

        _info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        _info.stageCount = static_cast<uint32_t>(_shader_stages.size());
        _info.pStages = _shader_stages.data();
        _info.pVertexInputState = &_vertex_input_info;
        _info.pInputAssemblyState = &_config.input_assembly_info;
        _info.pViewportState = &_config.viewport_info;
        _info.pRasterizationState = &_config.rasterization_info;
        _info.pMultisampleState = &_config.multisample_info;
        _info.pColorBlendState = &_config.color_blend_info;
        _info.pDepthStencilState = &_config.depth_stencil_info;
        _info.pDynamicState = &_config.dynamic_state_info;

        _info.layout = _layout;
        _info.renderPass = _config.render_pass;
        _info.subpass = _config.subpass;

        _info.basePipelineIndex = -1;
        _info.basePipelineHandle = VK_NULL_HANDLE;
    }
}
