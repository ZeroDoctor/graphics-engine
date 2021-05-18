cmake_minimum_required(VERSION 3.10)
project(vendor C CXX)

#spdlog
set(spdlog_PATH "${CMAKE_SOURCE_DIR}/vendor/spdlog")
if(NOT TARGET spdlog)
    message("adding spdlog")
    add_subdirectory("${spdlog_PATH}")
endif()

#glfw3
set(glfw3_PATH "${CMAKE_SOURCE_DIR}/vendor/glfw")
if(NOT TARGET glfw)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_INSTALL OFF)
    
    message("adding glfw3")
    add_subdirectory("${glfw3_PATH}")
endif()

#glew
set(glew_PATH "${CMAKE_SOURCE_DIR}/vendor/glew-cmake")
if(NOT TARGET glew)
    add_definitions(-DGLEW_STATIC)
    set(NOSTD_LIBS ON)

    message("adding glew")
    add_subdirectory("${glew_PATH}")
endif()

#imgui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_FILES 
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imconfig.h"
    "${IMGUI_DIR}/imgui.h"
    "${IMGUI_DIR}/imgui_internal.h"
    "${IMGUI_DIR}/imstb_rectpack.h"
    "${IMGUI_DIR}/imstb_textedit.h"
    "${IMGUI_DIR}/imstb_truetype.h"
)

message("adding imgui")
add_library(imgui STATIC "${IMGUI_FILES}")

target_include_directories(imgui PUBLIC "${IMGUI_DIR}")

if(WIN32)
    target_link_libraries(imgui PRIVATE imm32)
endif()