cmake_minimum_required(VERSION 3.10)

project("VKGame")

set(SOURCES
    "game/game.cpp"                         "game/game.hpp"
    "game/main.cpp"

    "renderer/device_manager.cpp"           "renderer/device_manager.hpp"
    "renderer/extension_manager.cpp"        "renderer/extension_manager.hpp"
    "renderer/renderer.cpp"                 "renderer/renderer.hpp"
    "renderer/swap_chain.cpp"               "renderer/swap_chain.hpp"
    "renderer/window.cpp"                   "renderer/window.hpp"

    "utils/log.cpp"                         "utils/log.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE "DEBUG")
endif()

set(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/deps")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${DEPS_DIR})

add_subdirectory("${DEPS_DIR}/cutils")
target_link_libraries(${PROJECT_NAME} clist cio)

add_subdirectory("${DEPS_DIR}/glfw")
target_link_libraries(${PROJECT_NAME} glfw)

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

if (WIN32)
    set(VOLK_STATIC_DEFINES ${VOLK_STATIC_DEFINES} VK_USE_PLATFORM_WIN32_KHR)
elseif (APPLE)
    set(VOLK_STATIC_DEFINES ${VOLK_STATIC_DEFINES} VK_USE_PLATFORM_MACOS_MVK)
elseif (UNIX)
    # will likely work on most unix machines
    set(VOLK_STATIC_DEFINES ${VOLK_STATIC_DEFINES} VK_USE_PLATFORM_XLIB_KHR)
endif()

add_subdirectory("${DEPS_DIR}/volk")
target_link_libraries(${PROJECT_NAME} volk)
