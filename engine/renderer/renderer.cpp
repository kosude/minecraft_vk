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
        _RecreateSwapchain();
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

    void Renderer::_RecreateSwapchain() {
        VkExtent2D extent = _window.GetExtent();
        while (extent.width <= 0 || extent.height <= 0) {
            extent = _window.GetExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(_device.LogicalDevice());

        if (!_swapchain) {
            _swapchain = std::make_unique<Swapchain>(_device, _surface, extent);
        } else {
            // recreate from existing swapchain when possible
            std::shared_ptr<Swapchain> old = std::move(_swapchain);
            _swapchain = std::make_unique<Swapchain>(_device, _surface, extent, old);

            if (!old->CompareSwapFormats(*(_swapchain.get()))) {
                Utils::Fatal("When recreating swap chain: image or depth buffer format has changed");
            }
        }
    }

    std::vector<ShaderInfo> Renderer::_GetShaders() {
        return std::vector<ShaderInfo>{
            { ShaderStage::Vertex,      "build/spv/simple_vert.glsl.spv" },
            { ShaderStage::Fragment,    "build/spv/simple_frag.glsl.spv" }
        };
    }
}
