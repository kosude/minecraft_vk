/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <map>

#include "utils/log.hpp"
#include "renderer/extension_manager.hpp"
#include "renderer/swap_chain.hpp"

#include "device_manager.hpp"

namespace VKGame::Renderer {
    DeviceQueueFamilyInfo DeviceManager::GetDeviceQueueFamilyInfo(const VkPhysicalDevice &physical_device, const VkSurfaceKHR &surface) {
        DeviceQueueFamilyInfo info = {};

        // get queue families
        unsigned int queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

        for (uint32_t i = 0; i < queue_family_count; i++) {
            // if graphics queue
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                info.graphics_family_index = i;
            }

            // if present support
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
            if (present_support) {
                info.present_family_index = i;
            }
        }

        return info;
    }

    bool DeviceManager::IsDeviceSuitable(const VkPhysicalDevice &physical_device, const VkSurfaceKHR &surface) {
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(physical_device, &device_properties);
        vkGetPhysicalDeviceFeatures(physical_device, &device_features);

        // check device extensions
        std::vector<const char *> required_exts = ExtensionManager::GetRequiredDeviceExtensions();
        if (!ExtensionManager::CheckDeviceExtensionNames(required_exts, ExtensionImportance::Optional, physical_device)) {
            Utils::Warn(std::string{"Device "} + device_properties.deviceName + " doesn't meet requirements: missing extension(s)");
            return false;
        }

        // check device features
        if (!ExtensionManager::CheckDeviceFeatures(device_features)) {
            Utils::Warn(std::string{"Device "} + device_properties.deviceName + " doesn't meet requirements: missing feature(s)");
            return false;
        }

        // check queue families
        DeviceQueueFamilyInfo queue_family_info = GetDeviceQueueFamilyInfo(physical_device, surface);
        if (!queue_family_info.graphics_family_index.has_value()) {
            Utils::Warn(std::string{"Device "} + device_properties.deviceName + " doesn't meet requirements: missing graphics queue family");
            return false;
        }
        if (!queue_family_info.present_family_index.has_value()) {
            Utils::Warn(std::string{"Device "} + device_properties.deviceName + " doesn't meet requirements: missing presentation queue family");
            return false;
        }

        // check swapchain support
        SwapchainSupportInfo swapchain_caps = Swapchain::GetSupportInfo(physical_device, surface);
        if (swapchain_caps.formats.empty() || swapchain_caps.present_modes.empty()) {
            return false;
        }

        // device is confirmed suitable at this point...
        return true;
    }

    uint64_t DeviceManager::ScorePhysicalDevice(const VkPhysicalDevice &physical_device) {
        uint64_t score = 0;

        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceProperties(physical_device, &device_properties);
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

        // increase score for typically discrete GPUs
        // (significant performance advantage over integrated ofc)
        if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 2000;
        }

        // get memory size
        for (unsigned int i = 0; i < memory_properties.memoryHeapCount; i++) {
            if (memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                // device local heap: .size will be size of VRAM in bytes.
                // we add this size to the score (converted to smaller number)
                score += memory_properties.memoryHeaps[i].size / (1024 * 1024);
            }
        }

        return score;
    }

    VkPhysicalDevice DeviceManager::GetOptimalPhysicalDevice(std::vector<VkPhysicalDevice> choices, const VkSurfaceKHR &surface) {
        // remove unsuitable devices from vector
        for (auto it = choices.begin(); it != choices.end();) {
            if (!IsDeviceSuitable(*it, surface)) {
                it = choices.erase(it);
            } else {
                it++;
            }
        }

        // no physical device determined suitable
        if (choices.size() <= 0) {
            Utils::Error("Could not find a suitable or supported GPU");
        }

        // ordered map to sort devices by score
        std::multimap<uint64_t, VkPhysicalDevice> ordered_candidates;
        for (const VkPhysicalDevice &d : choices) {
            uint64_t score = ScorePhysicalDevice(d);
            ordered_candidates.insert(std::make_pair(score, d));
        }

        // return best physical device
        return ordered_candidates.rbegin()->second;
    }
}
