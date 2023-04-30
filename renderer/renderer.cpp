/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <iostream>
#include <set>

#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#include "renderer/window.hpp"

#include "renderer/device_manager.hpp"
#include "renderer/extension_manager.hpp"
#include "renderer/swap_chain.hpp"
#include "utils/log.hpp"

#include "renderer.hpp"

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
            VKGame::Utils::Warn(type_str + ": " + data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            VKGame::Utils::Error(type_str + ": " + data->pMessage, false);
            break;
        default:
            VKGame::Utils::Log(type_str + ": " + data->pMessage);
            break;
    }

    return VK_FALSE;
}

namespace VKGame::Renderer {
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

        instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

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
            queue_family_info.present_family_index.value()
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
    }

    Renderer::Renderer(const Window &main_window) {
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
        _graphics_pipeline = std::unique_ptr<GraphicsPipeline>(new GraphicsPipeline(_device, swapchain_cl));
    }

    void Renderer::Destroy() {
        Utils::Note("Destroying renderer");

#       ifdef DEBUG
            vkDestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
#       endif

        for (VkImageView view : _main_swapchain_image_views) {
            vkDestroyImageView(_device, view, nullptr);
        }

        _graphics_pipeline->Destroy();

        vkDestroySwapchainKHR(_device, _main_swapchain, nullptr);

        vkDestroyDevice(_device, nullptr);
        vkDestroySurfaceKHR(_instance, _main_surface, nullptr);
        vkDestroyInstance(_instance, nullptr);
    }
}
