/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "index_buffer.hpp"

namespace mcvk::Renderer {
    IndexBuffer::IndexBuffer(const Device &device, VkDeviceSize size, VkIndexType index_type)
        : Buffer{device, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT}, _index_type{index_type} {
    }
}
