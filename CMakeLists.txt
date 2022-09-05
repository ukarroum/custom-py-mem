cmake_minimum_required(VERSION 3.23)
project(python_mem)

set(CMAKE_CXX_STANDARD 17)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
add_executable(python_mem
        cusmemmodule.cpp)
