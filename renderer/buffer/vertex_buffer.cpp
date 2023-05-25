/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "vertex_buffer.hpp"

namespace MCVK::Renderer::Buffer {
    VertexBuffer::VertexBuffer(const VkDevice &device, const VkPhysicalDevice &physical_device, const size_t &size, const VkSharingMode &share_mode,
        const std::vector<uint32_t> &queue_family_info)
        : Buffer(device, physical_device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, share_mode, queue_family_info)
    {
    }
}
