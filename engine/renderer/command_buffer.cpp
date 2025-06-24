/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "command_buffer.hpp"

#include "renderer/renderer.hpp"
#include "utils/log.hpp"

#include <array>

namespace mcvk::Renderer {
    CommandBuffer::CommandBuffer(const Device &device, const std::unique_ptr<Swapchain> &swapchain)
        : _device{device}, _swapchain{swapchain} {
    }

    CommandBuffer::~CommandBuffer() {
        vkFreeCommandBuffers(_device.GetDevice(), _device.GetGraphicsCommandPool(), 1, &_cb);
    }

    VkCommandBuffer CommandBuffer::BeginOneTimeSubmit(const Device &device, VkCommandPool command_pool) {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cb;
        if (vkAllocateCommandBuffers(device.GetDevice(), &alloc_info, &cb) != VK_SUCCESS) {
            Utils::Fatal("Failed to allocate memory transfer command buffer");
        }

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(cb, &begin_info) != VK_SUCCESS) {
            Utils::Fatal("Failed to begin recording to one-time-submit command buffer");
        }
        return cb;
    }

    void CommandBuffer::EndOneTimeSubmit(const Device &device, VkCommandPool command_pool, VkQueue queue, VkCommandBuffer cmdbuf) {
        if (vkEndCommandBuffer(cmdbuf) != VK_SUCCESS) {
            Utils::Fatal("Failed to record one-time-submit command buffer");
        }

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmdbuf;

        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device.GetDevice(), command_pool, 1, &cmdbuf);
    }

    void CommandBuffer::End() {
        if (!_frame_started) {
            Utils::Error("Attempted to end command buffer when no frame is in progress");
            return;
        }

        if (vkEndCommandBuffer(_cb) != VK_SUCCESS) {
            Utils::Fatal("Failed to record command buffer");
        }

        VkResult submit = _swapchain->SubmitCommandBuffers({ _cb }, &_current_image_index);
        if (submit == VK_ERROR_OUT_OF_DATE_KHR || submit == VK_SUBOPTIMAL_KHR || _renderer->_window.WasResized()) {
            _renderer->_RecreateSwapchain();
        } else if (submit != VK_SUCCESS) {
            Utils::Fatal("Failed to present swap chain image");
        }

        _frame_started = false;
    }

    void CommandBuffer::BeginRenderPass(VkClearColorValue clear_col) {
        VkRenderPassBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = _swapchain->GetRenderPass();
        info.framebuffer = _swapchain->GetFramebuffer(_current_image_index);

        info.renderArea.extent = _swapchain->GetExtent();
        info.renderArea.offset = { 0, 0 };

        std::array<VkClearValue, 2> clear{};
        clear[0].color = clear_col;
        clear[1].depthStencil = { 1, 0 };
        info.clearValueCount = static_cast<uint32_t>(clear.size());
        info.pClearValues = clear.data();

        vkCmdBeginRenderPass(_cb, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBuffer::EndRenderPass() {
        vkCmdEndRenderPass(_cb);
    }

    void CommandBuffer::BindPipeline(const GraphicsPipeline &pipeline) {
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());
    }

    void CommandBuffer::BindVertexBuffer(const VertexBuffer &buffer) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(_cb, 0, 1, &buffer.GetBuffer(), &offset);
    }

    void CommandBuffer::BindIndexBuffer(const IndexBuffer &buffer) {
        vkCmdBindIndexBuffer(_cb, buffer.GetBuffer(), 0, buffer.GetIndexType());
    }

    void CommandBuffer::BindDescriptorSets(const GraphicsPipeline &pipeline, const std::vector<VkDescriptorSet> &sets,
        const std::vector<uint32_t> &dynoffsets) {
        VkPipelineLayout layout = pipeline.GetPipelineLayout();

        vkCmdBindDescriptorSets(
            _cb,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            layout,
            0,
            static_cast<uint32_t>(sets.size()),
            sets.data(),
            static_cast<uint32_t>(dynoffsets.size()),
            dynoffsets.data());
    }

    void CommandBuffer::Draw(uint32_t vertex_count) {
        vkCmdDraw(_cb, vertex_count, 1, 0, 0);
    }

    void CommandBuffer::DrawIndexed(uint32_t index_count){
        vkCmdDrawIndexed(_cb, index_count, 1, 0, 0, 0);
    }

    void CommandBuffer::UpdateViewportAndScissor() {
        VkExtent2D extent = _swapchain->GetExtent();

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_cb, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = extent;
        scissor.offset = { 0, 0 };
        vkCmdSetScissor(_cb, 0, 1, &scissor);
    }

    void CommandBuffer::_Initialise(Renderer *const renderer) {
        _renderer = renderer;

        VkCommandBufferAllocateInfo info{};

        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandPool = _device.GetGraphicsCommandPool();
        info.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(_device.GetDevice(), &info, &_cb) != VK_SUCCESS) {
            Utils::Fatal("Failed to allocate command buffers");
        }
    }

    void CommandBuffer::_Begin() {
        if (_frame_started) {
            Utils::Error("Attempted to begin command buffer while a frame is already in progress");
            return;
        }

        VkResult acquire = _swapchain->AcquireNextImage(&_current_image_index);
        if (acquire == VK_ERROR_OUT_OF_DATE_KHR) {
            _renderer->_RecreateSwapchain();
            return;
        }
        if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR) {
            Utils::Fatal("Failed to acquire next swapchain image");
        }

        _frame_started = true;

        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(_cb, &info) != VK_SUCCESS) {
            Utils::Fatal("Failed to begin recording to command buffer");
        }
    }
}
