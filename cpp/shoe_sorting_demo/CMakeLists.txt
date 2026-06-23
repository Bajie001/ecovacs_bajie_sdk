cmake_minimum_required(VERSION 3.14)

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/robot_sdk_target.cmake")
project(shoe_sorting_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/robot_sdk_prebuilt.cmake")

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

set(SHOE_SORTING_EXTRA_LIBS)
if(ARCH STREQUAL "arm")
  find_library(SHOE_SORTING_BLAS_LIB
    NAMES blas
    PATHS /usr/lib/aarch64-linux-gnu/blas /usr/lib/aarch64-linux-gnu
    NO_DEFAULT_PATH
  )
  find_library(SHOE_SORTING_LAPACK_LIB
    NAMES lapack
    PATHS /usr/lib/aarch64-linux-gnu/lapack /usr/lib/aarch64-linux-gnu
    NO_DEFAULT_PATH
  )
  foreach(_shoe_sorting_link_dir
      /usr/lib/aarch64-linux-gnu/blas
      /usr/lib/aarch64-linux-gnu/lapack
      /usr/lib/aarch64-linux-gnu)
    if(EXISTS "${_shoe_sorting_link_dir}")
      add_link_options("LINKER:-rpath-link,${_shoe_sorting_link_dir}")
    endif()
  endforeach()
  if(SHOE_SORTING_BLAS_LIB)
    list(APPEND SHOE_SORTING_EXTRA_LIBS "${SHOE_SORTING_BLAS_LIB}")
  endif()
  if(SHOE_SORTING_LAPACK_LIB)
    list(APPEND SHOE_SORTING_EXTRA_LIBS "${SHOE_SORTING_LAPACK_LIB}")
  endif()
endif()

add_executable(shoe_sorting_demo
  shoe_sorting_demo.cpp
  pairing.cpp
  planning.cpp
  execution.cpp
  matching.cpp
  visualize.cpp
)
target_link_libraries(shoe_sorting_demo PRIVATE
  robot_sdk::robot_sdk_prebuilt
  ${OpenCV_LIBS}
  ${SHOE_SORTING_EXTRA_LIBS}
)
target_compile_features(shoe_sorting_demo PRIVATE cxx_std_17)
set_target_properties(shoe_sorting_demo PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CPP_SDK_OUTPUT_DIR}"
)
robot_sdk_bundle_runtime(shoe_sorting_demo)
