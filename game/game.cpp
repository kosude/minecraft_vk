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

        Renderer::VertexBuffer vbo{_renderer.GetDevice(), model.GetVertexDataSize()};
        vbo.Map();
        vbo.Write(model.GetVertexDataPtr());
        vbo.Unmap();
        Renderer::IndexBuffer ibo{_renderer.GetDevice(), model.GetIndexDataSize(), Renderer::Model::GetIndexType()};
        ibo.Map();
        ibo.Write(model.GetIndexDataPtr());
        ibo.Unmap();

        Renderer::UniformBuffer ubo_global{_renderer,
                                           Renderer::UniformBuffer::AlignOffset(_renderer.GetDevice(), sizeof(GlobalUniformData))};
        Renderer::UniformBuffer ubo_model{_renderer,
                                           Renderer::UniformBuffer::AlignOffset(_renderer.GetDevice(), sizeof(ModelUniformData))};

        ResourceMgr::MaterialResource mat;
        _resources.Load("grass_block.material", mat);
        auto grass_img_config = Renderer::Image::Config::Defaults(
            {(uint32_t) mat.colourmap->width, (uint32_t) mat.colourmap->height}, VK_FORMAT_R8G8B8A8_SRGB);
        Renderer::Image grass_img{_renderer.GetDevice(), grass_img_config, *mat.colourmap};

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
            .AddWriteBuffer(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, ubo_model, 0,
                            Renderer::UniformBuffer::AlignOffset(_renderer.GetDevice(), sizeof(ModelUniformData)))
            .AddWriteImage(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, grass_img)
            .UpdateSet(_renderer.GetDevice(), dset);

        const Renderer::GraphicsPipeline &g_simple = _renderer.Pipelines().GraphicsByName("g_simple");
        const Renderer::GraphicsPipeline &g_wireframe = _renderer.Pipelines().GraphicsByName("g_wireframe");

        Utils::Info("Entering main loop...");
        while (true) {
            if (!_window.Update()) {
                break;
            }

            {
                GlobalUniformData d;
                d.projection = glm::perspective(glm::radians(70.0f), _window.GetAspectRatio(), 0.1f, 100.0f);
                d.view = glm::lookAt(glm::vec3{0.0f, -1.5f, -2.0f}, glm::vec3{0.0f}, glm::vec3{0.0f, 3.5f, 0.0f});
                ubo_global.Write(&d);
            }
            {
                ModelUniformData d;
                d.transform = glm::rotate(glm::mat4{1.0f}, (float) glm::radians(std::fmod(glfwGetTime() * 100, 360)), glm::vec3{0, 1, 0});
                ubo_model.Write(&d);
            }

            if (auto drawbuf = _renderer.BeginDrawCommandBuffer()) {
                drawbuf->BeginRenderPass({ (float) std::abs(sin(glfwGetTime() * 2)), 0.0, 0.0 });

                drawbuf->UpdateViewportAndScissor();

                drawbuf->BindPipeline(g_simple);
                drawbuf->BindVertexBuffer(vbo);
                drawbuf->BindIndexBuffer(ibo);
                drawbuf->BindDescriptorSets(g_simple, { dset }, { 0 });
                drawbuf->DrawIndexed(model.indices.size());

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
