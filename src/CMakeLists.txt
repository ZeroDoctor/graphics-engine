cmake_minimum_required(VERSION 3.2)

project(vulkan-graphics-lib)

if (WIN32)
    list(APPEND GLM_INCLUDE_DIR "C:/Library/glm-0.9.9.8/glm/glm")
    list(APPEND GLM_LIBRARY "C:/Library/glm-0.9.9.8/glm/glm")
endif(WIN32)

find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glm REQUIRED FATAL_ERROR)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR} ${GLM_LIBRARY}")
find_package(Freetype REQUIRED FATAL_ERROR)

file(GLOB util "util/*.cpp")
file(GLOB init "init/*.cpp")
file(GLOB setup "setup/*.cpp")
file(GLOB renderer "renderer/*.cpp")
file(GLOB main "*.cpp")

add_executable (vulkan-graphics-lib "${main}" "${util}" "${init}" "${setup}" "${renderer}")

target_include_directories(vulkan-graphics-lib PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_include_directories(vulkan-graphics-lib PUBLIC ${GLM_INCLUDE_DIRS})
target_include_directories(vulkan-graphics-lib PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(vulkan-graphics-lib PRIVATE util)
target_include_directories(vulkan-graphics-lib PRIVATE init)
target_include_directories(vulkan-graphics-lib PRIVATE setup)
target_include_directories(vulkan-graphics-lib PRIVATE renderer)
target_include_directories(vulkan-graphics-lib PRIVATE main)

# target_compile_options(vulkan-graphics-lib PRIVATE "-Wno-format" "-Wno-format-security")
target_compile_options(vulkan-graphics-lib PRIVATE "-Wreturn-type")

target_link_libraries(vulkan-graphics-lib ${Vulkan_LIBRARY} ${GLM_LIBRARY} ${FREETYPE_LIBRARIES})
