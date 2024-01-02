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
        Renderer::Model model;
        model.vertices = {
            { { -0.6f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -0.6f,  0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -0.1f,  0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -0.1f, -0.5f, 0.5f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },

            { {  0.1f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  0.1f,  0.5f, 0.5f }, { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  0.6f,  0.5f, 0.5f }, { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  0.6f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
        };
        model.indices = {
            0, 1, 2, 0, 3, 2,
            4, 5, 6, 4, 7, 6,
        };

        uint16_t arr1[] = {
            0, 1, 2, 0, 3, 2,
        };
        uint16_t arr2[] = {
            4, 5, 6, 4, 7, 6,
        };

        Renderer::VertexBuffer vbo{_renderer.GetDevice(), model.GetVertexDataSize()};
        vbo.Map();
        vbo.Write(model.GetVertexDataPtr());
        vbo.Unmap();

        Renderer::IndexBuffer ibo1{_renderer.GetDevice(), sizeof(arr1), Renderer::Model::GetIndexType()};
        ibo1.Map();
        ibo1.Write(arr1);
        ibo1.Unmap();
        Renderer::IndexBuffer ibo2{_renderer.GetDevice(), sizeof(arr2), Renderer::Model::GetIndexType()};
        ibo2.Map();
        ibo2.Write(arr2);
        ibo2.Unmap();

        Utils::Info("Entering main loop...");
        while (true) {
            if (!_window.Update()) {
                break;
            }

            if (auto drawbuf = _renderer.BeginDrawCommandBuffer()) {
                drawbuf->BeginRenderPass({ 0.03, 0.03, 0.03 }); // clear to a dark grey

                drawbuf->UpdateViewportAndScissor();

                // drawbuf->BindPipeline(_renderer.Pipelines().SimpleGraphics());
                drawbuf->BindPipeline(_renderer.Pipelines().SimpleWireframeGraphics());
                drawbuf->BindVertexBuffer(vbo);

                drawbuf->BindIndexBuffer(ibo1);
                drawbuf->DrawIndexed(6);
                drawbuf->BindIndexBuffer(ibo2);
                drawbuf->DrawIndexed(6);

                drawbuf->EndRenderPass();
                drawbuf->End();
            }
        }

        // wait for the GPU to complete work
        _renderer.WaitDeviceIdle();

        Utils::Info("Window closed");
    }
}
