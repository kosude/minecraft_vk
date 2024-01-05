/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "game.hpp"

#include "engine/renderer/buffer/vertex_buffer.hpp"
#include "engine/renderer/buffer/index_buffer.hpp"
#include "engine/renderer/buffer/uniform_buffer.hpp"
#include "engine/renderer/data/model.hpp"

#include "utils/log.hpp"

namespace mcvk::Game {
    struct GlobalUniformData {
        glm::vec2 offset{0.0f, 0.0f};
    };

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

        Renderer::UniformBuffer ubo{_renderer, sizeof(GlobalUniformData)};

        VkDescriptorPool dpool;
        {
            VkDescriptorPoolSize dpoolsizes[1];
            dpoolsizes[0].descriptorCount = 1;
            dpoolsizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            VkDescriptorPoolCreateInfo dpoolinfo{};
            dpoolinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            dpoolinfo.maxSets = 1;
            dpoolinfo.poolSizeCount = 1;
            dpoolinfo.pPoolSizes = dpoolsizes;
            vkCreateDescriptorPool(_renderer.GetDevice().GetDevice(), &dpoolinfo, nullptr, &dpool);
        }
        std::vector<VkDescriptorSetLayout> dsetlayouts(1);
        {
            VkDescriptorSetLayoutBinding bindings[1];
            bindings[0].binding = 0;
            bindings[0].descriptorCount = 1;
            bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[0].pImmutableSamplers = nullptr;
            bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutCreateInfo dsetlayoutinfo{};
            dsetlayoutinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            dsetlayoutinfo.bindingCount = 1;
            dsetlayoutinfo.pBindings = bindings;
            vkCreateDescriptorSetLayout(_renderer.GetDevice().GetDevice(), &dsetlayoutinfo, nullptr, &dsetlayouts[0]);
        }
        std::vector<VkDescriptorSet> dsets(1);
        {
            VkDescriptorSetAllocateInfo dsetallocinfo{};
            dsetallocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            dsetallocinfo.descriptorPool = dpool;
            dsetallocinfo.descriptorSetCount = 1;
            dsetallocinfo.pSetLayouts = dsetlayouts.data();
            vkAllocateDescriptorSets(_renderer.GetDevice().GetDevice(), &dsetallocinfo, dsets.data());

            VkWriteDescriptorSet writes[1];
            // write uniform buffer info to the descriptor set
            VkDescriptorBufferInfo uboinfo{};
                uboinfo.buffer = ubo.GetBuffer();
                uboinfo.offset = 0;
                uboinfo.range = ubo.GetSize();
            writes[0] = {};
            writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[0].dstSet = dsets[0];
            writes[0].dstBinding = 0;
            writes[0].descriptorCount = 1;
            writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[0].pBufferInfo = &uboinfo;
            vkUpdateDescriptorSets(_renderer.GetDevice().GetDevice(), 1, writes, 0, nullptr);
        }

        _renderer.BuildPipelines(dsetlayouts);

        Utils::Info("Entering main loop...");
        while (true) {
            if (!_window.Update()) {
                break;
            }

            const Renderer::GraphicsPipeline &g_simple = _renderer.Pipelines().SimpleGraphics();

            GlobalUniformData ubo_data{
                { 0.2f, 0.2f }
            };
            ubo.Write(&ubo_data);

            if (auto drawbuf = _renderer.BeginDrawCommandBuffer()) {
                drawbuf->BeginRenderPass({ 0.03, 0.03, 0.03 }); // clear to a dark grey

                drawbuf->UpdateViewportAndScissor();

                drawbuf->BindPipeline(g_simple);
                drawbuf->BindVertexBuffer(vbo);
                drawbuf->BindDescriptorSets(g_simple, dsets);

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

        vkResetDescriptorPool(_renderer.GetDevice().GetDevice(), dpool, 0);
        vkDestroyDescriptorSetLayout(_renderer.GetDevice().GetDevice(), dsetlayouts[0], nullptr);
        vkDestroyDescriptorPool(_renderer.GetDevice().GetDevice(), dpool, nullptr);
    }
}
