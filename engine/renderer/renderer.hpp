/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/pipeline/graphics_pipeline.hpp"
#include "renderer/device.hpp"
#include "renderer/instance_manager.hpp"
#include "renderer/shader_set.hpp"
#include "renderer/swapchain.hpp"
#include "renderer/window.hpp"

#include <volk/volk.h>

#include <vector>
#include <memory>

namespace mcvk::Renderer {
    class Renderer {
    public:
        Renderer(const Window &window);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        inline const VkCommandBuffer &GetCurrentDrawCommandBuffer() const { return _draw_command_buffers[_current_frame_index]; }

        void WaitDeviceIdle();

        VkCommandBuffer BeginDrawCommandBuffer();
        void EndDrawCommandBuffer();

        void CmdBeginRenderPass(VkCommandBuffer cmdbuf);
        void CmdEndRenderPass(VkCommandBuffer cmdbuf);

        void CmdUpdateViewportAndScissor(VkCommandBuffer cmdbuf);

    private:
        void _RecreateSwapchain();
        void _CreatePipelines();
        void _CreateCommandBuffers();

        std::vector<ShaderInfo> _GetShaders();

        const Window &_window;
        const VkSurfaceKHR &_surface;

        InstanceManager _instance_mgr;
        Device _device;

        std::unique_ptr<Swapchain> _swapchain;
        std::vector<VkCommandBuffer> _draw_command_buffers;

        uint32_t _current_image_index{0};
        int32_t _current_frame_index{0};
        bool _frame_started{false};

        std::unique_ptr<GraphicsPipeline> _default_pipeline;
    };
}
