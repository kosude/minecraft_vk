/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "vertex.hpp"

namespace MCVK::Renderer::Data {
    VkVertexInputBindingDescription Vertex::GetBindingDescription() {
        VkVertexInputBindingDescription description = {};

        description.binding = 0;
        description.stride = sizeof(Vertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    std::array<VkVertexInputAttributeDescription, 2> Vertex::GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> arr = {};

        VkVertexInputAttributeDescription &bind_position = arr[0];
        VkVertexInputAttributeDescription &bind_colour = arr[1];

        // position binding (location = 0)
        bind_position.binding = 0;
        bind_position.location = 0;
        bind_position.format = VK_FORMAT_R32G32_SFLOAT; // 2 32-bit float components (vec2)
        bind_position.offset = offsetof(Vertex, position);

        // colour binding (location = 1)
        bind_colour.binding = 0;
        bind_colour.location = 1;
        bind_colour.format = VK_FORMAT_R32G32B32_SFLOAT; // 3 32-bit float components (vec3)
        bind_colour.offset = offsetof(Vertex, colour);

        return arr;
    }
}
