/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "pipeline_set.hpp"

#include "renderer/pipeline/pipeline.hpp"

namespace mcvk::Renderer {
    PipelineSet::PipelineSet(const Device &device, const std::unique_ptr<Swapchain> &swapchain)
        : _device{device}, _swapchain{swapchain} {
    }

    void PipelineSet::_Initialise() {
        _CreateGraphicsPipelines();
    }

    void PipelineSet::_CreateGraphicsPipelines() {
        auto shaders = _GetShaders();
        auto config = GraphicsPipeline::Config::Defaults();
        config.render_pass = _swapchain->GetRenderPass();

        // _g_simple pipeline
        {
            config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
            _g_simple = std::make_unique<GraphicsPipeline>(_device, shaders, config);
        }

        // _g_simple_wireframe pipeline
        {
            config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
            config.rasterization_info.polygonMode = VK_POLYGON_MODE_LINE;
            _g_simple_wireframe = std::make_unique<GraphicsPipeline>(_device, shaders, config);
        }

        GraphicsPipeline::BuildGraphicsPipelines(_device, {
            &*_g_simple,
            &*_g_simple_wireframe,
        });
    }

    std::vector<ShaderInfo> PipelineSet::_GetShaders() {
        return std::vector<ShaderInfo>{
            { ShaderStage::Vertex,      "build/spv/simple_vert.glsl.spv" },
            { ShaderStage::Fragment,    "build/spv/simple_frag.glsl.spv" }
        };
    }
}
