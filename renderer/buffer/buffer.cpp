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

    void Buffer::_CreateBuffer(VkBuffer *buffer, VkDeviceMemory *memory, const VkBufferUsageFlags &usage, const VkSharingMode &share_mode,
        const std::vector<uint32_t> &queue_family_info, const VkPhysicalDevice &physical_device, const VkMemoryPropertyFlags &memory_properties)
    {
        VkBufferCreateInfo info = {};

        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = _size;
        info.usage = usage ; // destination of memory transfers (from staging buffer)
        info.sharingMode = share_mode;

        if (share_mode == VK_SHARING_MODE_CONCURRENT) {
            info.queueFamilyIndexCount = queue_family_info.size();
            info.pQueueFamilyIndices = queue_family_info.data();
        }

        if (vkCreateBuffer(_device, &info, nullptr, buffer)) {
            Utils::Error("Failed to create buffer object");
        }

        // get memory space requirements for buffer + suitable types
        VkMemoryRequirements memory_reqs;
        vkGetBufferMemoryRequirements(_device, *buffer, &memory_reqs);

        // allocate memory
        VkMemoryAllocateInfo memory_alloc_info = {};
        memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_alloc_info.allocationSize = memory_reqs.size;
        memory_alloc_info.memoryTypeIndex = _ChooseMemoryType(physical_device, memory_properties, memory_reqs.memoryTypeBits);

        if (vkAllocateMemory(_device, &memory_alloc_info, nullptr, memory)) {
            Utils::Error("Failed to allocate memory for buffer object");
        }

        // bind this memory to the recently created buffer
        vkBindBufferMemory(_device, *buffer, *memory, 0);
    }

    void Buffer::_StageData(const VkCommandPool &command_pool, const VkQueue &queue) {
        VkCommandBufferAllocateInfo cmd_buffer_alloc_info = {};
        cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_buffer_alloc_info.commandPool = command_pool;
        cmd_buffer_alloc_info.commandBufferCount = 1;

        // temporary command buffer
        VkCommandBuffer command_buffer;
        if (vkAllocateCommandBuffers(_device, &cmd_buffer_alloc_info, &command_buffer)) {
            Utils::Error("Failed to allocate temporary memory transfer command buffer");
        }

        // start recording command buffer
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // tell driver that this command buffer is submitted once

        if (vkBeginCommandBuffer(command_buffer, &begin_info)) {
            Utils::Error("Failed to record operations to temporary memory transfer command buffer");
        }

        // transfer buffer contents
        VkBufferCopy copy_region = {};
        copy_region.size = _size;
        vkCmdCopyBuffer(command_buffer, _staging_buffer_handle, _handle, 1, &copy_region);

        // end operations recording
        vkEndCommandBuffer(command_buffer);

        // submit transfer commands
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);

        // wait for transfer to complete on this queue
        vkQueueWaitIdle(queue);

        // free command buffer space
        vkFreeCommandBuffers(_device, command_pool, 1, &command_buffer);
    }

    Buffer::Buffer() {
    }

    Buffer::Buffer(const VkDevice &device, const VkPhysicalDevice &physical_device, const size_t &size, const VkBufferUsageFlags &usage,
        const VkSharingMode &share_mode, const std::vector<uint32_t> &queue_family_info)
        : _device(device), _size(size)
    {
        // create host-visible staging buffer
        _CreateBuffer(&_staging_buffer_handle, &_staging_buffer_memory_handle, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, share_mode, queue_family_info,
            physical_device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // create device local buffer
        _CreateBuffer(&_handle, &_memory_handle, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, share_mode, queue_family_info, physical_device,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    void Buffer::Destroy() {
        vkDestroyBuffer(_device, _handle, nullptr);
        vkFreeMemory(_device, _memory_handle, nullptr);

        vkDestroyBuffer(_device, _staging_buffer_handle, nullptr);
        vkFreeMemory(_device, _staging_buffer_memory_handle, nullptr);
    }

    void Buffer::SetData(void *data, const VkCommandPool &command_pool, const VkQueue &queue) {
        // pointer to buffer within host memory that will be mapped to the buffer's VRAM memory
        void *host_buffer;

        if (vkMapMemory(_device, _staging_buffer_memory_handle, 0, _size, 0, &host_buffer)) {
            Utils::Error("Failed to map CPU memory to buffer object staging buffer");
        }

        // copy data into mapped staging buffer
        memcpy(host_buffer, data, _size);

        // copy staging buffer data into device local memory buffer (_handle)
        _StageData(command_pool, queue);
    }
}
