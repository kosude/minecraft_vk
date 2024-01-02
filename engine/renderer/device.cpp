/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "device.hpp"

#include "utils/log.hpp"

#include <volk/volk.h>

#include <set>

namespace mcvk::Renderer {
    Device::Device(const Window &window, const VkInstance &instance, const VkSurfaceKHR &surface)
        : _window{window}, _instance{instance}, _surface(surface) {
        _PickPhysicalDevice();
        _CreateLogicalDevice();
        _CreateCommandPools();
    }

    Device::~Device() {
        vkDestroyCommandPool(_device, _transfer_command_pool, nullptr);
        vkDestroyCommandPool(_device, _graphics_command_pool, nullptr);
        vkDestroyDevice(_device, nullptr);
    }

    uint32_t Device::FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(_physical_device, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((filter & (i << 1)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        Utils::Error("Failed to find suitable memory type");
        return UINT32_MAX;
    }

    VkFormat Device::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(_physical_device, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        Utils::Error("Failed to find supported format");
        return VK_FORMAT_UNDEFINED;
    }

    void Device::_PickPhysicalDevice() {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(_instance, &device_count, nullptr);
        if (device_count == 0) {
            Utils::Fatal("Failed to find a GPU with Vulkan support");
        }
        Utils::Info("Device manager found " + std::to_string(device_count) + " physical device(s).");
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(_instance, &device_count, devices.data());

        for (const auto &device : devices) {
            if (_CheckDeviceSuitable(device)) {
                _physical_device = device;
                break;
            }
        }

        if (_physical_device == VK_NULL_HANDLE) {
            Utils::Fatal("Failed to find a suitable GPU");
        }

        vkGetPhysicalDeviceProperties(_physical_device, &_properties);

        Utils::Info("Using physical device (GPU): \"" + std::string{_properties.deviceName} + "\"");

        Utils::Info(
            "Device manager found queue family indices for chosen physical device \"" + std::string{_properties.deviceName} + "\":\n" +
            "\tGraphics: " + std::to_string(_queue_families.graphics.value()) + "\n" +
            "\tPresent:  " + std::to_string(_queue_families.present.value()) + "\n" +
            "\tCompute:  " + std::to_string(_queue_families.compute.value()) + "\n" +
            "\tTransfer: " + std::to_string(_queue_families.transfer.value())
        );
    }

    void Device::_CreateLogicalDevice() {
        std::vector<VkDeviceQueueCreateInfo> queue_infos;
        std::set<uint32_t> families = {
            _queue_families.graphics.value(),
            _queue_families.present.value(),
            _queue_families.transfer.value() };

        float queue_priority = 1.0f;
        for (uint32_t fam : families) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = fam;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queue_priority;
            queue_infos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures features = _GetRequiredDeviceFeatures();

        VkDeviceCreateInfo device_info{};
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        device_info.pEnabledFeatures = &features;
        device_info.enabledExtensionCount = static_cast<uint32_t>(_extensions.size());
        device_info.ppEnabledExtensionNames = _extensions.data();
        // deprecated and ignored
        device_info.enabledLayerCount = 0;
        device_info.ppEnabledLayerNames = nullptr;

        device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
        device_info.pQueueCreateInfos = queue_infos.data();

        if (vkCreateDevice(_physical_device, &device_info, nullptr, &_device) != VK_SUCCESS) {
            Utils::Fatal("Failed to create logical device");
            return;
        }

        volkLoadDevice(_device);

        // get queue handles
        vkGetDeviceQueue(_device, _queue_families.graphics.value(), 0, &_graphics_queue);
        vkGetDeviceQueue(_device, _queue_families.present.value(), 0, &_present_queue);
        vkGetDeviceQueue(_device, _queue_families.transfer.value(), 0, &_transfer_queue);
    }

    void Device::_CreateCommandPools() {
        VkCommandPoolCreateInfo graphics_info{};
        graphics_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        graphics_info.queueFamilyIndex = _queue_families.graphics.value();
        graphics_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(_device, &graphics_info, nullptr, &_graphics_command_pool) != VK_SUCCESS) {
            Utils::Fatal("Failed to create command pool for graphics operations");
        }

        VkCommandPoolCreateInfo transfer_info{};
        transfer_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        transfer_info.queueFamilyIndex = _queue_families.transfer.value();
        transfer_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        if (vkCreateCommandPool(_device, &transfer_info, nullptr, &_transfer_command_pool) != VK_SUCCESS) {
            Utils::Fatal("Failed to create command pool for memory transfer operations");
        }
    }

    bool Device::_CheckDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = _FindQueueFamilies(device);

        bool exts_supported = _CheckExtensionSupport(device);

        bool swap_chain_adequate = false;
        if (exts_supported) {
            SwapChainSupportDetails swap_chain_support = _QuerySwapChainSupport(device);
            swap_chain_adequate = !swap_chain_support.surface_formats.empty() && !swap_chain_support.present_modes.empty();
        }

        bool features_supported = true;

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);
        VkPhysicalDeviceFeatures reqfeatures = _GetRequiredDeviceFeatures();

        // TODO: NGL I was COOKING when I wrote this!!! I feel like the crazy bearded C programmers from the 70s just smiled down on me (even though
        // this solution is horribly unsafe and assumes no padding but I DONT CARE!)
        // Make sure I implement this in Thallium! (for reference, doing this currently takes ~50 more lines, macros, and badly inefficient code; this
        // fixes all of that!!!!!EEEEEEEEE)
        VkBool32 *features_bools = reinterpret_cast<VkBool32 *>(&features);
        VkBool32 *reqfeatures_bools = reinterpret_cast<VkBool32 *>(&reqfeatures);
        uint32_t features_count = static_cast<uint32_t>(sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
        for (uint32_t i = 0; i < features_count; i++) {
            if (reqfeatures_bools[i] && !features_bools[i]) {
                features_supported = false;
                break;
            }
        }

        // store family indices for later
        _queue_families = indices;

        return
            indices.isComplete() && // necessary queue families were found
            exts_supported &&       // required extensions are supported
            swap_chain_adequate &&  // swap chain support is good
            features_supported;     // required devices features are supported
    }

    QueueFamilyIndices Device::_FindQueueFamilies(VkPhysicalDevice device) const {
        QueueFamilyIndices indices{};

        uint32_t family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);

        std::vector<VkQueueFamilyProperties> families(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, families.data());

        int i = 0;
        uint8_t min_transfer_score = 255;
        for (const auto &fam : families) {
            uint8_t cur_transfer_score = 0;

            // if graphics queue
            if (fam.queueCount > 0 && fam.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics = i;

                cur_transfer_score++;
            }

            // if present support
            VkBool32 supports_present = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &supports_present);
            if (fam.queueCount > 0 && supports_present) {
                indices.present = i;

                cur_transfer_score++;
            }

            // if compute queue
            if (fam.queueCount > 0 && fam.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                indices.compute = i;

                cur_transfer_score++;
            }

            // if transfer queue
            if (fam.queueCount > 0 && fam.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                // try to find a dedicated transfer queue if possible
                if (cur_transfer_score < min_transfer_score) {
                    min_transfer_score = cur_transfer_score;
                    indices.transfer = i;
                }
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    bool Device::_CheckExtensionSupport(VkPhysicalDevice device) const {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> available(extension_count);
        vkEnumerateDeviceExtensionProperties(
            device,
            nullptr,
            &extension_count,
            available.data());

        std::set<std::string> required(_extensions.begin(), _extensions.end());

        for (const auto &extension : available) {
            required.erase(extension.extensionName);
        }

        return required.empty();
    }

    SwapChainSupportDetails Device::_QuerySwapChainSupport(VkPhysicalDevice device) const {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &format_count, nullptr);

        if (format_count != 0) {
            details.surface_formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &format_count, details.surface_formats.data());
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &present_mode_count, nullptr);

        if (present_mode_count != 0) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                _surface,
                &present_mode_count,
                details.present_modes.data());
        }
        return details;
    }

    VkPhysicalDeviceFeatures Device::_GetRequiredDeviceFeatures() const {
        VkPhysicalDeviceFeatures features{};

        features.fillModeNonSolid = true;

        return features;
    }
}
