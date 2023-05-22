/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <limits>
#include <algorithm>

#include "renderer/device_manager.hpp"
#include "utils/log.hpp"

#include "swap_chain.hpp"

namespace MCVK::Renderer {
    VkSurfaceFormatKHR Swapchain::_GetOptimalSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &choices) {
        // use sRGB if available
        for (const VkSurfaceFormatKHR &c : choices) {
            if (c.format == VK_FORMAT_B8G8R8A8_SRGB && c.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return c;
            }
        }

        // TODO: other available formats could be ranked in future
        return choices[0];
    }

    VkPresentModeKHR Swapchain::_GetOptimalSwapPresentMode(const std::vector<VkPresentModeKHR> &choices) {
        for (const VkPresentModeKHR &c : choices) {
            // triple buffering should perform best in theory so we use it by default if available
            if (c == VK_PRESENT_MODE_MAILBOX_KHR) {
                return c;
            }
        }

        // most similar to v-sync: this is guaranteed to be available
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::_GetOptimalSwapExtent(const VkSurfaceCapabilitiesKHR &caps, const uint32_t &window_width, const uint32_t &window_height) {
        if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            // int max is used by some window mgrs to indicate the extent will be correct already (?)
            return caps.currentExtent;
        } else {
            VkExtent2D actual = {
                window_width,
                window_height
            };

            // clamp actual extent to possible extent limits
            actual.width = std::clamp(actual.width, caps.minImageExtent.width, caps.maxImageExtent.width);
            actual.height = std::clamp(actual.height, caps.minImageExtent.height, caps.maxImageExtent.height);

            return actual;
        }
    }

    Swapchain::Swapchain(const VkDevice &logical_device, const VkPhysicalDevice &physical_device, const VkSurfaceKHR &surface,
        const uint32_t &window_width, const uint32_t &window_height)
    {
        SwapchainSupportInfo support_info = GetSupportInfo(physical_device, surface);

        // swapchain settings
        VkSurfaceFormatKHR surface_format = _GetOptimalSwapSurfaceFormat(support_info.formats);
        VkPresentModeKHR present_mode = _GetOptimalSwapPresentMode(support_info.present_modes);
        VkExtent2D extent = _GetOptimalSwapExtent(support_info.caps, window_width, window_height);

        // requesting at least 1 more image than minimum is recommended to avoid waiting for new images to render to from the driver
        uint32_t image_count = support_info.caps.minImageCount + 1;

        // max of 0 means there is no limit -- clamp to max
        if (support_info.caps.maxImageCount != 0 && image_count > support_info.caps.maxImageCount) {
            image_count = support_info.caps.maxImageCount;
        }

        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = surface;

        info.minImageCount = image_count;
        info.imageFormat = surface_format.format;
        info.imageColorSpace = surface_format.colorSpace;
        info.imageExtent = extent;
        info.presentMode = present_mode;
        info.imageArrayLayers = 1; // 1 except in stereoscopic (3D) programs
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // specify swapchain behaviour when handling multiple queue families.
        DeviceQueueFamilyInfo queue_family_info = DeviceManager::GetDeviceQueueFamilyInfo(physical_device, surface);
        uint32_t queue_family_indices_arr[] = { queue_family_info.graphics_family_index.value(), queue_family_info.present_family_index.value() };

        if (queue_family_info.graphics_family_index != queue_family_info.present_family_index) {
            // separate graphics and present families: sharing image ownership between queue families.
            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = queue_family_indices_arr;
        } else {
            // same queue family for graphics and presentation - concurrent mode requires at least 2 families to be specified.
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        // to apply no transformation, we specify current transformation
        info.preTransform = support_info.caps.currentTransform;

        // blending with other windows in the DE; not needed for this program.
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        // pixels obscured by other windows are disregarded for performance
        info.clipped = VK_TRUE;

        info.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(logical_device, &info, nullptr, &_handle)) {
            Utils::Error("Failed to create swapchain");
        }

        // get + store handles
        vkGetSwapchainImagesKHR(logical_device, _handle, &image_count, nullptr);
        _images = std::vector<VkImage>(image_count);
        vkGetSwapchainImagesKHR(logical_device, _handle, &image_count, _images.data());

        // store other data
        _extent = extent;
        _image_format = surface_format.format;

        _logical_device = logical_device;
    }

    std::vector<VkImageView> Swapchain::CreateImageViews() {
        std::vector<VkImageView> views(_images.size());

        for (uint32_t i = 0; i < _images.size(); i++) {
            VkImageViewCreateInfo info = {};

            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = _images[i];

            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = _image_format;

            info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            // color targets, no mipmap levels or multiple layers
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_logical_device, &info, nullptr, &views[i])) {
                Utils::Error("Failed to create image view");
            }
        }

        return views;
    }

    SwapchainSupportInfo Swapchain::GetSupportInfo(const VkPhysicalDevice &physical_device, const VkSurfaceKHR &surface) {
        SwapchainSupportInfo info;

        // get surface caps
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &info.caps);

        // get supported formats
        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
        if (format_count > 0) {
            info.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, info.formats.data());
        }

        // get supported presentation modes
        uint32_t presentation_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentation_mode_count, nullptr);
        if (presentation_mode_count > 0) {
            info.present_modes.resize(presentation_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentation_mode_count, info.present_modes.data());
        }

        return info;
    }
}
