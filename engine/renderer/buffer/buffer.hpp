/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"

namespace mcvk::Renderer {
    class Buffer {
    public:
        Buffer(const Device &device, VkDeviceSize size);
        Buffer(const Device &device, VkDeviceSize size, VkBufferUsageFlags usage);
        virtual ~Buffer();

        inline const VkBuffer &GetBuffer() const { return _buffer; }
        inline const VkDeviceSize &GetSize() const { return _size; }

        virtual void Write(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        virtual void Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        virtual void Unmap();

        virtual void Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        virtual void Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    protected:
        virtual void _CreateBuffer(VkBuffer *buf, VkDeviceMemory *mem, VkBufferUsageFlags usage, VkMemoryPropertyFlags memprops);

        const Device &_device;

        VkDeviceSize _size;

    private:
        void _TransferStaged(VkDeviceSize size, VkDeviceSize offset);

        void *_mapped{nullptr};

        VkBuffer _buffer{VK_NULL_HANDLE};
        VkDeviceMemory _memory{VK_NULL_HANDLE};
        VkBuffer _stage{VK_NULL_HANDLE};
        VkDeviceMemory _stage_memory{VK_NULL_HANDLE};

        std::vector<uint32_t> _queue_families{};
        VkSharingMode _sharing_mode{VK_SHARING_MODE_EXCLUSIVE};
    };
}
