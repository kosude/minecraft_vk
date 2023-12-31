/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"

#include <memory>

namespace mcvk::Renderer {
    class Swapchain {
    public:
        Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent);
        Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent, std::shared_ptr<Swapchain> old);
        ~Swapchain();

        inline bool CompareSwapFormats(const Swapchain &swapchain) const {
            return
                // TODO create depth images (aka depth buffers)
                // swapchain._depth_image_format_      == _depth_image_format &&
                swapchain._swapchain_image_format   == _swapchain_image_format;
        }

    private:
        void _Init();

        void _CreateSwapchain();

        VkSurfaceFormatKHR _ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &candidates);
        VkPresentModeKHR _ChoosePresentMode(const std::vector<VkPresentModeKHR> &candidates);
        VkExtent2D _ChooseExtent(const VkSurfaceCapabilitiesKHR &caps);

        VkFormat _swapchain_image_format;
        VkExtent2D _swapchain_extent;

        std::vector<VkImage> _swapchain_images;

        const Device &_device;
        const VkSurfaceKHR &_surface;
        VkExtent2D _window_extent;

        VkSwapchainKHR _swapchain;
        std::shared_ptr<Swapchain> _old_swapchain;
    };
}
