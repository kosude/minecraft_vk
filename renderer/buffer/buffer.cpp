/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "buffer.hpp"

#include "utils/log.hpp"

#include <string.h>

namespace MCVK::Renderer::Buffer {
    uint32_t Buffer::_ChooseMemoryType(const VkPhysicalDevice &physical_device, const VkMemoryPropertyFlags &prop_flags,
        const uint32_t &type_filter)
    {
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            // iterate through suitable memory types, return first that fits the filter
            if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & prop_flags)) {
                return i;
            }
        }

        // no types found
        Utils::Error("Could not find a suitable memory type for generic buffer object");
        return 0;
    }

    Buffer::Buffer(const VkDevice &device, const VkPhysicalDevice &physical_device, const size_t &size, const VkBufferUsageFlags &usage,
        const VkMemoryPropertyFlags &memory_properties, const VkSharingMode &share_mode) : _device(device), _size(size)
    {
        VkBufferCreateInfo info = {};

        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = _size;
        info.usage = usage;
        info.sharingMode = share_mode;

        if (vkCreateBuffer(_device, &info, nullptr, &_handle)) {
            Utils::Error("Failed to create generic buffer object");
        }

        // get memory space requirements for buffer + suitable types
        VkMemoryRequirements memory_reqs;
        vkGetBufferMemoryRequirements(_device, _handle, &memory_reqs);

        // allocate memory
        VkMemoryAllocateInfo memory_alloc_info = {};
        memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_alloc_info.allocationSize = memory_reqs.size;
        memory_alloc_info.memoryTypeIndex = _ChooseMemoryType(physical_device, memory_properties, memory_reqs.memoryTypeBits);

        if (vkAllocateMemory(_device, &memory_alloc_info, nullptr, &_memory_handle)) {
            Utils::Error("Failed to allocate memory for generic buffer object");
        }

        // bind this memory to the recently created buffer
        if (vkBindBufferMemory(_device, _handle, _memory_handle, 0)) {
            Utils::Error("Failed to bind allocated memory to generic buffer object");
        }
    }

    void Buffer::Destroy() {
        vkDestroyBuffer(_device, _handle, nullptr);
        vkFreeMemory(_device, _memory_handle, nullptr);
    }

    void Buffer::SetData(void *data) {
        // pointer to buffer within host memory that will be mapped to the buffer's VRAM memory
        void *host_buffer;

        if (vkMapMemory(_device, _memory_handle, 0, _size, 0, &host_buffer)) {
            Utils::Error("Failed to map CPU memory to generic buffer object");
        }

        memcpy(host_buffer, data, _size);

        // unmap the host memory pointer
        vkUnmapMemory(_device, _memory_handle);
    }
}
