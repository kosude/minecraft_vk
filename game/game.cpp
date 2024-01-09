/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "game.hpp"

#include "engine/renderer/resource/buffer.hpp"
#include "engine/renderer/resource/descriptor.hpp"
#include "engine/renderer/data/model.hpp"
#include "engine/utils/log.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace mcvk::Game {
    struct GlobalUniformData {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
    };
    struct ModelUniformData {
        glm::mat4 transform{1.0f};
    };

    Game::Game(const std::filesystem::path &resourcedir)
        : _resources{resourcedir}, _window{720, 540, "Minecraft Vulkan"}, _renderer{_window, _resources} {
    }

    Game::~Game() {
    }

    void Game::Run() {
        ResourceMgr::ModelResource mdl;
        _resources.Load("cube.model", mdl);
        auto model = Renderer::Model::CreateFromResource(mdl);
        ResourceMgr::ModelResource monkey_mdl;
        _resources.Load("monkey.model", monkey_mdl);
        auto monkey_model = Renderer::Model::CreateFromResource(monkey_mdl);

        Renderer::VertexBuffer vbo{_renderer.GetDevice(), model.GetVertexDataSize()};
        vbo.Map();
        vbo.Write(model.GetVertexDataPtr());
        vbo.Unmap();
        Renderer::IndexBuffer ibo{_renderer.GetDevice(), model.GetIndexDataSize(), Renderer::Model::GetIndexType()};
        ibo.Map();
        ibo.Write(model.GetIndexDataPtr());
        ibo.Unmap();

        Renderer::VertexBuffer monkey_vbo{_renderer.GetDevice(), monkey_model.GetVertexDataSize()};
        monkey_vbo.Map();
        monkey_vbo.Write(monkey_model.GetVertexDataPtr());
        monkey_vbo.Unmap();
        Renderer::IndexBuffer monkey_ibo{_renderer.GetDevice(), monkey_model.GetIndexDataSize(), Renderer::Model::GetIndexType()};
        monkey_ibo.Map();
        monkey_ibo.Write(monkey_model.GetIndexDataPtr());
        monkey_ibo.Unmap();

        Renderer::UniformBuffer ubo_global{_renderer, sizeof(GlobalUniformData)};
        Renderer::UniformBuffer ubo_model{_renderer, sizeof(ModelUniformData) * 2}; // per-instance data

        ResourceMgr::MaterialResource mat;
        _resources.Load("grass_block.material", mat);
        auto grass_img_config = Renderer::Image::Config::Defaults(
            {(uint32_t) mat.colourmap->width, (uint32_t) mat.colourmap->height}, VK_FORMAT_R8G8B8A8_SRGB);
        Renderer::Image grass_img{_renderer.GetDevice(), grass_img_config, *mat.colourmap};

        ResourceMgr::MaterialResource monkey_mat;
        _resources.Load("monkey.material", monkey_mat);
        auto monkey_img_config = Renderer::Image::Config::Defaults(
            {(uint32_t) monkey_mat.colourmap->width, (uint32_t) monkey_mat.colourmap->height}, VK_FORMAT_R8G8B8A8_SRGB);
        Renderer::Image monkey_img{_renderer.GetDevice(), monkey_img_config, *monkey_mat.colourmap};

        VkDescriptorSetLayout dset_layout = Renderer::DescriptorSetLayoutBuilder::New()
            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT)
            .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT) // TODO find out how to properly do multiple samplers
            .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build(_renderer.GetDevice());

        _renderer.BuildPipelines({ dset_layout });

        std::vector<Renderer::DescriptorAllocatorGrowable::PoolSizeRatio> descriptor_ratios{
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
        };
        Renderer::DescriptorAllocatorGrowable dalloc{_renderer.GetDevice(), 2, descriptor_ratios};

        VkDescriptorSet dset = dalloc.AllocateSet(dset_layout);
        Renderer::DescriptorWriter::New()
            .AddWriteBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ubo_global)
            .AddWriteBuffer(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, ubo_model, 0, sizeof(ModelUniformData))
            .AddWriteImage(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, grass_img)
            .AddWriteImage(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, monkey_img)
            .UpdateSet(_renderer.GetDevice(), dset);

        const Renderer::GraphicsPipeline &g_simple = _renderer.Pipelines().GraphicsByName("g_simple");
        const Renderer::GraphicsPipeline &g_wireframe = _renderer.Pipelines().GraphicsByName("g_wireframe");
        const Renderer::GraphicsPipeline &g_monkey = _renderer.Pipelines().GraphicsByName("g_monkey");

        Utils::Info("Entering main loop...");
        while (true) {
            if (!_window.Update()) {
                break;
            }

            {
                GlobalUniformData d;
                d.projection = glm::perspective(glm::radians(70.0f), _window.GetAspectRatio(), 0.1f, 100.0f);
                d.view = glm::lookAt(glm::vec3{0.0f, -2.0f, -3.0f}, glm::vec3{0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
                ubo_global.Write(&d);
            }
            {
                ModelUniformData d[2];
                d[0].transform = glm::rotate(glm::mat4{1.0f}, (float) glm::radians(std::fmod(glfwGetTime() * 100, 360)), glm::vec3{0, 1, 0});
                d[1].transform = glm::scale(glm::rotate(glm::translate(glm::mat4{1.0f}, glm::vec3{0, -0.9f, 0}),
                    (float) glm::radians(std::fmod(glfwGetTime() * 100, 360)), glm::vec3{0, 1, 0}), glm::vec3{0.25f});

                ubo_model.Write(&d);
            }

            if (auto drawbuf = _renderer.BeginDrawCommandBuffer()) {
                drawbuf->BeginRenderPass({ 0.03, 0.03, 0.03 }); // clear to a dark grey

                drawbuf->UpdateViewportAndScissor();

                drawbuf->BindPipeline(g_simple);
                drawbuf->BindVertexBuffer(vbo);
                drawbuf->BindIndexBuffer(ibo);
                drawbuf->BindDescriptorSets(g_simple, { dset }, { 0 });
                drawbuf->DrawIndexed(model.indices.size());

                drawbuf->BindPipeline(g_monkey);
                drawbuf->BindVertexBuffer(monkey_vbo);
                drawbuf->BindIndexBuffer(monkey_ibo);
                drawbuf->BindDescriptorSets(g_simple, { dset }, { sizeof(ModelUniformData) });
                drawbuf->DrawIndexed(monkey_model.indices.size());

                drawbuf->EndRenderPass();
                drawbuf->End();
            }
        }

        // wait for the GPU to complete work
        _renderer.WaitDeviceIdle();

        Utils::Info("Window closed");

        vkDestroyDescriptorSetLayout(_renderer.GetDevice().GetDevice(), dset_layout, nullptr);
    }
}
