/*
 * Copyright (c) 2024 Jack Bennett.
 * All Rights Reserved.
 *
 * See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"

namespace mcvk::Renderer {
    class Renderer;

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

        void *_mapped{nullptr};
        VkBuffer _buffer{VK_NULL_HANDLE};
        VkDeviceMemory _memory{VK_NULL_HANDLE};
        VkBuffer _stage{VK_NULL_HANDLE};
        VkDeviceMemory _stage_memory{VK_NULL_HANDLE};

    private:
        void _TransferStaged(VkDeviceSize size, VkDeviceSize offset);

        std::vector<uint32_t> _queue_families{};
        VkSharingMode _sharing_mode{VK_SHARING_MODE_EXCLUSIVE};
    };

    class VertexBuffer : public Buffer {
    public:
        VertexBuffer(const Device &device, VkDeviceSize size);
    };

    class IndexBuffer : public Buffer {
    public:
        IndexBuffer(const Device &device, VkDeviceSize size, VkIndexType index_type);

        inline const VkIndexType &GetIndexType() const { return _index_type; }

    private:
        VkIndexType _index_type;
    };

    class UniformBuffer : public Buffer {
    public:
        UniformBuffer(const Renderer &renderer, VkDeviceSize size);

        void Write(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) override;
        static VkDeviceSize AlignOffset(const Device &device, VkDeviceSize size);

    private:
        const Renderer &_renderer;

        void _Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    };
}
