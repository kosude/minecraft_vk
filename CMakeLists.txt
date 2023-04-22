cmake_minimum_required(VERSION 3.10)

project("VKGame")

if (MSVC)
    add_compile_options(
        /W4 /WX
    )
else()
    add_compile_options(
        -Wall -Wextra -pedantic -Werror
    )
endif()

set(SOURCES
    "game/main.cpp"

    "utils/log.cpp"
    "utils/log.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

set(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/deps")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${DEPS_DIR})

add_subdirectory("${DEPS_DIR}/cutils")
target_link_libraries(${PROJECT_NAME} clist cio)
