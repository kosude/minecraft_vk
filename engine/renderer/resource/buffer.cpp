/*
 * Copyright (c) 2024 Jack Bennett.
 * All Rights Reserved.
 *
 * See the LICENCE file for more information.
 */

#include "buffer.hpp"

#include "renderer/command_buffer.hpp"
#include "renderer/renderer.hpp"
#include "utils/log.hpp"

#include <volk/volk.h>

#include <cstring>

namespace mcvk::Renderer {
    Buffer::Buffer(const Device &device, VkDeviceSize size)
        : _device{device}, _size{size} {
    }

    Buffer::Buffer(const Device &device, VkDeviceSize size, VkBufferUsageFlags usage)
        : _device{device}, _size{size} {
        // if graphics and transfer queues are in different families, then concurrently share data between those families
        QueueFamilyIndices families = _device.FindQueueFamilyIndices();
        if (families.graphics != families.transfer) {
            _sharing_mode = VK_SHARING_MODE_CONCURRENT;
            _queue_families = {
                families.graphics.value(),
                families.transfer.value() };
        }
        _CreateBuffer(&_stage, &_stage_memory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        _CreateBuffer(&_buffer, &_memory, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    Buffer::~Buffer() {
        if (_stage != VK_NULL_HANDLE) {
            vkDestroyBuffer(_device.GetDevice(), _stage, nullptr);
            vkFreeMemory(_device.GetDevice(), _stage_memory, nullptr);
        }

        if (_buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(_device.GetDevice(), _buffer, nullptr);
            vkFreeMemory(_device.GetDevice(), _memory, nullptr);
        }
    }

    void Buffer::Write(void *data, VkDeviceSize size, VkDeviceSize offset) {
        if (!_mapped) {
            Utils::Error("Attempted to write to buffer without mapping to host memory");
            return;
        }

        VkDeviceSize s;
        if (size == VK_WHOLE_SIZE) {
            s = _size;
        } else {
            s = size;
        }

        Invalidate(size, offset);
        std::memcpy(_mapped, data, s);
        Flush(size, offset);

        _TransferStaged(s, offset);
    }

    void Buffer::Map(VkDeviceSize size, VkDeviceSize offset) {
        if (vkMapMemory(_device.GetDevice(), _stage_memory, offset, size, 0, &_mapped) != VK_SUCCESS) {
            Utils::Fatal("Failed to map host memory to device staging buffer");
        }
    }

    void Buffer::Unmap() {
        if (_mapped) {
            vkUnmapMemory(_device.GetDevice(), _stage_memory);
            _mapped = nullptr;
        }
    }

    void Buffer::Flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = _stage_memory;
        range.offset = offset;
        range.size = size;
        if (vkFlushMappedMemoryRanges(_device.GetDevice(), 1, &range) != VK_SUCCESS) {
            Utils::Fatal("Failed to flush host mapped memory to device staging buffer");
        }
    }

    void Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = _stage_memory;
        range.offset = offset;
        range.size = size;
        if (vkInvalidateMappedMemoryRanges(_device.GetDevice(), 1, &range) != VK_SUCCESS) {
            Utils::Fatal("Failed to invalidate host mapped memory for device staging buffer");
        }
    }

    void Buffer::_CreateBuffer(VkBuffer *buf, VkDeviceMemory *mem, VkBufferUsageFlags usage, VkMemoryPropertyFlags memprops) {
        VkBufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.size = _size;
        create_info.usage = usage;
        create_info.sharingMode = _sharing_mode;
        if (create_info.sharingMode == VK_SHARING_MODE_CONCURRENT) {
            create_info.queueFamilyIndexCount = static_cast<uint32_t>(_queue_families.size());
            create_info.pQueueFamilyIndices = _queue_families.data();
        }
        if (vkCreateBuffer(_device.GetDevice(), &create_info, nullptr, buf) != VK_SUCCESS) {
            Utils::Fatal("Failed to create buffer object");
        }

        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(_device.GetDevice(), *buf, &requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = requirements.size;
        alloc_info.memoryTypeIndex = _device.FindMemoryType(requirements.memoryTypeBits, memprops);
        if (vkAllocateMemory(_device.GetDevice(), &alloc_info, nullptr, mem) != VK_SUCCESS) {
            Utils::Fatal("Failed to allocate device memory for buffer");
        }

        if (vkBindBufferMemory(_device.GetDevice(), *buf, *mem, 0) != VK_SUCCESS) {
            Utils::Fatal("Failed to bind buffer to device memory");
        }
    }

    void Buffer::_TransferStaged(VkDeviceSize size, VkDeviceSize offset) {
        // one-time-submit command buffer for memory transfer operations
        VkCommandBuffer cmdbuf = CommandBuffer::BeginOneTimeSubmit(_device, _device.GetTransferCommandPool());

        VkBufferCopy copy_region{};
        copy_region.size = size;
        copy_region.srcOffset = offset;
        copy_region.dstOffset = offset;

        vkCmdCopyBuffer(cmdbuf, _stage, _buffer, 1, &copy_region);

        CommandBuffer::EndOneTimeSubmit(_device, _device.GetTransferCommandPool(), _device.GetTransferQueue(), cmdbuf);
    }


    VertexBuffer::VertexBuffer(const Device &device, VkDeviceSize size)
        : Buffer{device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT} {
    }


    IndexBuffer::IndexBuffer(const Device &device, VkDeviceSize size, VkIndexType index_type)
        : Buffer{device, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT}, _index_type{index_type} {
    }


    UniformBuffer::UniformBuffer(const Renderer &renderer, VkDeviceSize size)
        : Buffer(renderer.GetDevice(), size), _renderer{renderer} {
        _buffer_handles.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

        uint32_t i = 0;
        for (auto &handle : _buffer_handles) {
            _CreateBuffer(&handle.buf, &handle.mem, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            // persistent mapping - map buffer immediately after creation
            _Map(i++);
        }
    }

    UniformBuffer::~UniformBuffer() {
        uint32_t i = 0;
        for (auto &handle : _buffer_handles) {
            vkDestroyBuffer(_device.GetDevice(), handle.buf, nullptr);
            vkFreeMemory(_device.GetDevice(), handle.mem, nullptr);
        }
    }

    const VkBuffer &UniformBuffer::GetBuffer() const {
        int32_t index = _renderer.GetCurrentFrame();
        return _buffer_handles[index].buf;
    }

    void UniformBuffer::Write(void *data, VkDeviceSize size, VkDeviceSize offset) {
        int32_t index = _renderer.GetCurrentFrame();
        void *mapped = _buffer_handles[index].mapped;

        VkDeviceSize s, o;
        if (size == VK_WHOLE_SIZE) {
            s = _size;
            o = 0;
        } else {
            s = size;
            o = offset;
        }

        std::memcpy(mapped, data, s);
    }

    void UniformBuffer::_Map(uint32_t index, VkDeviceSize size, VkDeviceSize offset) {
        VkDeviceMemory memory = _buffer_handles[index].mem;
        void **mapped = &(_buffer_handles[index].mapped);

        if (vkMapMemory(_device.GetDevice(), memory, offset, size, 0, mapped) != VK_SUCCESS) {
            Utils::Fatal("Failed to map host memory to device buffer (UBO: index " + std::to_string(index) + ")");
        }
    }
}
