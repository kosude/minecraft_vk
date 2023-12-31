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

        VkSurfaceKHR CreateSurface(VkInstance instance) const;

        static std::vector<const char *> GetGLFWRequiredExtensions();

    private:
        void _InitGLFWWindow();

        GLFWwindow *_window;

        const int _width;
        const int _height;

        const std::string _name;
    };
}
