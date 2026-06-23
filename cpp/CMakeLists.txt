cmake_minimum_required(VERSION 3.14)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/robot_sdk_target.cmake")
project(robot_sdk_demo_collection LANGUAGES CXX)

if(NOT DEFINED CPP_SDK_OUTPUT_DIR OR CPP_SDK_OUTPUT_DIR STREQUAL "")
  set(CPP_SDK_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/out/${ARCH}" CACHE PATH
    "demo 可执行文件输出目录")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CPP_SDK_OUTPUT_DIR}")

add_subdirectory(delivery_demo)
add_subdirectory(desk_demo)
add_subdirectory(toy_storage_demo)
