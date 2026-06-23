# 独立工程：只编译本演示源码，链接已构建的 librobot_sdk.so
# 1) 准备 SDK 头文件和预编译库（默认使用 depend/lib/x86，arm 使用 depend/lib/arm）
# 2) 再构建本演示：
#    cmake -S . -B build && cmake --build build
#    cmake -S . -B build -DARCH=arm && cmake --build build
# 若 .so 与头文件在其它前缀：cmake -S . -B build -DCPP_SDK_ROOT=/path/to/sdk
# 若 arm 交叉编译器路径不在 PATH 中，可覆盖：
#    -DCPP_SDK_ARM_C_COMPILER=/path/to/aarch64-linux-gnu-gcc
#    -DCPP_SDK_ARM_CXX_COMPILER=/path/to/aarch64-linux-gnu-g++
# 运行：./build/bin/toy_storage_demo
cmake_minimum_required(VERSION 3.14)

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/robot_sdk_target.cmake")
project(toy_storage_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/robot_sdk_prebuilt.cmake")

add_executable(toy_storage_demo toy_storage_demo.cpp)
target_link_libraries(toy_storage_demo PRIVATE robot_sdk::robot_sdk_prebuilt)
target_compile_features(toy_storage_demo PRIVATE cxx_std_17)
set_target_properties(toy_storage_demo PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CPP_SDK_OUTPUT_DIR}"
)
robot_sdk_bundle_runtime(toy_storage_demo)
