/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/buffer/image.hpp"
#include "renderer/device.hpp"

#include <memory>
#include <vector>

namespace mcvk::Renderer {
    class Swapchain {
    public:
        Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent);
        Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent, std::shared_ptr<Swapchain> old);
        ~Swapchain();

        inline VkRenderPass GetRenderPass() const { return _render_pass; }

        bool CompareSwapFormats(const Swapchain &swapchain) const;

    private:
        void _Init();

        void _CreateSwapchain();
        void _ManageSwapchainImages();
        void _CreateDepthImages();
        void _CreateRenderPass();
        void _CreateFramebuffers();

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
    };
}
