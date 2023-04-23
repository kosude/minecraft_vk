/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <iostream>

#include "renderer/vk_extension.hpp"
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
    void Renderer::_CreateInstance() {
        VkInstanceCreateInfo instance_create_info = {};

        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = nullptr;

        std::vector<const char *> enabled_layers = VkExtensionManager::GetRequiredLayers();
        VkExtensionManager::CheckLayerNames(enabled_layers, VkExtensionImportance::Required);
        std::vector<const char *> enabled_exts = VkExtensionManager::GetRequiredInstanceExtensions();
        VkExtensionManager::CheckInstanceExtensionNames(enabled_exts, VkExtensionImportance::Required);

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

        if (vkCreateInstance(&instance_create_info, nullptr, &_instance)) {
            Utils::Error("Failed to create Vulkan instance");
        }

        // load vulkan entrypoints + extensions
        volkLoadInstance(_instance);

        // create debug messenger if in debug mode
#       ifdef DEBUG
            if (vkCreateDebugUtilsMessengerEXT(_instance, &debug_messenger_create_info, nullptr, &_debug_messenger)) {
                Utils::Error("Failed to create Vulkan debug messenger");
            }
#       endif
    }

    Renderer::Renderer() {
        Utils::Note("Creating renderer");

        if (volkInitialize()) {
            Utils::Error("Failed to load Vulkan (via volk)");
        }

        _CreateInstance();
    }

    void Renderer::Destroy() {
        Utils::Note("Destroying renderer");

#       ifdef DEBUG
            vkDestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
#       endif

        vkDestroyInstance(_instance, nullptr);
    }
}
