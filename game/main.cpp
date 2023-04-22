/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <iostream>

#include "utils/log.hpp"

using namespace VKGame;

int main() {
    Utils::Log("Hello world");
    Utils::Note("Hello world");
    Utils::Warn("Hello world");
    Utils::Error("Hello world");

    return 0;
}
