/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "game.hpp"

#include "util/log.hpp"

namespace mcvk::Game {
    Game::Game() {
        Utils::Log("Log message");
        Utils::Info("Info message");
        Utils::Warn("Warning message");
        Utils::Error("Error message");
        Utils::Fatal("Fatal error message");
    }

    Game::~Game() {

    }

    void Game::Run() {

    }
}
