/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"

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
        ~Image();

        inline const VkImage &GetImage() const { return _image; }
        inline const VkImageView &GetImageView() const { return _image_view; }

    private:
        void _AllocImage();
        void _CreateImageView();

        const Device &_device;

        VkDeviceMemory _memory{VK_NULL_HANDLE};

        VkImage _image;
        VkImageView _image_view;

        Config _config;
    };
}
