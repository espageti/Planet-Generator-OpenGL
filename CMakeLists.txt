cmake_minimum_required(VERSION 3.15)
project(OpenGLPlanet LANGUAGES C CXX)

set(OpenGL_ROOT "C:/OpenGL" CACHE PATH "Root directory for OpenGL/ImGui installations")

set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor" CACHE PATH "Vendor directory")

set(SOURCES
    src/main.cpp
    src/engine.cpp
    src/shader.cpp
    src/shapesettings.cpp
    src/perlinNoiseFilter.cpp
    src/planetUI.cpp
    src/sphere.cpp

)

add_executable(OpenGLPlanet ${SOURCES})
target_compile_features(OpenGLPlanet PRIVATE cxx_std_17)

# Build the glad library
add_library(glad STATIC ${VENDOR_DIR}/glad/src/glad.c)
target_include_directories(glad PUBLIC ${VENDOR_DIR}/glad/include)

# Link the glad library to the main executable
target_link_libraries(OpenGLPlanet PRIVATE glad)

# Include directories for the main executable
target_include_directories(OpenGLPlanet PRIVATE
    ${OpenGL_ROOT}/include
    ${OpenGL_ROOT}
    ${OpenGL_ROOT}/include/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${VENDOR_DIR}/glad/include
)

# GLFW (fetched)
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# ================================
# FetchContent: GLM
# ================================
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# ================================
# FetchContent: ImGui
# ================================

FetchContent_Populate(imgui
  URL https://github.com/ocornut/imgui/archive/docking.zip
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)


add_library(imgui_glfw STATIC
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp

  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(imgui_glfw PUBLIC glfw)

target_include_directories(imgui_glfw
PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/imgui
  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends
)

# Link GLFW and OpenGL libraries
target_link_libraries(OpenGLPlanet PRIVATE
    glfw
    opengl32
    glm::glm
    imgui_glfw
)



# Shader copy
add_custom_command(TARGET OpenGLPlanet POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:OpenGLPlanet>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:OpenGLPlanet>/shaders"
    COMMENT "Copying shaders to build directory"
)
message(STATUS "Include directories for OpenGLPlanet: ${OpenGLPlanet_INCLUDE_DIRECTORIES}")

# Install
install(TARGETS OpenGLPlanet DESTINATION bin)