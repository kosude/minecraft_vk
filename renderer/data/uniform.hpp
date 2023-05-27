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
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
}

#endif
