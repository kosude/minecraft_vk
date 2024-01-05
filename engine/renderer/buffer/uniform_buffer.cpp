/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "uniform_buffer.hpp"

#include "renderer/swapchain.hpp"
#include "utils/log.hpp"

#include <volk/volk.h>

#include <cstring>

namespace mcvk::Renderer {
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
