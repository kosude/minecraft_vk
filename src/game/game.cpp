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
        : _window{720, 540, "Minecraft Vulkan"}, _renderer{} {
        Utils::Info("Initialising game");
    }

    Game::~Game() {
        Utils::Info("Terminating game");
    }

    void Game::Run() {
        while (true) {
            if (!_window.Update()) {
                break;
            }
        }
    }
}
