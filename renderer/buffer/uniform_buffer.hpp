/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __buffer__uniform_buffer_hpp
#define __buffer__uniform_buffer_hpp

#include "buffer.hpp"

namespace MCVK::Renderer::Buffer {
    /**
     * @brief Uniform buffer object (UBO)
     */
    class UniformBuffer : private Buffer {
    private:
        std::vector<VkBuffer> _handles;
        std::vector<VkDeviceMemory> _memory_handles;
        std::vector<void *> _buffers_mapped;

        uint16_t _buffer_count;

    public:
        /**
         * @brief Create and allocate space for the new uniform buffer object
         */
        UniformBuffer(
            const VkDevice &device,
            const VkPhysicalDevice &physical_device,
            const size_t &size,
            const uint16_t &buffer_count
        );

        /**
         * @brief Destroy the uniform buffer object
         */
        void Destroy() override;

        /**
         * @brief Fill the uniform buffer's memory with the given data
         */
        void SetData(
            void *data,
            uint16_t buffer_index
        );

        inline const std::vector<VkBuffer> &GetObjectHandles() const { return _handles; }
        inline const std::vector<VkDeviceMemory> &GetMemoryHandles() const { return _memory_handles; }
        inline const std::vector<void *> &GetMappedObjectHandles() const { return _buffers_mapped; }
    };
}

#endif
