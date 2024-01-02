/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "vertex_buffer.hpp"

namespace mcvk::Renderer {
    VertexBuffer::VertexBuffer(const Device &device, VkDeviceSize size)
        : Buffer{device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT} {
    }
}
