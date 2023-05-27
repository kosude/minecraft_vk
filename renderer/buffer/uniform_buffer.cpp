/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include "uniform_buffer.hpp"

#include <string.h>

namespace MCVK::Renderer::Buffer {
    UniformBuffer::UniformBuffer(const VkDevice &device, const VkPhysicalDevice &physical_device, const size_t &size, const uint16_t &buffer_count)
        : _buffer_count(buffer_count)
    {
        _device = device;
        _size = size;

        _handles.resize(buffer_count);
        _memory_handles.resize(buffer_count);
        _buffers_mapped.resize(buffer_count);

        for (uint16_t i = 0; i < buffer_count; i++) {
            // create buffer i
            _CreateBuffer(&_handles[i], &_memory_handles[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, std::vector<uint32_t>(),
                physical_device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            // map memory for persistent mapping (big performance increase as uniform buffers are written to quite often)
            // pointers to mapped host blocks are in _buffers_mapped.
            vkMapMemory(device, _memory_handles[i], 0, _size, 0, &_buffers_mapped[i]);
        }
    }

    void UniformBuffer::Destroy() {
        // note that inherited protected members _handle and _memory_handle are not used so don't need to be destroyed.
        // same with stage buffer handles

        for (uint16_t i = 0; i < _buffer_count; i++) {
            vkDestroyBuffer(_device, _handles[i], nullptr);
            vkFreeMemory(_device, _memory_handles[i], nullptr);
        }
    }

    void UniformBuffer::SetData(void *data, uint16_t buffer_index) {
        memcpy(_buffers_mapped[buffer_index], data, _size);
    }
}
