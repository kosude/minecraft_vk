/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/pipeline/graphics_pipeline.hpp"
#include "renderer/pipeline/pipeline_set.hpp"
#include "renderer/command_buffer.hpp"
#include "renderer/device.hpp"
#include "renderer/instance_manager.hpp"
#include "renderer/shader_set.hpp"
#include "renderer/swapchain.hpp"
#include "renderer/window.hpp"

#include "resource_mgr/resource_mgr.hpp"

#include <volk/volk.h>

#include <vector>
#include <memory>

namespace mcvk::Renderer {
    class Renderer {
    public:
        Renderer(const Window &window, const ResourceMgr::ResourceManager &resmgr);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        void BuildPipelines(const std::vector<VkDescriptorSetLayout> &set_layouts);

        inline const Device &GetDevice() const { return _device; }
        inline const PipelineSet &Pipelines() const { return _pipeline_set; }

        void WaitDeviceIdle();

        CommandBuffer *BeginDrawCommandBuffer();

    private:
        friend class CommandBuffer;

        void _RecreateSwapchain();
        void _CreateCommandBuffers();

        InstanceManager _instance_mgr;

        const Window &_window;
        const VkSurfaceKHR &_surface;

        Device _device;
        PipelineSet _pipeline_set;

        std::unique_ptr<Swapchain> _swapchain;
        CommandBuffer _draw_command_buffer;
    };
}
