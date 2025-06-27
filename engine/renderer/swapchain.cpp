/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "swapchain.hpp"

#include "utils/log.hpp"

#include <volk/volk.h>

#include <array>
#include <limits>

namespace mcvk::Renderer {
    Swapchain::Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent)
        : _device{device}, _surface{surface}, _window_extent{window_extent} {
        _Init();
    }

    Swapchain::Swapchain(const Device &device, const VkSurfaceKHR &surface, VkExtent2D window_extent, std::unique_ptr<Swapchain> &old)
        : _device{device}, _surface{surface}, _window_extent{window_extent}, _old_swapchain{std::move(old)} {
        _Init();
    }

    Swapchain::~Swapchain() {
        for (VkSemaphore &s : _draw_complete_sems) {
            vkDestroySemaphore(_device.GetDevice(), s, nullptr);
        }
        vkDestroySemaphore(_device.GetDevice(), _image_available_sem, nullptr);
        vkDestroyFence(_device.GetDevice(), _frame_fence, nullptr);

        // explicitly free swapchain image objects as they are child objects of the swapchain (created as part of vkCreateSwapchainKHR)
        for (auto &imgptr : _swapchain_images) {
            imgptr.reset();
        }

        if (_swapchain) {
            vkDestroySwapchainKHR(_device.GetDevice(), _swapchain, nullptr);
            _swapchain = nullptr;
        }

        for (auto framebuffer : _swapchain_framebuffers) {
            vkDestroyFramebuffer(_device.GetDevice(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(_device.GetDevice(), _render_pass, nullptr);
    }

    VkResult Swapchain::AcquireNextImage(uint32_t *const image_index) {
        vkWaitForFences(_device.GetDevice(), 1, &_frame_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

        VkResult result = vkAcquireNextImageKHR(
            _device.GetDevice(),
            _swapchain,
            std::numeric_limits<uint64_t>::max(),
            _image_available_sem,
            VK_NULL_HANDLE,
            image_index);

        return result;
    }

    VkResult Swapchain::SubmitCommandBuffers(const std::vector<VkCommandBuffer> &cmdbufs, uint32_t *const image_index) {
        VkFence frame_fence = _frame_fence;
        vkWaitForFences(_device.GetDevice(), 1, &frame_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(_device.GetDevice(), 1, &frame_fence);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &_image_available_sem;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &_draw_complete_sems[*image_index];

        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit_info.pWaitDstStageMask = wait_stages;

        submit_info.commandBufferCount = static_cast<uint32_t>(cmdbufs.size());
        submit_info.pCommandBuffers = cmdbufs.data();

        if (vkQueueSubmit(_device.GetGraphicsQueue(), 1, &submit_info, _frame_fence) != VK_SUCCESS) {
            Utils::Fatal("Failed to submit draw command buffer operations to graphics queue");
        }

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // wait for draw_complete semaphore (i.e. wait for GPU rendering to be complete)
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &_draw_complete_sems[*image_index];

        present_info.swapchainCount = 1;
        present_info.pSwapchains = &_swapchain;

        present_info.pImageIndices = image_index;

        return vkQueuePresentKHR(_device.GetPresentQueue(), &present_info);
    }

    void Swapchain::_Init() {
        _CreateSwapchain();
        _ManageSwapchainImages();
        _CreateDepthImages();
        _CreateRenderPass();
        _CreateFramebuffers();
        _CreateSynchronisationPrims();
    }

    void Swapchain::_CreateSwapchain() {
        SwapChainSupportDetails support = _device.SwapchainSupportDetails();

        VkSurfaceFormatKHR surface_format = _ChooseSurfaceFormat(support.surface_formats);
        VkPresentModeKHR present_mode = _ChoosePresentMode(support.present_modes);
        VkExtent2D extent = _ChooseExtent(support.capabilities);

        uint32_t image_count = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
            // clamp to maximum image count if applicable
            image_count = support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = _surface;

        info.minImageCount = image_count;
        info.imageFormat = surface_format.format;
        info.imageColorSpace = surface_format.colorSpace;
        info.imageExtent = extent;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = _device.FindQueueFamilyIndices();
        uint32_t family_indices[] = { indices.graphics.value(), indices.present.value() };

        if (indices.graphics != indices.present) {
            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            // images will be shared between two separate queue families so we specify their indices
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = family_indices;
        } else {
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        info.preTransform = support.capabilities.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        info.presentMode = present_mode;
        info.clipped = VK_TRUE;

        info.oldSwapchain = (_old_swapchain) ? _old_swapchain->_swapchain : VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(_device.GetDevice(), &info, nullptr, &_swapchain)) {
            Utils::Fatal("Failed to create swap chain");
        }

        _swapchain_image_format = surface_format.format;
        _swapchain_extent = extent;

        // destroy old swapchain class - cleans associated memory
        _old_swapchain.reset();
    }

    void Swapchain::_ManageSwapchainImages() {
        uint32_t img_count;

        vkGetSwapchainImagesKHR(_device.GetDevice(), _swapchain, &img_count, nullptr);
        _swapchain_images.resize(img_count);

        VkImage sc_images[img_count];
        vkGetSwapchainImagesKHR(_device.GetDevice(), _swapchain, &img_count, sc_images);

        // create image objects for each image, also creating image views
        auto image_config = Image::Config::Defaults(_swapchain_extent, _swapchain_image_format);
        image_config.image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        for (uint32_t i = 0; i < _swapchain_images.size(); i++) {
            _swapchain_images[i] = std::make_unique<Image>(_device, image_config, sc_images[i]);
        }
    }

    void Swapchain::_CreateDepthImages() {
        _depth_image_format = _FindDepthImageFormat();

        _depth_images.resize(_swapchain_images.size());

        for (auto &dimg : _depth_images) {
            auto config = Image::Config::Defaults(_swapchain_extent, _depth_image_format);
            config.image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            config.view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            dimg = std::make_unique<Image>(_device, config);
        }
    }

    void Swapchain::_CreateRenderPass() {
        VkAttachmentDescription colour_attachment{};
        colour_attachment.format = _swapchain_image_format;
        colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colour_attachment_ref{};
        colour_attachment_ref.attachment = 0;
        colour_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depth_attachment{};
        depth_attachment.format = _depth_image_format;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colour_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstSubpass = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colour_attachment, depth_attachment };

        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        if (vkCreateRenderPass(_device.GetDevice(), &render_pass_info, nullptr, &_render_pass) != VK_SUCCESS) {
            Utils::Fatal("Failed to create render pass");
        }
    }

    void Swapchain::_CreateFramebuffers() {
        _swapchain_framebuffers.resize(_swapchain_images.size());

        for (size_t i = 0; i < _swapchain_framebuffers.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                _swapchain_images[i]->GetImageView(),
                _depth_images[i]->GetImageView() };

            VkFramebufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.renderPass = _render_pass;
            info.attachmentCount = static_cast<uint32_t>(attachments.size());
            info.pAttachments = attachments.data();
            info.width = _swapchain_extent.width;
            info.height = _swapchain_extent.height;
            info.layers = 1;

            if (vkCreateFramebuffer(_device.GetDevice(), &info, nullptr, &_swapchain_framebuffers[i]) != VK_SUCCESS) {
                Utils::Fatal("Failed to create framebuffer for image attachment index " + std::to_string(i));
            }
        }
    }

    void Swapchain::_CreateSynchronisationPrims() {
        _draw_complete_sems.resize(_swapchain_images.size());

        VkSemaphoreCreateInfo sem_info{};
        sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        // create one draw_complete semaphore per swapchain image
        for (VkSemaphore &s : _draw_complete_sems) {
            if (vkCreateSemaphore(_device.GetDevice(), &sem_info, nullptr, &s) != VK_SUCCESS) {
                Utils::Fatal("Failed to create synchronisation primitives");
            }
        }

        if (vkCreateSemaphore(_device.GetDevice(), &sem_info, nullptr, &_image_available_sem) != VK_SUCCESS ||
            vkCreateFence(_device.GetDevice(), &fence_info, nullptr, &_frame_fence) != VK_SUCCESS) {
            Utils::Fatal("Failed to create synchronisation primitives");
        }
    }

    VkSurfaceFormatKHR Swapchain::_ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &candidates) {
        const VkFormat ideal_format = VK_FORMAT_B8G8R8A8_SRGB;
        const VkColorSpaceKHR ideal_colourspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        const VkSurfaceFormatKHR fallback = candidates[0];

        for (const auto &candidate : candidates) {
            if (candidate.format == ideal_format && candidate.colorSpace == ideal_colourspace) {
                return candidate;
            }
        }

        return fallback;
    }

    VkPresentModeKHR Swapchain::_ChoosePresentMode(const std::vector<VkPresentModeKHR> &candidates) {
        const VkPresentModeKHR ideal = VK_PRESENT_MODE_MAILBOX_KHR;

        const VkPresentModeKHR fallback = VK_PRESENT_MODE_FIFO_KHR; // guaranteed to be available

        for (const auto &candidate : candidates) {
            if (candidate == ideal) {
                return candidate;
            }
        }

        return fallback;
    }

    VkExtent2D Swapchain::_ChooseExtent(const VkSurfaceCapabilitiesKHR &caps) {
        if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return caps.currentExtent;
        } else {
            VkExtent2D extent = _window_extent;
            extent.width = std::max(
                caps.minImageExtent.width,
                std::min(caps.maxImageExtent.width, extent.width));
            extent.height = std::max(
                caps.minImageExtent.height,
                std::min(caps.maxImageExtent.height, extent.height));

            return extent;
        }
    }

    VkFormat Swapchain::_FindDepthImageFormat() {
        return _device.FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}
