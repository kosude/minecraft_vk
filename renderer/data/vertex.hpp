/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __data__vertex_hpp
#define __data__vertex_hpp

#include <volk/volk.h>
#include <glm/glm/glm.hpp>

#include <array>

namespace MCVK::Renderer::Data {
    /**
     * @brief Struct containing per-vertex properties/attributes
     */
    struct Vertex {
    public:
        glm::vec2 position;
        glm::vec3 colour;

        /**
         * @brief Get the vertex data binding information
         */
        static VkVertexInputBindingDescription GetBindingDescription();

        /**
         * @brief Get the description of each vertex attribute in an array
         */
        static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
    };
}

#endif
