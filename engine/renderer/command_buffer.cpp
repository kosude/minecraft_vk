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
        vkFreeCommandBuffers(_device.GetDevice(), _device.GetGraphicsCommandPool(), static_cast<uint32_t>(_cmdbufs.size()), _cmdbufs.data());
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
        _current_frame_index = (_current_frame_index + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
    }

    void CommandBuffer::BeginRenderPass() {
        VkRenderPassBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = _swapchain->GetRenderPass();
        info.framebuffer = _swapchain->GetFramebuffer(_current_image_index);

        info.renderArea.extent = _swapchain->GetExtent();
        info.renderArea.offset = { 0, 0 };

        std::array<VkClearValue, 2> clear{};
        clear[0].color = { 0.3, 0.5, 0.8 };
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

        _cmdbufs.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo info{};

        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandPool = _device.GetGraphicsCommandPool();
        info.commandBufferCount = static_cast<uint32_t>(_cmdbufs.size());
        if (vkAllocateCommandBuffers(_device.GetDevice(), &info, _cmdbufs.data()) != VK_SUCCESS) {
            Utils::Fatal("Failed to allocate command buffers");
        }
    }

    void CommandBuffer::_Begin() {
        if (_frame_started) {
            Utils::Error("Attempted to begin command buffer while a frame is already in progress");
            return;
        }

        _cb = _cmdbufs[_current_frame_index];

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