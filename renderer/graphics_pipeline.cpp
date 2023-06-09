/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include "utils/log.hpp"
#include "utils/file_io.hpp"
#include "renderer/data/vertex.hpp"
#include "renderer/data/uniform.hpp"

#include "graphics_pipeline.hpp"

namespace MCVK::Renderer {
    VkShaderModule GraphicsPipeline::_CreateShaderModule(const std::vector<char> &code) {
        VkShaderModuleCreateInfo info = {};

        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = code.size();
        info.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shader_module;
        if (vkCreateShaderModule(_logical_device, &info, nullptr, &shader_module)) {
            Utils::Error("Failed to create shader module");
        }

        return shader_module;
    }

    void GraphicsPipeline::_CreateShaderStages() {
        _vertex_shader_mod = _CreateShaderModule(Utils::ReadBinaryFile("spv/main_vertex.glsl.spv"));
        _fragment_shader_mod = _CreateShaderModule(Utils::ReadBinaryFile("spv/main_fragment.glsl.spv"));

        // create vertex shader stage
        _vertex_shader_stage_info = {};
        _vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        _vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        _vertex_shader_stage_info.module = _vertex_shader_mod; // pass shader module with source code to pipeline stage
        _vertex_shader_stage_info.pName = "main"; // entrypoint

        // create fragment shader stage
        _fragment_shader_stage_info = {};
        _fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        _fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        _fragment_shader_stage_info.module = _fragment_shader_mod; // pass shader module with source code to pipeline stage
        _fragment_shader_stage_info.pName = "main"; // entrypoint

        // enumerate stages
        _shader_stage_info_arr = {
            _vertex_shader_stage_info,
            _fragment_shader_stage_info
        };
    }

    void GraphicsPipeline::_CreateDynamicState() {
        // define state components that can be dynamically changed at runtime
        // viewport + scissor are dynamic so they can be updated with window resizing
        _dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        _dynamic_state_info = {};

        _dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

        _dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(_dynamic_states.size());
        _dynamic_state_info.pDynamicStates = _dynamic_states.data();
    }

    void GraphicsPipeline::_CreateVertexInputStage() {
        _vertex_input_stage_info = {};

        _vertex_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    }

    void GraphicsPipeline::_CreateInputAssemblyStage() {
        _input_assembly_stage_info = {};

        _input_assembly_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

        // drawing triangles
        _input_assembly_stage_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        _input_assembly_stage_info.primitiveRestartEnable = VK_FALSE;
    }

    void GraphicsPipeline::_CreateViewportState() {
        // create viewport: consider entire swapchain image extent
        _viewport = {};
        _viewport.x = 0.0f;
        _viewport.y = 0.0f;
        _viewport.width = (float) _swapchain.GetExtent().width;
        _viewport.height = (float) _swapchain.GetExtent().height;
        _viewport.minDepth = 0.0f;
        _viewport.maxDepth = 1.0f;

        // create scissor: do not discard anything
        _scissor = {};
        _scissor.offset = { 0, 0 };
        _scissor.extent = _swapchain.GetExtent();

        // requesting viewport state: only count needs to be specified as they are in dynamic state
        _viewport_state_info = {};
        _viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        _viewport_state_info.viewportCount = 1;
        _viewport_state_info.scissorCount = 1;
    }

    void GraphicsPipeline::_CreateRasterizer() {
        _rasterizer_info = {};
        _rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        // if true, fragments outside of near-far bounds are clamped instead of discarded - disabled; requires requesting another GPU feature
        _rasterizer_info.depthClampEnable = VK_FALSE;

        // if true, geometry never passes through rasterizer (disables output to framebuffer)
        _rasterizer_info.rasterizerDiscardEnable = VK_FALSE;

        _rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL; // can be set to do wireframe or dots, requires more GPU features
        _rasterizer_info.lineWidth = 1.0f; // thicker than 1.0f requires another GPU feature

        // back face culling
        // _rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
        _rasterizer_info.cullMode = VK_CULL_MODE_NONE;
        _rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE; // specifies how front faces are considered

        // can be helpful for shadow mapping
        _rasterizer_info.depthBiasEnable = VK_FALSE;
    }

    void GraphicsPipeline::_CreateMultisampler() {
        _multisampler_info = {};
        _multisampler_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

        // disable multisampling
        _multisampler_info.sampleShadingEnable = VK_FALSE;
        _multisampler_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    void GraphicsPipeline::_CreateColourBlendState() {
        _colour_blend_attachment_state_info = {};

        _colour_blend_attachment_state_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        _colour_blend_attachment_state_info.blendEnable = VK_FALSE;

        _colour_blend_state_info = {};
        _colour_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        _colour_blend_state_info.logicOpEnable = VK_FALSE;
        _colour_blend_state_info.attachmentCount = 1;
        _colour_blend_state_info.pAttachments = &_colour_blend_attachment_state_info;
    }

    void GraphicsPipeline::_CreateDescriptorPool() {
        VkDescriptorPoolSize pool_size = {};
        pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_size.descriptorCount = _frames_in_flight;

        VkDescriptorPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.poolSizeCount = 1;
        info.pPoolSizes = &pool_size;
        info.maxSets = _frames_in_flight;

        if (vkCreateDescriptorPool(_logical_device, &info, nullptr, &_descriptor_pool)) {
            Utils::Error("Failed to create descriptor pool");
        }
    }

    void GraphicsPipeline::_CreateDescriptorSets(const Buffer::UniformBuffer &ubo) {
        std::vector<VkDescriptorSetLayout> layouts(_frames_in_flight, _descriptor_set_layout);

        // creating one descriptor set for each frame in flight with same layouts
        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = _descriptor_pool;
        alloc_info.descriptorSetCount = _frames_in_flight;
        alloc_info.pSetLayouts = layouts.data();

        // allocate descriptor sets
        _descriptor_sets.resize(_frames_in_flight);
        if (vkAllocateDescriptorSets(_logical_device, &alloc_info, _descriptor_sets.data())) {
            Utils::Error("Failed to create descriptor sets");
        }

        for (uint16_t i = 0; i < _frames_in_flight; i++) {
            VkDescriptorBufferInfo buffer_info = {};

            // reference the uniform buffer for that frame in flight
            buffer_info.buffer = ubo.GetObjectHandles()[i];
            buffer_info.offset = 0;
            buffer_info.range = VK_WHOLE_SIZE; // works as we are overwriting the whole buffer

            // struct to configure the descriptor configuration
            VkWriteDescriptorSet descriptor_write = {};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

            // configuring descriptor set i
            descriptor_write.dstSet = _descriptor_sets[i];
            descriptor_write.dstBinding = 0; // same binding as GLSL location
            descriptor_write.dstArrayElement = 0;

            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_write.descriptorCount = 1;

            // this descriptor refers to buffer data
            descriptor_write.pBufferInfo = &buffer_info;

            vkUpdateDescriptorSets(_logical_device, 1, &descriptor_write, 0, nullptr);
        }
    }

    void GraphicsPipeline::_CreateDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding ubo_layout_binding = {};
        ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.descriptorCount = 1;

        ubo_layout_binding.binding = 0; // binding as used in the shader

        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 1;
        info.pBindings = &ubo_layout_binding;

        if (vkCreateDescriptorSetLayout(_logical_device, &info, nullptr, &_descriptor_set_layout)) {
            Utils::Error("Failed to create descriptor set layout");
        }
    }

    void GraphicsPipeline::_CreatePipelineLayout() {
        VkPipelineLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        info.setLayoutCount = 1;
        info.pSetLayouts = &_descriptor_set_layout;

        if (vkCreatePipelineLayout(_logical_device, &info, nullptr, &_pipeline_layout)) {
            Utils::Error("Failed to create pipeline layout");
        }
    }

    void GraphicsPipeline::_CreateRenderPass() {
        // single colour buffer attachment
        VkAttachmentDescription colour_attachment = {};
        colour_attachment.format = _swapchain.GetImageFormat();
        colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // not using multisampling
        colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear attachment data to a constant e.g. black
        colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store attachment data after render call

        // not using stencil buffer, so not relevant...
        colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // we want images to be ready for presentation after rendering, so we use VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // subpass reference to colour attachment
        VkAttachmentReference colour_attachment_ref = {};
        colour_attachment_ref.attachment = 0; // index of attachment in array (we only have one -> there is no array => index 0)
        colour_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // best performance for colour buffers

        // subpass - we only use one as we aren't doing e.g. post processing
        VkSubpassDescription subpass = {};

        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        // index of colour attachment is referenced by layout(location = 0) directive in fragment shaders
        subpass.pColorAttachments = &colour_attachment_ref;

        // subpass dependency to wait for colour attachemnt thus wait for an image to be available to render to
        VkSubpassDependency subpass_dependency = {};
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = 0;

        // wait for swapchain to finish reading from the image before accessing it
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &colour_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &subpass_dependency;

        if (vkCreateRenderPass(_logical_device, &render_pass_info, nullptr, &_render_pass)) {
            Utils::Error("Failed to create render pass");
        }
    }

    GraphicsPipeline::GraphicsPipeline(const VkDevice &logical_device, const Swapchain &swapchain, const uint16_t &frames_in_flight,
        const Buffer::UniformBuffer &ubo) : _logical_device(logical_device), _swapchain(swapchain), _frames_in_flight(frames_in_flight)
    {
        // constructing creation info structs
        _CreateDescriptorSetLayout();
        _CreateDescriptorPool();
        _CreateDescriptorSets(ubo);
        _CreateShaderStages();
        _CreateVertexInputStage();
        _CreateDynamicState();
        _CreateInputAssemblyStage();
        _CreateViewportState(); // note that viewport state is dynamic so will not actually be set up until runtime
        _CreateRasterizer();
        _CreateMultisampler();
        _CreateColourBlendState();
        _CreatePipelineLayout();
        _CreateRenderPass();

        // create grpahics pipeline...

        VkGraphicsPipelineCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        info.stageCount = 2;
        info.pStages = _shader_stage_info_arr.data();

        // vertex input binding description(s)
        auto binding_description = Data::Vertex::GetBindingDescription();
        _vertex_input_stage_info.vertexBindingDescriptionCount = 1;
        _vertex_input_stage_info.pVertexBindingDescriptions = &binding_description;

        // vertex input attribute description(s)
        auto attribute_descriptions = Data::Vertex::GetAttributeDescriptions();
        _vertex_input_stage_info.vertexAttributeDescriptionCount = attribute_descriptions.size();
        _vertex_input_stage_info.pVertexAttributeDescriptions = attribute_descriptions.data();

        info.pVertexInputState = &_vertex_input_stage_info;
        info.pInputAssemblyState = &_input_assembly_stage_info;
        info.pViewportState = &_viewport_state_info;
        info.pRasterizationState = &_rasterizer_info;
        info.pMultisampleState = &_multisampler_info;
        info.pDepthStencilState = nullptr;
        info.pColorBlendState = &_colour_blend_state_info;
        info.pDynamicState = &_dynamic_state_info;

        info.layout = _pipeline_layout;
        info.renderPass = _render_pass;
        info.subpass = 0;

        if (vkCreateGraphicsPipelines(_logical_device, VK_NULL_HANDLE, 1, &info, nullptr, &_handle)) {
            Utils::Error("Failed to create graphics pipeline");
        }
    }

    void GraphicsPipeline::Destroy() {
        vkDestroyPipeline(_logical_device, _handle, nullptr);
        vkDestroyPipelineLayout(_logical_device, _pipeline_layout, nullptr);

        vkDestroyDescriptorPool(_logical_device, _descriptor_pool, nullptr);
        vkDestroyDescriptorSetLayout(_logical_device, _descriptor_set_layout, nullptr);

        vkDestroyRenderPass(_logical_device, _render_pass, nullptr);

        vkDestroyShaderModule(_logical_device, _vertex_shader_mod, nullptr);
        vkDestroyShaderModule(_logical_device, _fragment_shader_mod, nullptr);
    }
}
