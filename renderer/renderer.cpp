/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <iostream>
#include <set>

#include "renderer/device_manager.hpp"
#include "renderer/extension_manager.hpp"
#include "renderer/swap_chain.hpp"
#include "utils/log.hpp"

#include "renderer/data/uniform.hpp"

#include "renderer.hpp"

#define MAX_FRAMES_IN_FLIGHT 2

// TODO: stop hardcoding in this file
#include "data/vertex.hpp"
#include <chrono>
#include <glm/glm/gtc/matrix_transform.hpp>
const std::vector<MCVK::Renderer::Data::Vertex> vertices = {
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { { -0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f } },
    { {  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
    { {  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } }
};
const std::vector<uint16_t> indices = {
    0, 2, 1, 3, 2, 0
};

static VKAPI_ATTR VkBool32 VKAPI_CALL __DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *data, void *user
) {
    // convert the type to a string
    std::string type_str;
    switch (type) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            type_str = "GENERAL";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            type_str = "VALIDATION";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            type_str = "PERFORMANCE";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            type_str = "DEVICE_ADDRESS_BINDING";
            break;
    }

    // redirect to appropriate logging function based on severity
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            MCVK::Utils::Warn(type_str + ": " + data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            MCVK::Utils::Error(type_str + ": " + data->pMessage, false);
            break;
        default:
            MCVK::Utils::Log(type_str + ": " + data->pMessage);
            break;
    }

    return VK_FALSE;
}

// TODO: stop hardcoding this here
static void _UpdateUniformBuffer(MCVK::Renderer::Buffer::UniformBuffer &ubo, uint32_t current_image, VkExtent2D swapchain_extent) {
    static auto start_time = std::chrono::high_resolution_clock::now();

    // calculate time in seconds since rendering started
    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

    // uniform buffer data will be updated
    auto udata = MCVK::Renderer::Data::Uniform();

    // rotate object around z axis
    udata.transform.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    udata.transform.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    udata.transform.proj = glm::perspective(glm::radians(45.0f), swapchain_extent.width / (float) swapchain_extent.height, 0.1f, 10.0f);

    // flip scale of y axis as glm was designed for opengl in which y coordinates are flipped relative to in vulkan
    udata.transform.proj[1][1] *= -1;

    udata.colour_multiplier = glm::abs(glm::sin(time));

    ubo.SetData(&udata, current_image);
}

namespace MCVK::Renderer {
    void Renderer::_CreateInstance(VkInstance *instance) {
        VkInstanceCreateInfo instance_create_info = {};

        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = nullptr;

        std::vector<const char *> enabled_layers = ExtensionManager::GetRequiredLayers();
        ExtensionManager::CheckLayerNames(enabled_layers, ExtensionImportance::Required);
        std::vector<const char *> enabled_exts = ExtensionManager::GetRequiredInstanceExtensions();
        ExtensionManager::CheckInstanceExtensionNames(enabled_exts, ExtensionImportance::Required);

        instance_create_info.enabledLayerCount = enabled_layers.size();
        instance_create_info.ppEnabledLayerNames = (const char *const *) enabled_layers.data();

        instance_create_info.enabledExtensionCount = enabled_exts.size();
        instance_create_info.ppEnabledExtensionNames = (const char *const *) enabled_exts.data();

#       ifdef APPLE
            instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#       endif

        // debug messenger create info (if in debug mode) used for pNext of instance and separate messenger
#       ifdef DEBUG
            VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {};
            debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debug_messenger_create_info.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debug_messenger_create_info.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
            debug_messenger_create_info.pfnUserCallback = __DebugCallback;

            instance_create_info.pNext = &debug_messenger_create_info;
#       endif

        if (vkCreateInstance(&instance_create_info, nullptr, instance)) {
            Utils::Error("Failed to create Vulkan instance");
        }

        // load vulkan entrypoints + extensions
        volkLoadInstance(*instance);

        // create debug messenger if in debug mode
#       ifdef DEBUG
            if (vkCreateDebugUtilsMessengerEXT(*instance, &debug_messenger_create_info, nullptr, &_debug_messenger)) {
                Utils::Error("Failed to create Vulkan debug messenger");
            }
#       endif
    }

    void Renderer::_CreateSurface(const Window &window, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(_instance, window._handle, nullptr, surface)) {
            Utils::Error("Failed to create Vulkan surface for game window");
        }
    }

    void Renderer::_CreateLogicalDevice(VkDevice *device) {
        DeviceQueueFamilyInfo queue_family_info = DeviceManager::GetDeviceQueueFamilyInfo(_physical_device, _main_surface);

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families = {
            queue_family_info.graphics_family_index.value(),
            queue_family_info.present_family_index.value(),
            queue_family_info.compute_family_index.value(),
            queue_family_info.transfer_family_index.value()
        };

        float queue_priority = 1.0f;

        // create one queue for each unique queue family
        for (uint32_t qf : unique_queue_families) {
            VkDeviceQueueCreateInfo queue_create_info {};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = qf;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;

            queue_create_infos.push_back(queue_create_info);
        }

        // requesting device features
        // we do not need to check these as they would already have been asserted when selecting the physical device
        VkPhysicalDeviceFeatures device_features = ExtensionManager::GetRequiredDeviceFeatures();

        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.queueCreateInfoCount = queue_create_infos.size();

        device_create_info.pEnabledFeatures = &device_features;

        // setting layers for compatibility with older Vulkan implementations...
        std::vector<const char *> enabled_layers = ExtensionManager::GetRequiredLayers();
        device_create_info.enabledLayerCount = enabled_layers.size();
        device_create_info.ppEnabledLayerNames = enabled_layers.data();

        // extensions - again, these would have been checked when selecting physical device.
        std::vector<const char *> enabled_exts = ExtensionManager::GetRequiredDeviceExtensions();
        device_create_info.enabledExtensionCount = enabled_exts.size();
        device_create_info.ppEnabledExtensionNames = enabled_exts.data();

        if (vkCreateDevice(_physical_device, &device_create_info, nullptr, device)) {
            Utils::Error("Failed to create Vulkan device interface");
        }

        // retrieve queue handles
        // 0 is used as the queue index as there is only one queue created for each family.
        vkGetDeviceQueue(*device, queue_family_info.graphics_family_index.value(), 0, &_graphics_queue);
        vkGetDeviceQueue(*device, queue_family_info.present_family_index.value(), 0, &_present_queue);
        vkGetDeviceQueue(*device, queue_family_info.transfer_family_index.value(), 0, &_transfer_queue);
    }

    void Renderer::_CreateFramebuffers(const VkDevice &device) {
        _main_swapchain_framebuffers.resize(_main_swapchain_image_views.size());

        // create a framebuffer for each image view
        for (uint32_t i = 0; i < _main_swapchain_image_views.size(); i++) {
            VkImageView attachments[] = {
                _main_swapchain_image_views[i]
            };

            VkFramebufferCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

            info.renderPass = _graphics_pipeline->_render_pass;

            info.attachmentCount = 1;
            info.pAttachments = attachments;

            info.width = _main_swapchain_extent.width;
            info.height = _main_swapchain_extent.height;

            info.layers = 1;

            if (vkCreateFramebuffer(device, &info, nullptr, &_main_swapchain_framebuffers[i])) {
                Utils::Error("Failed to create Vulkan framebuffer");
            }
        }
    }

    void Renderer::_CreateCommandPools(const VkDevice &device, const VkPhysicalDevice &physical_device, const VkSurfaceKHR &surface) {
        DeviceQueueFamilyInfo queue_family_info = DeviceManager::GetDeviceQueueFamilyInfo(physical_device, surface);

        // draw command pool

        VkCommandPoolCreateInfo draw_info = {};
        draw_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        draw_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        draw_info.queueFamilyIndex = queue_family_info.graphics_family_index.value();
        if (vkCreateCommandPool(device, &draw_info, nullptr, &_draw_command_pool)) {
            Utils::Error("Failed to create Vulkan render operations command pool");
        }

        // transfer command pool

        VkCommandPoolCreateInfo transfer_info = {};
        transfer_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        transfer_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        transfer_info.queueFamilyIndex = queue_family_info.transfer_family_index.value();
        if (vkCreateCommandPool(device, &transfer_info, nullptr, &_transfer_command_pool)) {
            Utils::Error("Failed to create Vulkan memory transfer operations command pool");
        }
    }

    void Renderer::_CreateCommandBuffers(const VkDevice &device) {
        _draw_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

        // command buffers for draw operations

        VkCommandBufferAllocateInfo draw_info = {};
        draw_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        draw_info.commandPool = _draw_command_pool;
        draw_info.commandBufferCount = _draw_command_buffers.size();
        draw_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(device, &draw_info, _draw_command_buffers.data())) {
            Utils::Error("Failed to create Vulkan render operations command buffers");
        }
    }

    void Renderer::_RecordDrawCommandBuffer(const uint32_t &framebuffer_index) {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(_draw_command_buffers[_current_frame], &begin_info)) {
            Utils::Error("Failed to begin recording draw operations to Vulkan command buffer");
        }

        // start a render pass
        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

        render_pass_info.renderPass = _graphics_pipeline->_render_pass;
        render_pass_info.framebuffer = _main_swapchain_framebuffers[framebuffer_index];

        render_pass_info.renderArea.offset = { 0, 0 };
        render_pass_info.renderArea.extent = _main_swapchain_extent;

        // clear the screen to a specified colour
        VkClearValue clear_colour = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_colour;

        vkCmdBeginRenderPass(_draw_command_buffers[_current_frame], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        // bind graphics pipeline
        vkCmdBindPipeline(_draw_command_buffers[_current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline->_handle);

        // set dynamic state: viewport
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_main_swapchain_extent.width);
        viewport.height = static_cast<float>(_main_swapchain_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_draw_command_buffers[_current_frame], 0, 1, &viewport);

        // set dynamic state: scissor
        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = _main_swapchain_extent;
        vkCmdSetScissor(_draw_command_buffers[_current_frame], 0, 1, &scissor);

        // TODO: don't hardcode in this file
        // bind vertex buffer
        VkBuffer vertex_buffers[] = { _vertex_buffer->GetObjectHandle() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(_draw_command_buffers[_current_frame], 0, 1, vertex_buffers, offsets);
        // bind index buffer
        vkCmdBindIndexBuffer(_draw_command_buffers[_current_frame], _index_buffer->GetObjectHandle(), 0, VK_INDEX_TYPE_UINT16);

        // bind descriptor sets (e.g. for uniform buffers)
        vkCmdBindDescriptorSets(_draw_command_buffers[_current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline->_pipeline_layout, 0, 1,
            &_graphics_pipeline->_descriptor_sets[_current_frame], 0, nullptr);

        // issue draw call (for triangle)
        vkCmdDrawIndexed(_draw_command_buffers[_current_frame], indices.size(), 1, 0, 0, 0);

        // end render pass
        vkCmdEndRenderPass(_draw_command_buffers[_current_frame]);

        // finish recording operations
        if (vkEndCommandBuffer(_draw_command_buffers[_current_frame])) {
            Utils::Error("Failed to record draw operations to Vulkan command buffer");
        }
    }

    void Renderer::_CreateSynchronisationObjects(const VkDevice &device) {
        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        _image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        _render_complete_sempahores.resize(MAX_FRAMES_IN_FLIGHT);
        _in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (
                vkCreateSemaphore(device, &semaphore_info, nullptr, &_image_available_semaphores[i]) ||
                vkCreateSemaphore(device, &semaphore_info, nullptr, &_render_complete_sempahores[i]) ||
                vkCreateFence(device, &fence_info, nullptr, &_in_flight_fences[i]))
            {
                Utils::Error("Failed to create Vulkan synchronisation primitives");
            }
        }
    }

    void Renderer::_RecreateSwapchain() {
        int width, height;
        glfwGetFramebufferSize(_main_window._handle, &width, &height);

        // pause when window minimised
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(_main_window._handle, &width, &height);
            glfwWaitEvents();
        }

        DeviceWaitIdle();

        // clean up previous swapchain objects
        for (VkFramebuffer fb : _main_swapchain_framebuffers) {
            vkDestroyFramebuffer(_device, fb, nullptr);
        }
        for (VkImageView view : _main_swapchain_image_views) {
            vkDestroyImageView(_device, view, nullptr);
        }
        vkDestroySwapchainKHR(_device, _main_swapchain, nullptr);

        // overwrite swapchain data
        Swapchain swapchain_cl(_device, _physical_device, _main_surface, (uint32_t) width, (uint32_t) height);
        _main_swapchain = swapchain_cl.GetHandle();
        _main_swapchain_images = swapchain_cl.GetImages();
        _main_swapchain_image_views = swapchain_cl.CreateImageViews();
        _main_swapchain_image_format = swapchain_cl.GetImageFormat();
        _main_swapchain_extent = swapchain_cl.GetExtent();

        // recreate framebuffers
        _CreateFramebuffers(_device);
    }

    Renderer::Renderer(Window &main_window) : _main_window(main_window) {
        Utils::Note("Creating renderer");

        if (volkInitialize()) {
            Utils::Error("Failed to load Vulkan (via volk)");
        }

        // create vulkan instance
        _CreateInstance(&_instance);

        // create game window vulkan surface
        _CreateSurface(main_window, &_main_surface);

        // get best vulkan physical device
        uint32_t physical_device_choice_count = 0;
        vkEnumeratePhysicalDevices(_instance, &physical_device_choice_count, nullptr);
        std::vector<VkPhysicalDevice> physical_device_choices(physical_device_choice_count);
        vkEnumeratePhysicalDevices(_instance, &physical_device_choice_count, physical_device_choices.data());

        _physical_device = DeviceManager::GetOptimalPhysicalDevice(physical_device_choices, _main_surface);

        // create vulkan logical device
        _CreateLogicalDevice(&_device);

        // create command pool(s) and their buffer(s)
        _CreateCommandPools(_device, _physical_device, _main_surface);
        _CreateCommandBuffers(_device);

        // TODO: stop hardcoding in this file
        DeviceQueueFamilyInfo queue_family_info = DeviceManager::GetDeviceQueueFamilyInfo(_physical_device, _main_surface);
        _vertex_buffer = std::unique_ptr<Buffer::VertexBuffer>(
            new Buffer::VertexBuffer(
                _device,
                _physical_device,
                sizeof(Data::Vertex) * vertices.size(),
                VK_SHARING_MODE_CONCURRENT,
                {
                    queue_family_info.graphics_family_index.value(),
                    queue_family_info.transfer_family_index.value()
                }
            )
        );
        _vertex_buffer->SetData((void *) vertices.data(), _transfer_command_pool, _transfer_queue);
        _index_buffer = std::unique_ptr<Buffer::IndexBuffer>(
            new Buffer::IndexBuffer(
                _device,
                _physical_device,
                sizeof(uint16_t) * indices.size(),
                VK_SHARING_MODE_CONCURRENT,
                {
                    queue_family_info.graphics_family_index.value(),
                    queue_family_info.transfer_family_index.value()
                }
            )
        );
        _index_buffer->SetData((void *) indices.data(), _transfer_command_pool, _transfer_queue);
        _uniform_buffer = std::unique_ptr<Buffer::UniformBuffer>(
            new Buffer::UniformBuffer(
                _device,
                _physical_device,
                sizeof(Data::Uniform),
                MAX_FRAMES_IN_FLIGHT
            )
        );

        // create swapchain + store extra swap data
        int width, height;
        glfwGetFramebufferSize(main_window._handle, &width, &height);

        Swapchain swapchain_cl(_device, _physical_device, _main_surface, (uint32_t) width, (uint32_t) height);
        _main_swapchain = swapchain_cl.GetHandle();
        _main_swapchain_images = swapchain_cl.GetImages();
        _main_swapchain_image_views = swapchain_cl.CreateImageViews();
        _main_swapchain_image_format = swapchain_cl.GetImageFormat();
        _main_swapchain_extent = swapchain_cl.GetExtent();

        // create graphics pipeline
        _graphics_pipeline = std::unique_ptr<GraphicsPipeline>(new GraphicsPipeline(_device, swapchain_cl, MAX_FRAMES_IN_FLIGHT, *_uniform_buffer));

        // create framebuffers
        _CreateFramebuffers(_device);

        // create synchronisation primitives
        _CreateSynchronisationObjects(_device);
    }

    void Renderer::Destroy() {
        Utils::Note("Destroying renderer");

#       ifdef DEBUG
            vkDestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
#       endif

        for (VkFramebuffer fb : _main_swapchain_framebuffers) {
            vkDestroyFramebuffer(_device, fb, nullptr);
        }
        for (VkImageView view : _main_swapchain_image_views) {
            vkDestroyImageView(_device, view, nullptr);
        }
        vkDestroySwapchainKHR(_device, _main_swapchain, nullptr);

        _graphics_pipeline->Destroy();

        // TODO: stop hardcoding in this file
        _vertex_buffer->Destroy();
        _index_buffer->Destroy();
        _uniform_buffer->Destroy();

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(_device, _image_available_semaphores[i], nullptr);
            vkDestroySemaphore(_device, _render_complete_sempahores[i], nullptr);
            vkDestroyFence(_device, _in_flight_fences[i], nullptr);
        }

        vkDestroyCommandPool(_device, _draw_command_pool, nullptr);
        vkDestroyCommandPool(_device, _transfer_command_pool, nullptr);

        vkDestroyDevice(_device, nullptr);
        vkDestroySurfaceKHR(_instance, _main_surface, nullptr);
        vkDestroyInstance(_instance, nullptr);
    }

    void Renderer::Draw() {
        // wait for and reset fence indicating this frame can be rendered
        vkWaitForFences(_device, 1, &_in_flight_fences[_current_frame], VK_TRUE, UINT64_MAX);

        // get image to render to
        uint32_t image_index;
        VkResult image_aquire_result =
            vkAcquireNextImageKHR(_device, _main_swapchain, UINT64_MAX, _image_available_semaphores[_current_frame], VK_NULL_HANDLE, &image_index);

        // check if the swapchain has become incompatible with the surface (e.g. after resize): VK_ERROR_OUT_OF_DATE_KHR
        if (image_aquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
            _RecreateSwapchain();
            return;
        }
        // VK_SUBOPTIMAL_KHR: swapchain can still be used but properties are not exactly matched.
        else if (image_aquire_result != VK_SUCCESS && image_aquire_result != VK_SUBOPTIMAL_KHR) {
            Utils::Error("Failed to acquire swapchain image for rendering");
        }

        // reset fence if we are submitting work
        vkResetFences(_device, 1, &_in_flight_fences[_current_frame]);

        // record draw operations to the command buffer
        vkResetCommandBuffer(_draw_command_buffers[_current_frame], 0);
        _RecordDrawCommandBuffer(image_index);

        _UpdateUniformBuffer(*_uniform_buffer, _current_frame, _main_swapchain_extent);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // wait for _image_available_semaphore before writing to the colour attachment.
        VkSemaphore wait_semaphores[] = { _image_available_semaphores[_current_frame] };
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &_draw_command_buffers[_current_frame];

        // signal render complete semaphore when command buffer(s) are complete
        VkSemaphore signal_semaphores[] = { _render_complete_sempahores[_current_frame] };
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;

        // _in_flight_fence signalled when command buffer(s) are complete
        if (vkQueueSubmit(_graphics_queue, 1, &submit_info, _in_flight_fences[_current_frame])) {
            Utils::Error("Failed to submit Vulkan draw operations to GPU");
        }

        // submit render result back to swapchain to eventually present it
        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // wait for render completeness
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        // specify swapchains to present to and the index of the image for each swapchain
        VkSwapchainKHR swapchains[] = { _main_swapchain };
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapchains;
        present_info.pImageIndices = &image_index;

        // submit present request to the presentation queue
        VkResult queue_present_result = vkQueuePresentKHR(_present_queue, &present_info);

        if (queue_present_result == VK_ERROR_OUT_OF_DATE_KHR || queue_present_result == VK_SUBOPTIMAL_KHR || _main_window._framebuffer_resized) {
            _main_window._framebuffer_resized = false;
            _RecreateSwapchain();
        } else if (queue_present_result != VK_SUCCESS) {
            Utils::Error("Failed to submit request to present swapchain image");
        }

        _current_frame = (_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::DeviceWaitIdle() {
        vkDeviceWaitIdle(_device);
    }
}
