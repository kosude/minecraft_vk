/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __data__uniform_hpp
#define __data__uniform_hpp

#include <glm/glm/glm.hpp>

namespace MCVK::Renderer::Data {
    /**
     * @brief Struct containing uniform layout data
     */
    struct Uniform {
    public:
        // align memory based on vulkan alignment expectations:
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout

        alignas(4) float colour_multiplier;

        struct {
            alignas(16) glm::mat4 model;
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 proj;
        } transform;
    };
}

#endif
