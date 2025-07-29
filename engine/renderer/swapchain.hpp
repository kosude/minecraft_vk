/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/resource/image.hpp"
#include "renderer/device.hpp"

#include <memory>
#include <vector>

namespace mcvk::Renderer {
    class Swapchain {
    public:
        Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent);
        Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent, std::shared_ptr<Swapchain> old);
        ~Swapchain();

        Swapchain(const Swapchain &) = delete;
        Swapchain &operator=(const Swapchain &) = delete;

        inline const VkRenderPass &GetRenderPass() const { return _render_pass; }
        inline const VkFramebuffer &GetFramebuffer(uint32_t index) const { return _swapchain_framebuffers[index]; }
        inline const VkExtent2D &GetExtent() const { return _swapchain_extent; }

        bool CompareSwapFormats(const Swapchain &swapchain) const;

        VkResult AcquireNextImage(uint32_t *const image_index);
        VkResult SubmitCommandBuffers(const std::vector<VkCommandBuffer> &cmdbufs, uint32_t *const image_index);

    private:
        void _Init();

        void _CreateSwapchain();
        void _ManageSwapchainImages();
        void _CreateDepthImages();
        void _CreateRenderPass();
        void _CreateFramebuffers();
        void _CreateSynchronisationPrims();

        VkSurfaceFormatKHR _ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &candidates);
        VkPresentModeKHR _ChoosePresentMode(const std::vector<VkPresentModeKHR> &candidates);
        VkExtent2D _ChooseExtent(const VkSurfaceCapabilitiesKHR &caps);
        VkFormat _FindDepthImageFormat();

        VkExtent2D _swapchain_extent;
        VkFormat _swapchain_image_format;
        VkFormat _depth_image_format;

        VkRenderPass _render_pass;
        std::vector<VkFramebuffer> _swapchain_framebuffers;

        std::vector<std::unique_ptr<Image>> _swapchain_images;
        std::vector<std::unique_ptr<Image>> _depth_images;

        const Device &_device;
        const VkSurfaceKHR &_surface;
        VkExtent2D _window_extent;

        VkSwapchainKHR _swapchain;
        std::shared_ptr<Swapchain> _old_swapchain;

        VkSemaphore _image_available_sem;
        std::vector<VkSemaphore> _draw_complete_sems;
        VkFence _frame_fence;
    };
}
