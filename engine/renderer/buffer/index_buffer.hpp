/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/buffer/buffer.hpp"

namespace mcvk::Renderer {
    class IndexBuffer : public Buffer {
    public:
        IndexBuffer(const Device &device, VkDeviceSize size, VkIndexType index_type);

        inline const VkIndexType &GetIndexType() const { return _index_type; }

    private:
        VkIndexType _index_type;
    };
}
