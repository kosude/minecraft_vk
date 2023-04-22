/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __renderer__renderer_hpp
#define __renderer__renderer_hpp

namespace VKGame::Renderer {
    /**
     * @brief Game renderer (Vulkan)
     */
    class Renderer {
    public:
        /**
         * @brief Construct a new game renderer
         */
        Renderer();

        /**
         * @brief Destroy the game renderer -- clean up Vulkan objects, etc.
         */
        ~Renderer();
    };
}

#endif
