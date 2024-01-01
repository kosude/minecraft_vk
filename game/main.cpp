/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "game/game.hpp"
#include "utils/log.hpp"

#include <stdexcept>

using namespace mcvk;

int main(int argc, char **argv) {
    Utils::ResetLogColour();

    try {
        Game::Game game{};

        game.Run();
    } catch (const std::exception &e) {
        Utils::Fatal(e.what(), false);
        return EXIT_FAILURE;
    }

    return 0;
}
