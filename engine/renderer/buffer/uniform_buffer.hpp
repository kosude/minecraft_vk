/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "buffer.hpp"

#include "renderer/renderer.hpp"

namespace mcvk::Renderer {
    class UniformBuffer : private Buffer {
    public:
        UniformBuffer(const Renderer &renderer, VkDeviceSize size);
        ~UniformBuffer() override;

        const VkBuffer &GetBuffer() const;
        using Buffer::GetSize;

        void Write(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    private:
        struct BufferHandle {
            VkBuffer buf;
            VkDeviceMemory mem;
            void *mapped;
        };

        const Renderer &_renderer;

        std::vector<BufferHandle> _buffer_handles;

        void _Map(uint32_t index, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    };
}
