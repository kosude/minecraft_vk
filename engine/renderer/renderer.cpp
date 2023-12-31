/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "renderer.hpp"

#include "renderer/window.hpp"
#include "utils/log.hpp"

namespace mcvk::Renderer {
    Renderer::Renderer(const Window &window)
        : _window{window},
        _instance_mgr{window, _surface},
        _device{window, _instance_mgr.Instance(), _surface} {
        _CreatePipelines();
    }

    Renderer::~Renderer() {
        vkDestroySurfaceKHR(_instance_mgr.Instance(), _surface, nullptr);
    }

    void Renderer::_CreatePipelines() {
        std::vector<ShaderInfo> shaders = _GetShaders();

        auto config = GraphicsPipeline::Config::Defaults();

        _default_pipeline = std::make_unique<GraphicsPipeline>(_device, shaders, config);
    }

    std::vector<ShaderInfo> Renderer::_GetShaders() {
        return std::vector<ShaderInfo>{
            { ShaderStage::Vertex,      "build/spv/simple_vert.glsl.spv" },
            { ShaderStage::Fragment,    "build/spv/simple_frag.glsl.spv" }
        };
    }
}
