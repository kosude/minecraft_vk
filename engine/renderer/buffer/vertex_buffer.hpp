/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/buffer/buffer.hpp"

namespace mcvk::Renderer {
    class VertexBuffer : public Buffer {
    public:
        VertexBuffer(const Device &device, VkDeviceSize size);
    };
}
