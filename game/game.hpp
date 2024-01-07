/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "engine/renderer/renderer.hpp"
#include "engine/renderer/window.hpp"
#include "engine/resource_mgr/resource_mgr.hpp"

#include <filesystem>

namespace mcvk::Game {
    class Game {
    public:
        Game(const std::filesystem::path &resourcedir);
        ~Game();

        void Run();

    private:
        ResourceMgr::ResourceManager _resources;

        Renderer::Window _window;
        Renderer::Renderer _renderer;
    };
}
