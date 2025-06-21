/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "game/game.hpp"
#include "engine/utils/log.hpp"

using namespace mcvk;

int main(int argc, char **argv) {
    Utils::ResetLogColour();

    auto execdir = std::filesystem::path{argv[0]};
    auto resourcedir = std::filesystem::path{execdir.remove_filename().string() + "/resources/"};

    try {
        Game::Game game{resourcedir};

        game.Run();
    } catch (const std::exception &e) {
        Utils::Fatal(e.what(), false);
        return EXIT_FAILURE;
    }

    return 0;
}
