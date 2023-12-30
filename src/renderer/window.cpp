/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "window.hpp"

#include "util/log.hpp"

static bool GLFW_ERRCALLBACK_SET = false;

namespace mcvk::Renderer {
    Window::Window(int wid, int hei, const std::string &name)
        : _width(wid), _height(hei), _name(name) {
        _InitGLFWWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(_window);

        glfwTerminate();
    }

    bool Window::Update() {
        glfwPollEvents();

        return !glfwWindowShouldClose(_window);
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
