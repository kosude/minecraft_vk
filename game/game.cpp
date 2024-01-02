/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "game.hpp"

#include "engine/renderer/buffer/buffer.hpp"
#include "engine/renderer/data/model.hpp"

#include "utils/log.hpp"

namespace mcvk::Game {
    Game::Game()
        : _window{720, 540, "Minecraft Vulkan"}, _renderer{_window} {
    }

    Game::~Game() {
    }

    void Game::Run() {
        Renderer::Model triangle;
        triangle.vertices = {
            { { -0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }
        };

        Renderer::Buffer vbo{_renderer.GetDevice(), triangle.GetVertexDataSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
        vbo.Map();
        vbo.Write(triangle.GetVertexDataPtr());
        vbo.Unmap();

        Utils::Info("Entering main loop...");
        while (true) {
            if (!_window.Update()) {
                break;
            }

            if (auto drawbuf = _renderer.BeginDrawCommandBuffer()) {
                drawbuf->BeginRenderPass();

                drawbuf->UpdateViewportAndScissor();

                drawbuf->BindPipeline(_renderer.GetDefaultGraphicsPipeline());
                drawbuf->BindVertexBuffer(vbo);

                drawbuf->EndRenderPass();
                drawbuf->End();
            }
        }

        // wait for the GPU to complete work
        _renderer.WaitDeviceIdle();

        Utils::Info("Window closed");
    }
}
