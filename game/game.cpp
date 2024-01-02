/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "game.hpp"

#include "engine/renderer/buffer/vertex_buffer.hpp"
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
            { { -0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -0.5f,  0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  0.5f,  0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  0.5f, -0.5f, 0.5f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
        };
        triangle.indices = {
            0, 1, 2, 0, 3, 2
        };

        Renderer::VertexBuffer vbo{_renderer.GetDevice(), triangle.GetVertexDataSize()};
        vbo.Map();
        vbo.Write(triangle.GetVertexDataPtr());
        vbo.Unmap();

        Renderer::IndexBuffer ibo{_renderer.GetDevice(), triangle.GetIndexDataSize(), Renderer::Model::GetIndexType()};
        ibo.Map();
        ibo.Write(triangle.GetIndexDataPtr());
        ibo.Unmap();

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
                drawbuf->BindIndexBuffer(ibo);

                drawbuf->DrawIndexed(triangle.indices.size());

                drawbuf->EndRenderPass();
                drawbuf->End();
            }
        }

        // wait for the GPU to complete work
        _renderer.WaitDeviceIdle();

        Utils::Info("Window closed");
    }
}
