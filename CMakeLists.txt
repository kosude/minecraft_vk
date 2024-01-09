cmake_minimum_required(VERSION 3.13)

project("minecraftvk")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/deps")
set(RESOURCES_DIR "${CMAKE_CURRENT_LIST_DIR}/resources")

set(GAME_TARGET "game")
set(GAME_TARGET_DEFINITIONS)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} "DEBUG")
endif()
if (WIN32)
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} "WIN32")
elseif (APPLE)
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} "APPLE")
elseif (UNIX)
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} "UNIX")
endif()

set(C_CPP_SOURCES
    "game/game.cpp"
    "game/main.cpp"

    "engine/renderer/data/model.cpp"
    "engine/renderer/pipeline/graphics_pipeline.cpp"
    "engine/renderer/pipeline/pipeline_set.cpp"
    "engine/renderer/pipeline/pipeline.cpp"
    "engine/renderer/resource/buffer.cpp"
    "engine/renderer/resource/descriptor.cpp"
    "engine/renderer/resource/image.cpp"
    "engine/renderer/command_buffer.cpp"
    "engine/renderer/device.cpp"
    "engine/renderer/instance_manager.cpp"
    "engine/renderer/renderer.cpp"
    "engine/renderer/shader_set.cpp"
    "engine/renderer/swapchain.cpp"
    "engine/renderer/window.cpp"

    "engine/resource_mgr/image_load.cpp"
    "engine/resource_mgr/resource_mgr.cpp"

    "engine/utils/log.cpp"
)



add_library(mini INTERFACE)
target_include_directories(mini INTERFACE "${DEPS_DIR}/mini/src/")

add_subdirectory("${DEPS_DIR}/tinyobjloader")

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${DEPS_DIR}/glm/")

find_package(Vulkan REQUIRED)
if (WIN32)
    set(VOLK_STATIC_DEFINES ${VOLK_STATIC_DEFINES} VK_USE_PLATFORM_WIN32_KHR)
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} WIN32)
elseif (APPLE)
    set(VOLK_STATIC_DEFINES ${VOLK_STATIC_DEFINES} VK_USE_PLATFORM_METAL_EXT)
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} APPLE)
elseif (UNIX)
    # will likely work on most unix machines
    set(VOLK_STATIC_DEFINES ${VOLK_STATIC_DEFINES} VK_USE_PLATFORM_XLIB_KHR)
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} UNIX)
endif()
add_subdirectory("${DEPS_DIR}/volk")

set(GLFW_BUILD_DOCS OFF)
add_subdirectory("${DEPS_DIR}/glfw")

set(CUTILS_CLIST OFF)
set(CUTILS_CARRAY OFF)
add_subdirectory("${DEPS_DIR}/cutils")

add_library(stb_image "${DEPS_DIR}/stb/stb_image.c")
target_include_directories(stb_image INTERFACE "${DEPS_DIR}/stb/")



add_executable(${GAME_TARGET} ${C_CPP_SOURCES})
target_link_libraries(${GAME_TARGET}
    mini
    tinyobjloader
    glm
    volk
    glfw
    cutils
    stb_image)
target_include_directories(${GAME_TARGET}
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/"
    "${DEPS_DIR}")
target_compile_definitions(${GAME_TARGET}
    PRIVATE
    ${GAME_TARGET_DEFINITIONS})



add_subdirectory("${RESOURCES_DIR}")
