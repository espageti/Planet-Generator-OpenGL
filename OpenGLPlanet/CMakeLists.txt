cmake_minimum_required(VERSION 3.15)
project(OpenGLPlanet LANGUAGES C CXX)

set(OpenGL_ROOT "C:/OpenGL" CACHE PATH "Root directory for OpenGL/ImGui installations")

set(GLAD_C_PATH "${OpenGL_ROOT}/src/glad.c" CACHE FILEPATH "Path to glad.c")

set(SOURCES
    main.cpp
    engine.cpp
    shader.cpp
    shapesettings.cpp
    perlinNoiseFilter.cpp
    planetUI.cpp
    sphere.cpp
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    ${GLAD_C_PATH}
)

add_executable(OpenGLPlanet ${SOURCES})
target_compile_features(OpenGLPlanet PRIVATE cxx_std_17)

# Glad include
target_include_directories(OpenGLPlanet PRIVATE
    ${OpenGL_ROOT}/include
    ${OpenGL_ROOT}
    ${OpenGL_ROOT}/include/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

# GLFW (fetched)
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# Link
target_link_libraries(OpenGLPlanet PRIVATE
    glfw
    opengl32
)

# Shader copy
add_custom_command(TARGET OpenGLPlanet POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:OpenGLPlanet>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:OpenGLPlanet>/shaders"
    COMMENT "Copying shaders to build directory"
)

# Install
install(TARGETS OpenGLPlanet DESTINATION bin)
