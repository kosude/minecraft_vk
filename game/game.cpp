/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "game.hpp"

#include "utils/log.hpp"

namespace mcvk::Game {
    Game::Game()
        : _window{720, 540, "Minecraft Vulkan"}, _renderer{_window} {
    }

    Game::~Game() {
    }

    void Game::Run() {
        Utils::Info("Entering main loop...");
        while (true) {
            if (!_window.Update())
                break;
        }

        Utils::Info("Window closed");
    }
}
