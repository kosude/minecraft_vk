/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#define VK_NO_PROTOTYPES
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

namespace mcvk::Renderer {
    class Window {
    public:
        Window(int wid, int hei, const std::string &name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        bool Update();

        inline VkExtent2D GetExtent() const { return { static_cast<uint32_t>(_width), static_cast<uint32_t>(_height) }; }
        inline const bool &WasResized() const { return _framebuffer_resized; }
        inline void CompleteResize() { _framebuffer_resized = false; }
        float GetAspectRatio() const;
        static std::vector<const char *> GetGLFWRequiredExtensions();

        VkSurfaceKHR CreateSurface(VkInstance instance) const;

    private:
        static void _FramebufferResizeCallback(GLFWwindow *window, int width, int height);

        void _InitGLFWWindow();

        GLFWwindow *_window;

        int _width;
        int _height;
        bool _framebuffer_resized = false;

        const std::string _name;
    };
}
