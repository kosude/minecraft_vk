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
            if (!_window.Update()) {
                break;
            }

            if (auto drawbuf = _renderer.BeginDrawCommandBuffer()) {
                _renderer.CmdBeginRenderPass(drawbuf);



                _renderer.CmdEndRenderPass(drawbuf);

                _renderer.EndDrawCommandBuffer();
            }
        }

        // wait for the GPU to complete work
        _renderer.WaitDeviceIdle();

        Utils::Info("Window closed");
    }
}
