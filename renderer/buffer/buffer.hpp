/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __buffer__buffer_hpp
#define __buffer__buffer_hpp

#include <vector>
#include <volk/volk.h>

struct DeviceQueueFamilyInfo;

namespace MCVK::Renderer::Buffer {
    /**
     * @brief Basic buffer class
     */
    class Buffer {
    private:
        uint32_t _ChooseMemoryType(
            const VkPhysicalDevice &physical_device,
            const VkMemoryPropertyFlags &prop_flags,
            const uint32_t &type_filter
        );

        void _CreateBuffer(
            VkBuffer *buffer,
            VkDeviceMemory *memory,
            const VkBufferUsageFlags &usage,
            const VkSharingMode &share_mode,
            const std::vector<uint32_t> &queue_family_info,
            const VkPhysicalDevice &physical_device,
            const VkMemoryPropertyFlags &memory_properties
        );

        // submit transfer of data from staging buffer to device local memory
        void _StageData(
            const VkCommandPool &command_pool,
            const VkQueue &queue
        );

    protected:
        VkBuffer _handle;
        VkDeviceMemory _memory_handle;

        VkDevice _device;

        size_t _size;

        VkBuffer _staging_buffer_handle;
        VkDeviceMemory _staging_buffer_memory_handle;

    public:
        /**
         * @brief Create and allocate space for the buffer object
         */
        Buffer(
            const VkDevice &device,
            const VkPhysicalDevice &physical_device,
            const size_t &size,
            const VkBufferUsageFlags &usage,
            const VkSharingMode &share_mode,
            const std::vector<uint32_t> &queue_family_info
        );

        /**
         * @brief Destroy the buffer object
         */
        void Destroy();

        /**
         * @brief Fill the buffer's memory with the given data, memory transfer operations done on given command pool + queue
         */
        void SetData(
            void *data,
            const VkCommandPool &command_pool,
            const VkQueue &queue
        );

        inline VkBuffer GetObjectHandle() const { return _handle; }
        inline VkDeviceMemory GetMemoryHandle() const { return _memory_handle; }
    };
}

#endif
