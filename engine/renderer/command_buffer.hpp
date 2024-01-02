/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/buffer/index_buffer.hpp"
#include "renderer/buffer/vertex_buffer.hpp"
#include "renderer/pipeline/graphics_pipeline.hpp"
#include "renderer/device.hpp"
#include "renderer/swapchain.hpp"

#include <volk/volk.h>

#include <utility>
#include <vector>

namespace mcvk::Renderer {
    class Renderer;

    class CommandBuffer {
    public:
        CommandBuffer(const Device &device, const std::unique_ptr<Swapchain> &swapchain);
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer &) = delete;
        CommandBuffer &operator=(const CommandBuffer &) = delete;

        static VkCommandBuffer BeginOneTimeSubmit(const Device &device, VkCommandPool command_pool);
        static void EndOneTimeSubmit(const Device &device, VkCommandPool command_pool, VkQueue queue, VkCommandBuffer cmdbuf);

        void End();

        void BeginRenderPass(VkClearColorValue clear_col = {0});
        void EndRenderPass();

        void BindPipeline(const GraphicsPipeline &pipeline);
        void BindVertexBuffer(const VertexBuffer &buffer);
        void BindIndexBuffer(const IndexBuffer &buffer);

        void Draw(uint32_t vertex_count);
        void DrawIndexed(uint32_t index_count);

        void UpdateViewportAndScissor();

    private:
        friend class Renderer;

        void _Initialise(Renderer *const renderer);

        void _Begin();

        const Device &_device;
        const std::unique_ptr<Swapchain> &_swapchain;
        Renderer *_renderer{nullptr};

        std::vector<VkCommandBuffer> _cmdbufs;
        VkCommandBuffer _cb;

        uint32_t _current_image_index{0};
        int32_t _current_frame_index{0};
        bool _frame_started{false};
    };
}
