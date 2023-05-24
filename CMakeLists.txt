cmake_minimum_required(VERSION 3.10)

project("minecraftvk")

set(GAME_TARGET "game")
set(GAME_TARGET_DEFINITIONS)

set(SOURCES
    "game/game.cpp"                         "game/game.hpp"
    "game/main.cpp"

    "renderer/buffer/buffer.cpp"            "renderer/buffer/buffer.hpp"

    "renderer/data/vertex.cpp"              "renderer/data/vertex.hpp"

    "renderer/device_manager.cpp"           "renderer/device_manager.hpp"
    "renderer/extension_manager.cpp"        "renderer/extension_manager.hpp"
    "renderer/graphics_pipeline.cpp"        "renderer/graphics_pipeline.hpp"
    "renderer/renderer.cpp"                 "renderer/renderer.hpp"
    "renderer/swap_chain.cpp"               "renderer/swap_chain.hpp"
    "renderer/window.cpp"                   "renderer/window.hpp"

    "utils/log.cpp"                         "utils/log.hpp"
    "utils/file_io.cpp"                     "utils/file_io.hpp"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${GAME_TARGET} ${SOURCES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} "DEBUG")
endif()

set(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/deps")
target_include_directories(${GAME_TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${DEPS_DIR})

add_subdirectory("${DEPS_DIR}/cutils")
target_link_libraries(${GAME_TARGET} clist cio)

add_subdirectory("${DEPS_DIR}/glfw")
target_link_libraries(${GAME_TARGET} glfw)

find_package(Vulkan REQUIRED)
target_include_directories(${GAME_TARGET} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${GAME_TARGET} ${Vulkan_LIBRARIES})

if (WIN32)
    set(VOLK_STATIC_DEFINES ${VOLK_STATIC_DEFINES} VK_USE_PLATFORM_WIN32_KHR)
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} WIN32)
elseif (APPLE)
    set(VOLK_STATIC_DEFINES ${VOLK_STATIC_DEFINES} VK_USE_PLATFORM_MACOS_MVK)
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} APPLE)
elseif (UNIX)
    # will likely work on most unix machines
    set(VOLK_STATIC_DEFINES ${VOLK_STATIC_DEFINES} VK_USE_PLATFORM_XLIB_KHR)
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} UNIX)
endif()

add_subdirectory("${DEPS_DIR}/volk")
target_link_libraries(${GAME_TARGET} volk)

target_compile_definitions(${GAME_TARGET} PRIVATE ${GAME_TARGET_DEFINITIONS})

set(SHADER_DIR "${CMAKE_CURRENT_LIST_DIR}/renderer/shaders")
set(SHADER_SOURCES
    "${SHADER_DIR}/main_fragment.glsl"
    "${SHADER_DIR}/main_vertex.glsl"
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/spv/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/spv/"
)

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_FILENAME ${SHADER} NAME)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/spv/${SHADER_FILENAME}.spv"
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -g -o "${CMAKE_CURRENT_BINARY_DIR}/spv/${SHADER_FILENAME}.spv"
        DEPENDS ${SHADER}
        COMMENT "Building SPIR-V object from ${SHADER_FILENAME}"
    )

    list(APPEND SPV_SHADERS "${CMAKE_CURRENT_BINARY_DIR}/spv/${SHADER_FILENAME}.spv")
endforeach()

add_custom_target(shaders_spv ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/spv/" ${SPV_SHADERS})
add_dependencies(${GAME_TARGET} shaders_spv)
