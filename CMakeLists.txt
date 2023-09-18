cmake_minimum_required(VERSION 3.10)

project(SERVER)

file(GLOB CPP_FILES
    ${PROJECT_SOURCE_DIR}/src/base/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.cpp)

set(CMAKE_CXX_STANDARD 11)

add_executable(server main.cpp ${CPP_FILES})

target_link_libraries(server pthread)