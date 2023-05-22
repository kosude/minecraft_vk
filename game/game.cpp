/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <iostream>

#include "utils/log.hpp"

#include "game.hpp"

#define GAME_WINDOW_WIDTH 640
#define GAME_WINDOW_HEIGHT 480
#define GAME_WINDOW_TITLE "Minecraft Vulkan"

namespace MCVK::Game {
    Game::Game() : _main_window(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, GAME_WINDOW_TITLE), _renderer(_main_window) {
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
