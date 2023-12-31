/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "model.hpp"

namespace mcvk::Renderer {
    std::vector<VkVertexInputBindingDescription> Model::Vertex::GetBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0,  VK_FORMAT_R32G32B32_SFLOAT,  offsetof(Vertex, position) });
        attributeDescriptions.push_back({ 1, 0,  VK_FORMAT_R32G32B32_SFLOAT,  offsetof(Vertex, colour) });
        attributeDescriptions.push_back({ 2, 0,  VK_FORMAT_R32G32B32_SFLOAT,  offsetof(Vertex, normal) });
        attributeDescriptions.push_back({ 3, 0,  VK_FORMAT_R32G32_SFLOAT,     offsetof(Vertex, uv) });

        return attributeDescriptions;
    }
}
