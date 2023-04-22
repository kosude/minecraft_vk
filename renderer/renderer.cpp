/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <iostream>

#include "utils/log.hpp"

#include "renderer.hpp"

namespace VKGame::Renderer {
    Renderer::Renderer() {
        Utils::Note("Creating renderer");
    }

    Renderer::~Renderer() {
        Utils::Note("Destroying renderer");
    }
}
