/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "image.hpp"

#include "utils/log.hpp"

#include <volk/volk.h>

namespace mcvk::Renderer {
    Image::Config Image::Config::Defaults(VkExtent2D extent, VkFormat format) {
        Config config{};

        config.image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        config.image_info.imageType = VK_IMAGE_TYPE_2D;
        config.image_info.extent.width = extent.width;
        config.image_info.extent.height = extent.height;
        config.image_info.extent.depth = 1;
        config.image_info.mipLevels = 1;
        config.image_info.arrayLayers = 1;
        config.image_info.format = format;
        config.image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        config.image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        config.image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        config.image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        config.image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        config.view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        config.view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        config.view_info.format = format;
        config.view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        config.view_info.subresourceRange.baseMipLevel = 0;
        config.view_info.subresourceRange.levelCount = 1;
        config.view_info.subresourceRange.baseArrayLayer = 0;
        config.view_info.subresourceRange.layerCount = 1;

        return config;
    }

    Image::Image(const Device &device, const Config &config)
        : _device{device}, _config{config} {
        _AllocImage();
        _CreateImageView();
    }

    Image::Image(const Device &device, const Config &config, VkImage image)
        : _device{device}, _image{image}, _config{config} {
        _config.view_info.image = _image;

        _CreateImageView();
    }

    Image::~Image() {
        vkDestroyImageView(_device.GetDevice(), _image_view, nullptr);

        if (_memory != VK_NULL_HANDLE) {
            // if memory is NULL, then we assume the image was passed directly to the ctor and is therefore handled elsewhere, e.g. by a swapchain
            vkDestroyImage(_device.GetDevice(), _image, nullptr);

            vkFreeMemory(_device.GetDevice(), _memory, nullptr);
        }
    }

    void Image::_AllocImage() {
        if (vkCreateImage(_device.GetDevice(), &_config.image_info, nullptr, &_image) != VK_SUCCESS) {
            Utils::Fatal("Failed to create image");
        }
        _config.view_info.image = _image;

        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(_device.GetDevice(), _image, &requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = requirements.size;
        alloc_info.memoryTypeIndex = _device.FindMemoryType(requirements.memoryTypeBits, _config.mem_props);

        if (vkAllocateMemory(_device.GetDevice(), &alloc_info, nullptr, &_memory) != VK_SUCCESS) {
            Utils::Fatal("Failed to allocate device memory for image");
        }

        if (vkBindImageMemory(_device.GetDevice(), _image, _memory, 0) != VK_SUCCESS) {
            Utils::Fatal("Failed to bind image to device memory");
        }
    }

    void Image::_CreateImageView() {
        if (vkCreateImageView(_device.GetDevice(), &_config.view_info, nullptr, &_image_view) != VK_SUCCESS) {
            Utils::Fatal("Failed to create image view");
        }
    }
}
