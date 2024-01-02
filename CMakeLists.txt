cmake_minimum_required(VERSION 3.13)

project("minecraftvk")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/deps")
set(SHADER_DIR "${CMAKE_CURRENT_LIST_DIR}/src/renderer/shaders")

set(GAME_TARGET "game")
set(GAME_TARGET_DEFINITIONS)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GAME_TARGET_DEFINITIONS ${GAME_TARGET_DEFINITIONS} "DEBUG")
endif()

set(C_CPP_SOURCES
    "game/game.cpp"
    "game/main.cpp"

    "engine/renderer/buffer/buffer.cpp"
    "engine/renderer/buffer/image.cpp"
    "engine/renderer/data/model.cpp"
    "engine/renderer/pipeline/graphics_pipeline.cpp"
    "engine/renderer/pipeline/pipeline_factory.cpp"
    "engine/renderer/command_buffer.cpp"
    "engine/renderer/device.cpp"
    "engine/renderer/instance_manager.cpp"
    "engine/renderer/renderer.cpp"
    "engine/renderer/shader_set.cpp"
    "engine/renderer/swapchain.cpp"
    "engine/renderer/window.cpp"

    "engine/utils/log.cpp"
)
set(SHADER_SOURCES
    "engine/renderer/shaders/simple_frag.glsl"
    "engine/renderer/shaders/simple_vert.glsl"
)



find_package(Vulkan REQUIRED)
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

set(GLFW_BUILD_DOCS OFF)
add_subdirectory("${DEPS_DIR}/glfw")

set(CUTILS_CLIST OFF)
set(CUTILS_CARRAY OFF)
add_subdirectory("${DEPS_DIR}/cutils")



add_executable(${GAME_TARGET} ${C_CPP_SOURCES})
target_link_libraries(${GAME_TARGET}
    volk
    glfw
    cutils)
target_include_directories(${GAME_TARGET}
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/"
    "${DEPS_DIR}")
target_compile_definitions(${GAME_TARGET}
    PRIVATE
    ${GAME_TARGET_DEFINITIONS})



add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/spv/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/spv/"
)

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_FILENAME ${SHADER} NAME)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/spv/${SHADER_FILENAME}.spv"
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}" -g -o "${CMAKE_CURRENT_BINARY_DIR}/spv/${SHADER_FILENAME}.spv"
        DEPENDS ${SHADER}
        COMMENT "Building SPIR-V object from ${SHADER_FILENAME}"
    )

    list(APPEND SPV_SHADERS "${CMAKE_CURRENT_BINARY_DIR}/spv/${SHADER_FILENAME}.spv")
endforeach()

add_custom_target(shaders_spv ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/spv/" ${SPV_SHADERS})
add_dependencies(${GAME_TARGET} shaders_spv)
