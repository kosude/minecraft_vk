/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "pipeline_set.hpp"

#include "renderer/pipeline/pipeline.hpp"

namespace mcvk::Renderer {
    PipelineSet::PipelineSet(const Device &device, const std::unique_ptr<Swapchain> &swapchain, const ResourceMgr::ResourceManager &resmgr)
        : _device{device}, _swapchain{swapchain}, _resmgr{resmgr} {
    }

    void PipelineSet::_Initialise(const std::vector<VkDescriptorSetLayout> &set_layouts) {
        _CreateGraphicsPipelines(set_layouts);
    }

    void PipelineSet::_CreateGraphicsPipelines(const std::vector<VkDescriptorSetLayout> &set_layouts) {
        auto config = GraphicsPipeline::Config::Defaults();
        config.render_pass = _swapchain->GetRenderPass();
        config.set_layouts = set_layouts;

        ResourceMgr::ShaderResource shader;
        _resmgr.Load("simple.shader", shader);

        // _g_simple pipeline
        {
            config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
            _g_simple = std::make_unique<GraphicsPipeline>(_device, shader.shaders, config);
        }

        // _g_simple_wireframe pipeline
        {
            config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
            config.rasterization_info.polygonMode = VK_POLYGON_MODE_LINE;
            _g_simple_wireframe = std::make_unique<GraphicsPipeline>(_device, shader.shaders, config);
        }

        GraphicsPipeline::BuildGraphicsPipelines(_device, {
            &*_g_simple,
            &*_g_simple_wireframe,
        });
    }
}
