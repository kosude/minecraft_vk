/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __renderer__swap_chain_hpp
#define __renderer__swap_chain_hpp

#include <volk/volk.h>

#include <vector>

namespace VKGame::Renderer {
    /**
     * @brief Struct containing capability and support info for a swapchain
     */
    struct SwapchainSupportInfo {
    public:
        VkSurfaceCapabilitiesKHR caps;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    /**
     * @brief Swapchain functions and class
     */
    class Swapchain {
    private:
        VkSwapchainKHR _handle;
        std::vector<VkImage> _images;
        VkFormat _image_format;
        VkExtent2D _extent;

        VkDevice _logical_device;

        /**
         * @brief Choose + return best swap surface format from the given choices of formats
         */
        VkSurfaceFormatKHR _GetOptimalSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &choices
        );
        /**
         * @brief Choose + return best swap present mode from the given choices of modes
         */
        VkPresentModeKHR _GetOptimalSwapPresentMode(
            const std::vector<VkPresentModeKHR> &choices
        );
        /**
         * @brief Get best swap extent (swap chain image resolutions) for the window surface
         */
        VkExtent2D _GetOptimalSwapExtent(
            const VkSurfaceCapabilitiesKHR &caps,
            const uint32_t &window_width,
            const uint32_t &window_height
        );

    public:
        /**
         * @brief Construct a new Vulkan swapchain to render with given device to given surface
         */
        Swapchain(
            const VkDevice &logical_device,
            const VkPhysicalDevice &physical_device,
            const VkSurfaceKHR &surface,
            const uint32_t &window_width,
            const uint32_t &window_height
        );

        /**
         * @brief Create and return image views for images in the swapchain
         */
        std::vector<VkImageView> CreateImageViews();

        /**
         * @brief Get vk swapchain object
         */
        inline VkSwapchainKHR GetHandle() const { return _handle; }
        /**
         * @brief Get swapchain image handles
         */
        inline std::vector<VkImage> GetImages() const { return _images; }
        /**
         * @brief Get swap image format
         */
        inline VkFormat GetImageFormat() const { return _image_format; }
        /**
         * @brief Get swap extent
         */
        inline VkExtent2D GetExtent() const { return _extent; }

        /**
         * @brief Get capabilities and support info for swapchains interfacing given device and surface
         */
        static SwapchainSupportInfo GetSupportInfo(
            const VkPhysicalDevice &physical_device,
            const VkSurfaceKHR &surface
        );
    };
}

#endif
