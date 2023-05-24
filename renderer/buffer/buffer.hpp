/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __buffer__vertex_buffer_hpp
#define __buffer__vertex_buffer_hpp

#include <volk/volk.h>

namespace MCVK::Renderer::Buffer {
    /**
     * @brief Basic buffer class
     */
    class Buffer {
    protected:
        VkBuffer _handle;
        VkDeviceMemory _memory_handle;

        VkDevice _device;

        size_t _size;

        uint32_t _ChooseMemoryType(
            const VkPhysicalDevice &physical_device,
            const VkMemoryPropertyFlags &prop_flags,
            const uint32_t &type_filter
        );

    public:
        /**
         * @brief Create and allocate space for the buffer object
         */
        Buffer(
            const VkDevice &device,
            const VkPhysicalDevice &physical_device,
            const size_t &size,
            const VkBufferUsageFlags &usage,
            const VkMemoryPropertyFlags &memory_properties,
            const VkSharingMode &share_mode = VK_SHARING_MODE_EXCLUSIVE
        );

        /**
         * @brief Destroy the buffer object
         */
        void Destroy();

        /**
         * @brief Fill the buffer's memory with the given data
         */
        void SetData(
            void *data
        );

        inline VkBuffer GetObjectHandle() const { return _handle; }
        inline VkDeviceMemory GetMemoryHandle() const { return _memory_handle; }
    };
}

#endif
