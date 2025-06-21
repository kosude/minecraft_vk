/*
 * Copyright (c) 2024 Jack Bennett.
 * All Rights Reserved.
 *
 * See the LICENCE file for more information.
 */

#include "image.hpp"

#include "renderer/command_buffer.hpp"
#include "utils/log.hpp"

#include <volk/volk.h>

#include <cstring>

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
        config.image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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
        : _device{device}, _config{config}, _format{config.image_info.format} {
        _AllocImage();
        _CreateImageView();
    }

    Image::Image(const Device &device, const Config &config, VkImage image)
        : _device{device}, _image{image}, _config{config}, _format{config.image_info.format} {
        _config.view_info.image = _image;

        _CreateImageView();
    }

    Image::Image(const Device &device, const Config &config, const ResourceMgr::ImageLoadResult &data)
        : _device{device}, _config{config}, _format{config.image_info.format} {
        QueueFamilyIndices families = _device.FindQueueFamilyIndices();
        if (families.graphics != families.transfer) {
            _sharing_mode = VK_SHARING_MODE_CONCURRENT;
            _queue_families = {
                families.graphics.value(),
                families.transfer.value() };
        }

        _AllocImage();
        _CreateImageView();
        _Write(data);
        _CreateSampler();
    }

    Image::~Image() {
        vkDestroySampler(_device.GetDevice(), _sampler, nullptr);

        vkDestroyImageView(_device.GetDevice(), _image_view, nullptr);

        if (_memory != VK_NULL_HANDLE) {
            // if memory is NULL, then we assume the image was passed directly to the ctor and is therefore handled elsewhere, e.g. by a swapchain
            vkDestroyImage(_device.GetDevice(), _image, nullptr);

            vkFreeMemory(_device.GetDevice(), _memory, nullptr);
        }
    }

    void Image::_AllocImage() {
        if (vkCreateImage(_device.GetDevice(), &_config.image_info, nullptr, &_image) != VK_SUCCESS) {
            Utils::Fatal("Failed to create image object");
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

    void Image::_CreateStagingBuffer(VkBuffer &stage, VkDeviceMemory &stage_mem, VkDeviceSize size) {
        VkBufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.size = size;
        create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        create_info.sharingMode = _sharing_mode;
        if (create_info.sharingMode == VK_SHARING_MODE_CONCURRENT) {
            create_info.queueFamilyIndexCount = static_cast<uint32_t>(_queue_families.size());
            create_info.pQueueFamilyIndices = _queue_families.data();
        }
        if (vkCreateBuffer(_device.GetDevice(), &create_info, nullptr, &stage) != VK_SUCCESS) {
            Utils::Fatal("Failed to create image staging buffer object");
        }

        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(_device.GetDevice(), stage, &requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = requirements.size;
        alloc_info.memoryTypeIndex =
            _device.FindMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (vkAllocateMemory(_device.GetDevice(), &alloc_info, nullptr, &stage_mem) != VK_SUCCESS) {
            Utils::Fatal("Failed to allocate device memory for image staging buffer");
        }

        if (vkBindBufferMemory(_device.GetDevice(), stage, stage_mem, 0) != VK_SUCCESS) {
            Utils::Fatal("Failed to bind image staging buffer to device memory");
        }
    }

    void Image::_CreateSampler() {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_NEAREST; // when texels are scaled up
        info.minFilter = VK_FILTER_NEAREST; // when texels are scaled down
        // addressing mode (i.e. repeat, clamp to border, mirrored repeat, etc)
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.anisotropyEnable = VK_TRUE;
        info.maxAnisotropy = _device.GetProperties().limits.maxSamplerAnisotropy;
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE; // normalised is (0, 0) to (1, 1) - unnormalised is rubbish
        info.compareEnable = VK_FALSE;
        info.compareOp = VK_COMPARE_OP_ALWAYS;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.mipLodBias = 0.0f;
        info.minLod = 0.0f;
        info.maxLod = 0.0f;

        if (vkCreateSampler(_device.GetDevice(), &info, nullptr, &_sampler) != VK_SUCCESS) {
            Utils::Fatal("Failed to create sampler");
        }
    }

    void Image::_TransitionImageLayout(VkImageLayout old, VkImageLayout newl) {
        VkCommandBuffer cmdbuf = CommandBuffer::BeginOneTimeSubmit(_device, _device.GetGraphicsCommandPool());

        VkImageMemoryBarrier image_barrier{};
        image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_barrier.oldLayout = old;
        image_barrier.newLayout = newl;
        image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barrier.image = _image;
        image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_barrier.subresourceRange.baseMipLevel = 0;
        image_barrier.subresourceRange.levelCount = 1;
        image_barrier.subresourceRange.baseArrayLayer = 0;
        image_barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags src_stage, dst_stage;

        // UNDEFINED -> TRANSFER-DST: transfer writes that don't need to wait on anything
        if (old == VK_IMAGE_LAYOUT_UNDEFINED && newl == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            image_barrier.srcAccessMask = 0;
            image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        // TRANSFER-DST -> SHADER-READ: shader reads which should wait on transfer writes
        else if (old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newl == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            // note that we're using the image in the fragment shader
            dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            Utils::Fatal("Encountered unsupported image layout transition");
        }

        vkCmdPipelineBarrier(
            cmdbuf,
            src_stage, dst_stage,
            0,
            0, nullptr,
            0, nullptr,
            1, &image_barrier);

        CommandBuffer::EndOneTimeSubmit(_device, _device.GetGraphicsCommandPool(), _device.GetGraphicsQueue(), cmdbuf);

        _layout = newl;
    }

    void Image::_Write(const ResourceMgr::ImageLoadResult &data) {
        VkDeviceSize image_size = data.width * data.height * 4;

        VkBuffer stage;
        VkDeviceMemory stage_mem;
        _CreateStagingBuffer(stage, stage_mem, image_size);

        void *mapped;
        vkMapMemory(_device.GetDevice(), stage_mem, 0, VK_WHOLE_SIZE, 0, &mapped);
        std::memcpy(mapped, data.bytes, static_cast<size_t>(image_size));
        vkUnmapMemory(_device.GetDevice(), stage_mem);

        _TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // transition to transfer-dst layout
        _TransferStaged(stage, image_size,
            { static_cast<uint32_t>(data.width), static_cast<uint32_t>(data.height), 1 });
        _TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // transition to shader-read layout

        vkDestroyBuffer(_device.GetDevice(), stage, nullptr);
        vkFreeMemory(_device.GetDevice(), stage_mem, nullptr);
    }

    void Image::_TransferStaged(VkBuffer stage, VkDeviceSize size, VkExtent3D extent) {
        VkCommandBuffer cmdbuf = CommandBuffer::BeginOneTimeSubmit(_device, _device.GetTransferCommandPool());

        VkBufferImageCopy copy_region{};
        copy_region.bufferOffset = 0;
        copy_region.bufferRowLength = 0;
        copy_region.bufferImageHeight = 0;
        copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.imageSubresource.mipLevel = 0;
        copy_region.imageSubresource.baseArrayLayer = 0;
        copy_region.imageSubresource.layerCount = 1;
        copy_region.imageOffset = { 0, 0, 0 };
        copy_region.imageExtent = extent;

        vkCmdCopyBufferToImage(cmdbuf, stage, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

        CommandBuffer::EndOneTimeSubmit(_device, _device.GetTransferCommandPool(), _device.GetTransferQueue(), cmdbuf);
    }
}
