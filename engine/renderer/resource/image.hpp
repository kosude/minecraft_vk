/*
 * Copyright (c) 2024 Jack Bennett.
 * All Rights Reserved.
 *
 * See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"
#include "resource_mgr/image_load.hpp"

namespace mcvk::Renderer {
    class Image {
    public:
        struct Config {
            VkImageCreateInfo image_info;
            VkImageViewCreateInfo view_info;

            VkMemoryPropertyFlags mem_props;

            static Config Defaults(VkExtent2D extent, VkFormat format);
        };

        Image(const Device &device, const Config &config);
        Image(const Device &device, const Config &config, VkImage image);
        Image(const Device &device, const Config &config, const ResourceMgr::ImageLoadResult &data);
        ~Image();

        inline const VkImage &GetImage() const { return _image; }
        inline const VkImageView &GetImageView() const { return _image_view; }
        inline const VkImageLayout &GetImageLayout() const { return _layout; }
        inline const VkSampler &GetSampler() const { return _sampler; }

    private:
        void _AllocImage();
        void _CreateImageView();
        void _CreateStagingBuffer(VkBuffer &stage, VkDeviceMemory &stage_mem, VkDeviceSize size);
        void _CreateSampler();

        void _TransitionImageLayout(VkImageLayout old, VkImageLayout newl);
        void _Write(const ResourceMgr::ImageLoadResult &data);
        void _TransferStaged(VkBuffer stage, VkDeviceSize size, VkExtent3D extent);

        const Device &_device;

        VkDeviceMemory _memory{VK_NULL_HANDLE};

        VkImage _image{VK_NULL_HANDLE};
        VkImageView _image_view{VK_NULL_HANDLE};
        VkSampler _sampler{VK_NULL_HANDLE};

        Config _config{};
        VkFormat _format;
        VkImageLayout _layout{VK_IMAGE_LAYOUT_UNDEFINED};

        std::vector<uint32_t> _queue_families{};
        VkSharingMode _sharing_mode{VK_SHARING_MODE_EXCLUSIVE};
    };
}
