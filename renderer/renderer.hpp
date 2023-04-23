/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __renderer__renderer_hpp
#define __renderer__renderer_hpp

#include <volk/volk.h>

#include <vector>
#include <iostream>

namespace VKGame::Renderer {
    /**
     * @brief Game renderer (Vulkan)
     */
    class Renderer {
    private:
        VkInstance _instance;
#       ifdef DEBUG
            VkDebugUtilsMessengerEXT _debug_messenger;
#       endif

        void _CreateInstance(); // (also creates a debug messenger if in debug config)

    public:
        /**
         * @brief Construct a new game renderer
         */
        Renderer();

        /**
         * @brief Destroy the game renderer -- clean up Vulkan objects, etc.
         */
        void Destroy();
    };
}

#endif
