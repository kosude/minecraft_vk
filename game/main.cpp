/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <stdexcept>

#include "game/game.hpp"
#include "utils/log.hpp"

using namespace VKGame;

int main() {
    Game::Game game;

    try {
        game.Run();
    } catch (const std::exception &e) {
        Utils::Error(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
