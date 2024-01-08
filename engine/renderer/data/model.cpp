/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "model.hpp"

#include "resource_mgr/resource_entity.hpp"
#include "utils/hash.hpp"

#include <unordered_map>

namespace std {
    template <>
    struct hash<mcvk::Renderer::Model::Vertex> {
        size_t operator()(mcvk::Renderer::Model::Vertex const &vertex) const {
            size_t seed = 0;
            mcvk::Utils::HashCombine(seed, vertex.position, vertex.colour, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace mcvk::Renderer {
    Model Model::CreateFromResource(const ResourceMgr::ModelResource &resource) {
        std::vector<Vertex> vertices{};
        std::vector<uint16_t> indices{};

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto &shape : resource.to_shapes) {
            for (const auto &index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        resource.to_attrib.vertices[3 * index.vertex_index + 0],
                        resource.to_attrib.vertices[3 * index.vertex_index + 1],
                        resource.to_attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.colour = {
                        resource.to_attrib.colors[3 * index.vertex_index + 0],
                        resource.to_attrib.colors[3 * index.vertex_index + 1],
                        resource.to_attrib.colors[3 * index.vertex_index + 2]
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        resource.to_attrib.normals[3 * index.normal_index + 0],
                        resource.to_attrib.normals[3 * index.normal_index + 1],
                        resource.to_attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        resource.to_attrib.texcoords[3 * index.texcoord_index + 0],
                        resource.to_attrib.texcoords[3 * index.texcoord_index + 1]
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }

        Model mdl{};
        mdl.vertices = vertices;
        mdl.indices = indices;
        return mdl;
    }

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
