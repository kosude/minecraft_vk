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

#include <array>

namespace mcvk::Renderer {
    Renderer::Renderer(const Window &window)
        : _window{window},
        _instance_mgr{window},
        _surface{_instance_mgr.GetSurface()},
        _device{window, _instance_mgr.GetInstance(), _surface} {
        _RecreateSwapchain();
        _CreatePipelines();
        _CreateCommandBuffers();
    }

    Renderer::~Renderer() {
        vkFreeCommandBuffers(
            _device.GetDevice(), _device.GetGraphicsCommandPool(),
            static_cast<uint32_t>(_draw_command_buffers.size()), _draw_command_buffers.data());
    }

    void Renderer::WaitDeviceIdle() {
        vkDeviceWaitIdle(_device.GetDevice());
    }

    VkCommandBuffer Renderer::BeginDrawCommandBuffer() {
        if (_frame_started) {
            Utils::Error("Attempted to begin command buffer while a frame is already in progress");
            return VK_NULL_HANDLE;
        }

        VkResult acquire = _swapchain->AcquireNextImage(&_current_image_index);
        if (acquire == VK_ERROR_OUT_OF_DATE_KHR) {
            _RecreateSwapchain();
            return nullptr;
        }
        if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR) {
            Utils::Fatal("Failed to acquire next swapchain image");
        }

        _frame_started = true;

        VkCommandBuffer cmdbuf = GetCurrentDrawCommandBuffer();
        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(cmdbuf, &info) != VK_SUCCESS) {
            Utils::Fatal("Failed to begin recording to command buffer");
        }

        return cmdbuf;
    }

    void Renderer::EndDrawCommandBuffer() {
        if (!_frame_started) {
            Utils::Error("Attempted to end command buffer when no frame is in progress");
            return;
        }

        VkCommandBuffer cmdbuf = GetCurrentDrawCommandBuffer();

        if (vkEndCommandBuffer(cmdbuf) != VK_SUCCESS) {
            Utils::Fatal("Failed to record command buffer");
        }

        VkResult submit = _swapchain->SubmitCommandBuffers({ cmdbuf }, &_current_image_index);
        if (submit == VK_ERROR_OUT_OF_DATE_KHR || submit == VK_SUBOPTIMAL_KHR) {
            _RecreateSwapchain();
        } else if (submit != VK_SUCCESS) {
            Utils::Fatal("Failed to present swap chain image");
        }

        _frame_started = false;
        _current_frame_index = (_current_frame_index + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::CmdBeginRenderPass(VkCommandBuffer cmdbuf) {
        VkRenderPassBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = _swapchain->GetRenderPass();
        info.framebuffer = _swapchain->GetFramebuffer(_current_image_index);

        info.renderArea.extent = _swapchain->GetExtent();
        info.renderArea.offset = { 0, 0 };

        std::array<VkClearValue, 2> clear{};
        clear[0].color = { 0.4, 0.7, 1.0 };
        clear[1].depthStencil = { 1, 0 };
        info.clearValueCount = static_cast<uint32_t>(clear.size());
        info.pClearValues = clear.data();

        vkCmdBeginRenderPass(cmdbuf, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void Renderer::CmdEndRenderPass(VkCommandBuffer cmdbuf) {
        vkCmdEndRenderPass(cmdbuf);
    }

    void Renderer::CmdUpdateViewportAndScissor(VkCommandBuffer cmdbuf) {
        VkExtent2D extent = _swapchain->GetExtent();

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmdbuf, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = extent;
        scissor.offset = { 0, 0 };
        vkCmdSetScissor(cmdbuf, 0, 1, &scissor);
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
        _draw_command_buffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo info{};

        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandPool = _device.GetGraphicsCommandPool();
        info.commandBufferCount = static_cast<uint32_t>(_draw_command_buffers.size());
        if (vkAllocateCommandBuffers(_device.GetDevice(), &info, _draw_command_buffers.data()) != VK_SUCCESS) {
            Utils::Fatal("Failed to allocate command buffers");
        }
    }

    std::vector<ShaderInfo> Renderer::_GetShaders() {
        return std::vector<ShaderInfo>{
            { ShaderStage::Vertex,      "build/spv/simple_vert.glsl.spv" },
            { ShaderStage::Fragment,    "build/spv/simple_frag.glsl.spv" }
        };
    }
}
