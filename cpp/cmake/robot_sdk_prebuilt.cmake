# 供 demo 独立演示使用：仅链接已构建的 librobot_sdk.so，不编译 SDK 源码。
# 要求 CPP_SDK_ROOT 下存在 include/robot_sdk/*.h，并在 lib/<arch>/ 下提供 librobot_sdk.so。
#
# 默认 CPP_SDK_ROOT = <仓库根>/cpp/depend
if(NOT DEFINED CPP_SDK_ROOT OR CPP_SDK_ROOT STREQUAL "")
  get_filename_component(_robot_sdk_repo_root "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
  set(CPP_SDK_ROOT "${_robot_sdk_repo_root}/cpp/depend" CACHE PATH
    "预编译 robot_sdk 根目录（含 include/ 与 lib/<target>）")
endif()

if(NOT DEFINED ARCH OR ARCH STREQUAL "")
  set(ARCH "x86" CACHE STRING "目标架构，可选: x86, arm")
endif()
set_property(CACHE ARCH PROPERTY STRINGS x86 arm)
string(TOLOWER "${ARCH}" ARCH)

if(NOT ARCH STREQUAL "x86" AND NOT ARCH STREQUAL "arm")
  message(FATAL_ERROR
    "不支持的 ARCH=\"${ARCH}\"，可选值仅支持 x86 或 arm。")
endif()

if(NOT DEFINED CPP_SDK_ARCH OR CPP_SDK_ARCH STREQUAL "")
  set(CPP_SDK_ARCH "${ARCH}" CACHE STRING
    "robot_sdk 预编译库目录名，默认跟随 ARCH")
endif()

if(NOT DEFINED CPP_SDK_OUTPUT_DIR OR CPP_SDK_OUTPUT_DIR STREQUAL "")
  set(CPP_SDK_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin" CACHE PATH
    "demo 可执行文件输出目录")
endif()

set(_CPP_SDK_LIB_DIR_CANDIDATES)
list(APPEND _CPP_SDK_LIB_DIR_CANDIDATES
  "${CPP_SDK_ROOT}/lib/${CPP_SDK_ARCH}"
  "${CPP_SDK_ROOT}/${CPP_SDK_ARCH}/lib"
)
list(APPEND _CPP_SDK_LIB_DIR_CANDIDATES "${CPP_SDK_ROOT}/lib")
list(REMOVE_DUPLICATES _CPP_SDK_LIB_DIR_CANDIDATES)

unset(_CPP_SDK_IMPORTED_LIB CACHE)
find_library(_CPP_SDK_IMPORTED_LIB
  NAMES robot_sdk
  PATHS ${_CPP_SDK_LIB_DIR_CANDIDATES}
  NO_DEFAULT_PATH
)
if(NOT _CPP_SDK_IMPORTED_LIB)
  string(JOIN "\n  " _robot_sdk_lib_dir_hint ${_CPP_SDK_LIB_DIR_CANDIDATES})
  message(FATAL_ERROR
    "未找到与当前架构匹配的 librobot_sdk.so。\n"
    "目标架构: ${ARCH}\n"
    "库目录名: ${CPP_SDK_ARCH}\n"
    "已检查目录:\n  ${_robot_sdk_lib_dir_hint}\n"
    "请通过 -DCPP_SDK_ROOT= 指定 SDK 根目录，或通过 -DCPP_SDK_ARCH= 覆盖库目录名。")
endif()

get_filename_component(CPP_SDK_LIBRARY_DIR "${_CPP_SDK_IMPORTED_LIB}" DIRECTORY)

file(GLOB CPP_SDK_RUNTIME_LIBS
  LIST_DIRECTORIES FALSE
  "${CPP_SDK_LIBRARY_DIR}/librobot_sdk.so*"
)
if(NOT CPP_SDK_RUNTIME_LIBS)
  message(FATAL_ERROR
    "在 \"${CPP_SDK_LIBRARY_DIR}\" 下未找到可打包的 librobot_sdk.so 运行时文件。")
endif()

find_path(_CPP_SDK_IMPORTED_INCLUDE_DIR
  NAMES robot_sdk/robot_sdk.h
  PATHS "${CPP_SDK_ROOT}/include"
  NO_DEFAULT_PATH
)
if(NOT _CPP_SDK_IMPORTED_INCLUDE_DIR)
  message(FATAL_ERROR
    "未在 \"${CPP_SDK_ROOT}/include\" 找到 robot_sdk/robot_sdk.h。\n"
    "请确认已构建 SDK（会同步头文件到 build/include）或设置正确的 CPP_SDK_ROOT。")
endif()

if(NOT TARGET robot_sdk::robot_sdk_prebuilt)
  add_library(robot_sdk::robot_sdk_prebuilt SHARED IMPORTED)
  set_target_properties(robot_sdk::robot_sdk_prebuilt PROPERTIES
    IMPORTED_LOCATION "${_CPP_SDK_IMPORTED_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${_CPP_SDK_IMPORTED_INCLUDE_DIR}"
  )
endif()

function(robot_sdk_bundle_runtime target_name)
  if(NOT TARGET "${target_name}")
    message(FATAL_ERROR "target \"${target_name}\" 不存在，无法打包运行时依赖。")
  endif()

  # 让 demo 在输出目录中优先查找同目录下打包的 SDK 动态库。
  set_target_properties("${target_name}" PROPERTIES
    INSTALL_RPATH "\$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH FALSE
  )

  foreach(_runtime_lib IN LISTS CPP_SDK_RUNTIME_LIBS)
    add_custom_command(TARGET "${target_name}" POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${_runtime_lib}"
        "$<TARGET_FILE_DIR:${target_name}>/"
      VERBATIM
    )
  endforeach()
endfunction()
