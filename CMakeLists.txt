cmake_minimum_required(VERSION 3.10)
project(SimpleViewer)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/opengl/include)

file(GLOB_RECURSE VIEWER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(SimpleViewer STATIC ${VIEWER_SOURCES})

target_link_libraries(SimpleViewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl/lib/freeglut.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl/lib/glew32.lib
    opengl32)

add_executable(SimpleViewerTest example.cpp)

target_link_libraries(SimpleViewerTest SimpleViewer)
