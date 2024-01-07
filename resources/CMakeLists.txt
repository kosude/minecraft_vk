set(RES_SHADER_CONF)
set(RES_GLSL)

file(GLOB RES_SHADER_CONF
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.shader")
file(GLOB RES_GLSL
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/glsl/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/glsl/*.frag")



# shader config files (.shader)
# copied from resources/shaders/ -> build/resources/shaders/

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
    COMMENT ""
)
foreach(SHADER_CONF ${RES_SHADER_CONF})
    get_filename_component(SHADER_CONF_FN ${SHADER_CONF} NAME)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_CONF_FN}"
        COMMAND ${CMAKE_COMMAND} -E copy "${SHADER_CONF}" "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_CONF_FN}"
        DEPENDS ${SHADER_CONF}
        COMMENT "")
    list(APPEND SHADER_CONF_OUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_CONF_FN}")
endforeach()
add_custom_target(
    gameresources_shaderconfs ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shaders/" ${SHADER_CONF_OUT})
add_dependencies(${GAME_TARGET} gameresources_shaderconfs)



# GLSL shader compilations (.vert, .frag, .comp, .tess, etc.)
# generated from resources/shaders/glsl/ -> build/resources/shaders/spv/

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/"
    COMMENT ""
)
foreach(GLSL ${RES_GLSL})
    get_filename_component(GLSL_FN ${GLSL} NAME)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/${GLSL_FN}.spv"
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${GLSL}" -g -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/${GLSL_FN}.spv"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/" ${GLSL}
        COMMENT "")
    list(APPEND GLSL_OUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/${GLSL_FN}.spv")
endforeach()
add_custom_target(
    gameresources_glslshaders ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/" ${GLSL_OUT})
add_dependencies(${GAME_TARGET} gameresources_glslshaders)