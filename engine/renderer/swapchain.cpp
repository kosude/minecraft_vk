/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "swapchain.hpp"

#include "utils/log.hpp"

#include <volk/volk.h>

#include <limits>

namespace mcvk::Renderer {
    Swapchain::Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent)
        : _device{device}, _surface{surface}, _window_extent{window_extent} {
        _Init();
    }

    Swapchain::Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent, std::shared_ptr<Swapchain> old)
        : _device{device}, _surface{surface}, _window_extent{window_extent} {
        _Init();

        _old_swapchain = nullptr;
    }

    Swapchain::~Swapchain() {
        if (_swapchain) {
            vkDestroySwapchainKHR(_device.LogicalDevice(), _swapchain, nullptr);
            _swapchain = nullptr;
        }
    }

    void Swapchain::_Init() {
        _CreateSwapchain();
    }

    void Swapchain::_CreateSwapchain() {
        SwapChainSupportDetails support = _device.SwapchainSupportDetails();

        VkSurfaceFormatKHR surface_format = _ChooseSurfaceFormat(support.surface_formats);
        VkPresentModeKHR present_mode = _ChoosePresentMode(support.present_modes);
        VkExtent2D extent = _ChooseExtent(support.capabilities);

        uint32_t image_count = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
            // clamp to maximum image count if applicable
            image_count = support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = _surface;

        info.minImageCount = image_count;
        info.imageFormat = surface_format.format;
        info.imageColorSpace = surface_format.colorSpace;
        info.imageExtent = extent;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = _device.FindQueueFamilyIndices();
        uint32_t family_indices[] = { indices.graphics.value(), indices.present.value() };

        if (indices.graphics != indices.present) {
            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            // images will be shared between two separate queue families so we specify their indices
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = family_indices;
        } else {
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        info.preTransform = support.capabilities.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        info.presentMode = present_mode;
        info.clipped = VK_TRUE;

        info.oldSwapchain = (_old_swapchain) ? _old_swapchain->_swapchain : VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(_device.LogicalDevice(), &info, nullptr, &_swapchain)) {
            Utils::Fatal("Failed to create swap chain");
        }

        // get images created for the swapchain (for colour attachment usage)
        vkGetSwapchainImagesKHR(_device.LogicalDevice(), _swapchain, &image_count, nullptr);
        _swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(_device.LogicalDevice(), _swapchain, &image_count, _swapchain_images.data());

        _swapchain_image_format = surface_format.format;
        _swapchain_extent = extent;

        Utils::Info("Created swap chain with " + std::to_string(_swapchain_images.size()) + " colour buffers");
    }

    VkSurfaceFormatKHR Swapchain::_ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &candidates) {
        const VkFormat ideal_format = VK_FORMAT_B8G8R8A8_SRGB;
        const VkColorSpaceKHR ideal_colourspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        const VkSurfaceFormatKHR fallback = candidates[0];

        for (const auto &candidate : candidates) {
            if (candidate.format == ideal_format && candidate.colorSpace == ideal_colourspace) {
                return candidate;
            }
        }

        return fallback;
    }

    VkPresentModeKHR Swapchain::_ChoosePresentMode(const std::vector<VkPresentModeKHR> &candidates) {
        const VkPresentModeKHR ideal = VK_PRESENT_MODE_MAILBOX_KHR;

        const VkPresentModeKHR fallback = VK_PRESENT_MODE_FIFO_KHR; // guaranteed to be available

        for (const auto &candidate : candidates) {
            if (candidate == ideal) {
                return candidate;
            }
        }

        return fallback;
    }

    VkExtent2D Swapchain::_ChooseExtent(const VkSurfaceCapabilitiesKHR &caps) {
        if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return caps.currentExtent;
        } else {
            VkExtent2D extent = _window_extent;
            extent.width = std::max(
                caps.minImageExtent.width,
                std::min(caps.maxImageExtent.width, extent.width));
            extent.height = std::max(
                caps.minImageExtent.height,
                std::min(caps.maxImageExtent.height, extent.height));

            return extent;
        }
    }
}
