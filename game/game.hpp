/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "engine/renderer/renderer.hpp"
#include "engine/renderer/window.hpp"

namespace mcvk::Game {
    class Game {
    public:
        Game();
        ~Game();

        void Run();

    private:
        Renderer::Window _window;
        Renderer::Renderer _renderer;
    };
}
