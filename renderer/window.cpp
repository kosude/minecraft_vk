/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include "utils/log.hpp"

#include "window.hpp"

namespace VKGame::Renderer {
    Window::Window(const uint32_t &width, const uint32_t &height, const std::string &title) {
        Utils::Note("Creating window");

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        _handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    }

    Window::~Window() {
        Utils::Note("Destroying window");

        glfwDestroyWindow(_handle);
    }

    bool Window::Update() {
        glfwPollEvents();

        return !glfwWindowShouldClose(_handle);
    }

    void Window::TerminateGLFW() {
        glfwTerminate();
    }
}
