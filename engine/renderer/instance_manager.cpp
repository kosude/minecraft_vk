/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "instance_manager.hpp"

#include "utils/log.hpp"

#include <cstring>
#include <sstream>
#include <unordered_set>

static VKAPI_ATTR VkBool32 VKAPI_CALL __DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *data, void *user);

namespace mcvk::Renderer {
    InstanceManager::InstanceManager(const mcvk::Renderer::Window &window, VkSurfaceKHR &surface) {
        _CreateInstance();
        _CreateDebugMessenger();

        surface = _CreateSurface(window);
    }

    InstanceManager::~InstanceManager() {
#       ifdef DEBUG
            vkDestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
#       endif

        vkDestroyInstance(_instance, nullptr);
    }

    void InstanceManager::_CreateInstance() {
        if (volkInitialize()) {
            Utils::Fatal("Failed to initialise Vulkan loader");
            return;
        }

#       ifdef DEBUG
            if (!_CheckValidationLayerSupport()) {
                Utils::Fatal("Validation layers requested, but not available");
                return;
            }
#       endif

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Minecraft Vulkan";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instance_info{};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &app_info;

        std::vector<const char *> extensions = _GetRequiredExtensions();
        if (!_CheckExtensionsSupport(extensions)) {
            Utils::Fatal("Missing required instance extension(s)");
            return;
        }
        instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instance_info.ppEnabledExtensionNames = extensions.data();

        // set up and use debug messenger if using validation layers
        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info;
#       ifdef DEBUG
            instance_info.enabledLayerCount = static_cast<uint32_t>(_validation_layers.size());
            instance_info.ppEnabledLayerNames = _validation_layers.data();

            _PopulateDebugMessengerCreateInfo(debug_messenger_info);
            instance_info.pNext = &debug_messenger_info;
#       else
            instance_info.enabledLayerCount = 0;
            instance_info.ppEnabledLayerNames = nullptr;
#       endif

        if (vkCreateInstance(&instance_info, nullptr, &_instance) != VK_SUCCESS) {
            Utils::Fatal("Failed to create Vulkan instance");
            return;
        }

        volkLoadInstanceOnly(_instance);
    }

    void InstanceManager::_CreateDebugMessenger() {
#       ifdef DEBUG
            Utils::Info("Compiled in DEBUG configuration; creating Vulkan debug messenger...");

            VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info;
            _PopulateDebugMessengerCreateInfo(debug_messenger_info);

            if (vkCreateDebugUtilsMessengerEXT(_instance, &debug_messenger_info, nullptr, &_debug_messenger)) {
                Utils::Error("Failed to create Vulkan debug messenger");
            }
#       endif
        return;
    }

    VkSurfaceKHR InstanceManager::_CreateSurface(const Window &window) {
        return window.CreateSurface(_instance);
    }

    std::vector<const char *> InstanceManager::_GetRequiredExtensions() {
        std::vector<const char *> extensions = Window::GetGLFWRequiredExtensions();

#       ifdef DEBUG
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#       endif

        return extensions;
    }

    bool InstanceManager::_CheckExtensionsSupport(const std::vector<const char *> &required) {
        uint32_t extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

        std::stringstream stream{};

        stream << "Validating required instance extensions..."
            << std::endl << "\tAvailable extensions:";
        std::unordered_set<std::string> available;
        for (const auto &ext : extensions) {
            stream << std::endl << "\t\t" << ext.extensionName;
            available.insert(ext.extensionName);
        }

        stream << std::endl << "\tRequired (enabled) extensions:";
        for (const auto &req : required) {
            stream << std::endl << "\t\t" << req;

            if (available.find(req) == available.end()) {
                return false;
            }
        }

        Utils::Info(stream.str());

        return true;
    }

    bool InstanceManager::_CheckValidationLayerSupport() {
#       ifdef DEBUG
            uint32_t layer_count;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

            std::vector<VkLayerProperties> available(layer_count);
            vkEnumerateInstanceLayerProperties(&layer_count, available.data());

            for (const char *layer_name : _validation_layers) {
                bool found = false;

                for (const auto &props : available) {
                    if (!std::strcmp(layer_name, props.layerName)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }
#       endif

        return true;
    }

    void InstanceManager::_PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info) {
        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = __DebugCallback;
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL __DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *data, void *user) {
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
            mcvk::Utils::Warn(type_str + ": " + data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            mcvk::Utils::Error(type_str + ": " + data->pMessage);
            break;
        default:
            mcvk::Utils::Log(type_str + ": " + data->pMessage);
            break;
    }

    return VK_FALSE;
}
