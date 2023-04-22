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
    Game::Game() {
        Utils::Note("Initialising game");
    }

    Game::~Game() {
        Utils::Note("Terminating game");
    }
}
