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
    Renderer::Renderer(const Window &window, const ResourceMgr::ResourceManager &resmgr)
        : _window{window},
        _instance_mgr{window},
        _surface{_instance_mgr.GetSurface()},
        _device{window, _instance_mgr.GetInstance(), _surface},
        _pipeline_set{_device, _swapchain, resmgr},
        _draw_command_buffer{_device, _swapchain} {
        _RecreateSwapchain();
        _CreateCommandBuffers();
    }

    Renderer::~Renderer() {
    }

    void Renderer::BuildPipelines(const std::vector<VkDescriptorSetLayout> &set_layouts) {
        _pipeline_set._Initialise(set_layouts);
    }

    void Renderer::WaitDeviceIdle() {
        vkDeviceWaitIdle(_device.GetDevice());
    }

    CommandBuffer *Renderer::BeginDrawCommandBuffer() {
        _draw_command_buffer._Begin();
        return &_draw_command_buffer;
    }

    void Renderer::_RecreateSwapchain() {
        VkExtent2D extent = _window.GetExtent();
        // block while window is minimised
        while (extent.width == 0 || extent.height == 0) {
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
}
