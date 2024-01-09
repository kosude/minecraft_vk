set(RES_MODEL)
set(RES_PIPELINE_CONF)
set(RES_SHADER_CONF)
set(RES_GLSL)

file(GLOB RES_MODEL
    "${CMAKE_CURRENT_SOURCE_DIR}/models/*.model"
    "${CMAKE_CURRENT_SOURCE_DIR}/models/*.obj")
file(GLOB RES_PIPELINE_CONF
    "${CMAKE_CURRENT_SOURCE_DIR}/pipelines/*.pipeline")
file(GLOB RES_SHADER_CONF
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.shader")
file(GLOB RES_GLSL
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/glsl/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/glsl/*.frag")



# model object files (.obj)
# copied from resources/models/ -> build/resources/models/

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/models/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/models/"
    COMMENT ""
)
foreach(MODEL ${RES_MODEL})
    get_filename_component(MODEL_FN ${MODEL} NAME)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/models/${MODEL_FN}"
        COMMAND ${CMAKE_COMMAND} -E copy "${MODEL}" "${CMAKE_CURRENT_BINARY_DIR}/models/${MODEL_FN}"
        DEPENDS ${MODEL}
        COMMENT "")
    list(APPEND MODEL_OUT "${CMAKE_CURRENT_BINARY_DIR}/models/${MODEL_FN}")
endforeach()
add_custom_target(
    gameresources_models ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/models/" ${MODEL_OUT})
add_dependencies(${GAME_TARGET} gameresources_models)



# pipeline config files (.pipeline)
# copied from resources/pipelines/ -> build/resources/pipelines/

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pipelines/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/pipelines/"
    COMMENT ""
)
foreach(PIPELINE_CONF ${RES_PIPELINE_CONF})
    get_filename_component(PIPELINE_CONF_FN ${PIPELINE_CONF} NAME)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pipelines/${PIPELINE_CONF_FN}"
        COMMAND ${CMAKE_COMMAND} -E copy "${PIPELINE_CONF}" "${CMAKE_CURRENT_BINARY_DIR}/pipelines/${PIPELINE_CONF_FN}"
        DEPENDS ${PIPELINE_CONF}
        COMMENT "")
    list(APPEND PIPELINE_CONF_OUT "${CMAKE_CURRENT_BINARY_DIR}/pipelines/${PIPELINE_CONF_FN}")
endforeach()
add_custom_target(
    gameresources_pipelineconfs ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pipelines/" ${PIPELINE_CONF_OUT})
add_dependencies(${GAME_TARGET} gameresources_pipelineconfs)



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
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GLSLC_CMD_O_FLAG "-O0")
else()
    set(GLSLC_CMD_O_FLAG "-O")
endif()
foreach(GLSL ${RES_GLSL})
    get_filename_component(GLSL_FN ${GLSL} NAME)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/${GLSL_FN}.spv"
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${GLSL}" -g ${GLSLC_CMD_O_FLAG} -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/${GLSL_FN}.spv"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/" ${GLSL}
        COMMENT "")
    list(APPEND GLSL_OUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/${GLSL_FN}.spv")
endforeach()
add_custom_target(
    gameresources_glslshaders ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shaders/spv/" ${GLSL_OUT})
add_dependencies(${GAME_TARGET} gameresources_glslshaders)
