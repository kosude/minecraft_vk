/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __game__game_hpp
#define __game__game_hpp

#include "renderer/renderer.hpp"
#include "renderer/window.hpp"

namespace MCVK::Game {
    /**
     * @brief Game application class
     */
    class Game {
    private:
        Renderer::Window _main_window;
        Renderer::Renderer _renderer;

    public:
        /**
         * @brief Initialise the game process
         */
        Game();

        /**
         * @brief Terminate the game process
         *
         */
        ~Game();

        /**
         * @brief Game main loop
         */
        void Run();
    };
}

#endif
