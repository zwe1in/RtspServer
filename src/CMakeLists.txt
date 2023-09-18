cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

file(GLOB src_file "*.cpp")

add_executable(main ${src_file})
