/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <iostream>

#include "utils/log.hpp"

#include "game.hpp"

namespace VKGame::Game {
    Game::Game() : _main_window(640, 480, "VKGame"), _renderer(_main_window) {
        Utils::Note("Initialising game");
    }

    Game::~Game() {
        Utils::Note("Terminating game");

        _renderer.Destroy();

        _main_window.Destroy();
        Renderer::Window::TerminateGLFW();
    }

    void Game::Run() {
        Utils::Note("Thank you for playing! ^-^");

        while (true) {
            // exit when main window closes
            if (!_main_window.Update()) {
                break;
            }
        }
    }
}
