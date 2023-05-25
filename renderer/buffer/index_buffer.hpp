/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __buffer__index_buffer_hpp
#define __buffer__index_buffer_hpp

#include <volk/volk.h>

#include "buffer.hpp"

struct DeviceQueueFamilyInfo;

namespace MCVK::Renderer::Buffer {
    /**
     * @brief Index buffer class
     */
    class IndexBuffer : public Buffer {
    public:
        /**
         * @brief Create and allocate space for the new index buffer object
         */
        IndexBuffer(
            const VkDevice &device,
            const VkPhysicalDevice &physical_device,
            const size_t &size,
            const VkSharingMode &share_mode,
            const std::vector<uint32_t> &queue_family_info
        );
    };
}

#endif
