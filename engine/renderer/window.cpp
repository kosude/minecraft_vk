/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "window.hpp"

#include "utils/log.hpp"

static bool GLFW_ERRCALLBACK_SET = false;

namespace mcvk::Renderer {
    Window::Window(int wid, int hei, const std::string &name)
        : _width(wid), _height(hei), _name(name) {
        Utils::Info("Creating GLFW window with dims " + std::to_string(wid) + "x" + std::to_string(hei) + ", name \"" + name + "\"");

        _InitGLFWWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(_window);

        glfwTerminate();
    }

    VkSurfaceKHR Window::CreateSurface(VkInstance instance) const {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(instance, _window, nullptr, &surface) != VK_SUCCESS) {
            Utils::Fatal("Failed to create window surface");
            return VK_NULL_HANDLE;
        }

        return surface;
    }

    bool Window::Update() {
        glfwPollEvents();

        return !glfwWindowShouldClose(_window);
    }

    std::vector<const char *> Window::GetGLFWRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        return extensions;
    }

    void Window::_InitGLFWWindow() {
        if (!GLFW_ERRCALLBACK_SET) {
            glfwSetErrorCallback([](int error_code, const char* description) {
                Utils::Fatal("GLFW error code " + std::to_string(error_code) + " - " + description);
            });
            GLFW_ERRCALLBACK_SET = true;
        }

        if (!glfwInit()) {
            Utils::Fatal("Failed to initialise GLFW");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        _window = glfwCreateWindow(_width, _height, _name.c_str(), nullptr, nullptr);
        if (!_window) {
            Utils::Fatal("Failed to create GLFW window object");
            return;
        }
    }
}
