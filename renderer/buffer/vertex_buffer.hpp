/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __buffer__vertex_buffer_hpp
#define __buffer__vertex_buffer_hpp

#include "buffer.hpp"

struct DeviceQueueFamilyInfo;

namespace MCVK::Renderer::Buffer {
    /**
     * @brief Vertex buffer class
     */
    class VertexBuffer : public Buffer {
    public:
        /**
         * @brief Create and allocate space for the new vertex buffer object
         */
        VertexBuffer(
            const VkDevice &device,
            const VkPhysicalDevice &physical_device,
            const size_t &size,
            const VkSharingMode &share_mode,
            const std::vector<uint32_t> &queue_family_info
        );
    };
}

#endif
