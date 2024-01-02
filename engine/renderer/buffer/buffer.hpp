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
        Buffer(const Device &device, VkDeviceSize size, VkBufferUsageFlags usage);
        ~Buffer();

        const VkBuffer &GetBuffer() const { return _buffer; }

        void Write(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();

        void Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    private:
        void _CreateBuffer(VkBuffer *buf, VkDeviceMemory *mem, VkBufferUsageFlags usage, VkMemoryPropertyFlags memprops);

        void _TransferStaged(VkDeviceSize size, VkDeviceSize offset);

        void *_mapped{nullptr};

        const Device &_device;

        VkBuffer _buffer;
        VkDeviceMemory _memory;
        VkBuffer _stage;
        VkDeviceMemory _stage_memory;

        VkDeviceSize _size;
    };
}
