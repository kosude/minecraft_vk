/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include <volk/volk.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace mcvk::ResourceMgr {
    struct ModelResource;
}

namespace mcvk::Renderer {
    class Model {
    public:
        struct Vertex {
            glm::vec3 position{};
            glm::vec3 colour{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

            bool operator==(const Vertex &other) const {
                return
                    (position == other.position) &&
                    (colour == other.colour) &&
                    (normal == other.normal) &&
                    (uv == other.uv);
            }
        };

        static Model CreateFromResource(const ResourceMgr::ModelResource &resource);

        std::vector<Vertex> vertices;
        inline size_t GetVertexDataSize() const { return vertices.size() * sizeof(Vertex); }
        inline void *GetVertexDataPtr() const { return (void *) vertices.data(); }

        std::vector<uint16_t> indices;
        inline size_t GetIndexDataSize() const { return indices.size() * sizeof(uint16_t); }
        inline void *GetIndexDataPtr() const { return (void *) indices.data(); }
        inline static VkIndexType GetIndexType() { return VK_INDEX_TYPE_UINT16; }
    };
}
