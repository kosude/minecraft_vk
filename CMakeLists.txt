cmake_minimum_required(VERSION 3.10)

project("VKGame")

if (MSVC)
    set(PROJ_COMPILE_OPTIONS
        /W4 /WX
    )
else()
    set(PROJ_COMPILE_OPTIONS
        -Wall -Wextra -pedantic -Werror
    )
endif()

set(SOURCES
    "game/game.cpp"                         "game/game.hpp"
    "game/main.cpp"

    "renderer/renderer.cpp"                 "renderer/renderer.hpp"
    "renderer/window.cpp"                   "renderer/window.hpp"

    "utils/log.cpp"                         "utils/log.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE ${PROJ_COMPILE_OPTIONS})

set(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/deps")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${DEPS_DIR})

add_subdirectory("${DEPS_DIR}/cutils")
target_link_libraries(${PROJECT_NAME} clist cio)

add_subdirectory("${DEPS_DIR}/glfw")
target_link_libraries(${PROJECT_NAME} glfw)

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
