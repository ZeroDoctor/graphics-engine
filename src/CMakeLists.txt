cmake_minimum_required(VERSION 3.2)

project(graphics-engine)

set (CMAKE_CXX_STANDARD 17)

if (WIN32)
    list(APPEND GLM_INCLUDE_DIR "C:/Library/glm-0.9.9.8/glm/glm")
    set(glm_DIR "C:/Library/glm-0.9.9.8/glm/cmake/glm")
endif(WIN32)

find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glm REQUIRED FATAL_ERROR)
find_package(Freetype REQUIRED FATAL_ERROR)

file(GLOB util "util/*.cpp")
file(GLOB init "init/*.cpp")
file(GLOB setup "setup/*.cpp")
file(GLOB renderer "renderer/*.cpp")
file(GLOB main "*.cpp")

add_executable (graphics-engine "${main}" "${util}" "${init}" "${setup}" "${renderer}")

target_include_directories(graphics-engine PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_include_directories(graphics-engine PUBLIC ${GLM_INCLUDE_DIRS})
target_include_directories(graphics-engine PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(graphics-engine PRIVATE util)
target_include_directories(graphics-engine PRIVATE init)
target_include_directories(graphics-engine PRIVATE setup)
target_include_directories(graphics-engine PRIVATE renderer)
target_include_directories(graphics-engine PRIVATE main)

# target_compile_options(graphics-engine PRIVATE "-Wno-format" "-Wno-format-security")
target_compile_options(graphics-engine PRIVATE "-Wreturn-type")

target_link_libraries(graphics-engine ${Vulkan_LIBRARY} ${GLM_LIBRARY} ${FREETYPE_LIBRARIES})
