/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __renderer__window_hpp
#define __renderer__window_hpp

#include <glfw/include/GLFW/glfw3.h>

#include <iostream>

namespace MCVK::Renderer {
    /**
     * @brief Window class built on GLFW
     */
    class Window {
    private:
        friend class Renderer;

        GLFWwindow *_handle;

    public:
        /**
         * @brief Create a new window
         *
         * @param width Window width
         * @param height Window height
         * @param title Windowbar title
         */
        Window(
            const uint32_t &width,
            const uint32_t &height,
            const std::string &title
        );

        /**
         * @brief Destroy the window
         */
        void Destroy();

        /**
         * @brief Update logic for the window
         *
         * @return False if the window should close
         */
        bool Update();

        /**
         * @brief Terminate GLFW
         */
        static void TerminateGLFW();
    };
}

#endif
