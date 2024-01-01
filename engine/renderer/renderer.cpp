/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "renderer.hpp"

#include "renderer/pipeline/pipeline_factory.hpp"
#include "renderer/window.hpp"
#include "utils/log.hpp"

namespace mcvk::Renderer {
    Renderer::Renderer(const Window &window)
        : _window{window},
        _instance_mgr{window},
        _surface{_instance_mgr.GetSurface()},
        _device{window, _instance_mgr.GetInstance(), _surface},
        _draw_command_buffer{_device, _swapchain} {
        _RecreateSwapchain();
        _CreatePipelines();
        _CreateCommandBuffers();
    }

    Renderer::~Renderer() {
    }

    void Renderer::WaitDeviceIdle() {
        vkDeviceWaitIdle(_device.GetDevice());
    }

    CommandBuffer *Renderer::BeginDrawCommandBuffer() {
        _draw_command_buffer._Begin();
        return &_draw_command_buffer;
    }

    void Renderer::_CreatePipelines() {
        std::vector<ShaderInfo> shaders = _GetShaders();

        auto config = GraphicsPipeline::Config::Defaults();
        config.render_pass = _swapchain->GetRenderPass();

        _default_pipeline = std::make_unique<GraphicsPipeline>(_device, shaders, config);

        PipelineFactory::BuildPipelines(_device, { _default_pipeline.get() });
    }

    void Renderer::_RecreateSwapchain() {
        VkExtent2D extent = _window.GetExtent();
        while (extent.width <= 0 || extent.height <= 0) {
            extent = _window.GetExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(_device.GetDevice());

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

    void Renderer::_CreateCommandBuffers() {
        _draw_command_buffer._Initialise(this);
    }

    std::vector<ShaderInfo> Renderer::_GetShaders() {
        return std::vector<ShaderInfo>{
            { ShaderStage::Vertex,      "build/spv/simple_vert.glsl.spv" },
            { ShaderStage::Fragment,    "build/spv/simple_frag.glsl.spv" }
        };
    }
}
