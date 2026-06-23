# 统一的 demo 构建目标配置：
# - 默认构建 x86，使用宿主机默认编译器
# - 传 -DARCH=arm 时切换到 aarch64 交叉编译工具链

set(ARCH "x86" CACHE STRING "目标架构，可选: x86, arm")
set_property(CACHE ARCH PROPERTY STRINGS x86 arm)
string(TOLOWER "${ARCH}" ARCH)

if(NOT ARCH STREQUAL "x86" AND NOT ARCH STREQUAL "arm")
  message(FATAL_ERROR
    "不支持的 ARCH=\"${ARCH}\"，可选值仅支持 x86 或 arm。")
endif()

if(ARCH STREQUAL "arm")
  set(CPP_SDK_ARM_TRIPLE "aarch64-linux-gnu" CACHE STRING
    "arm 交叉编译工具链前缀")
  set(CPP_SDK_ARM_C_COMPILER "${CPP_SDK_ARM_TRIPLE}-gcc" CACHE FILEPATH
    "arm 交叉编译 C 编译器")
  set(CPP_SDK_ARM_CXX_COMPILER "${CPP_SDK_ARM_TRIPLE}-g++" CACHE FILEPATH
    "arm 交叉编译 C++ 编译器")

  if(IS_ABSOLUTE "${CPP_SDK_ARM_C_COMPILER}")
    set(_robot_sdk_arm_c_compiler "${CPP_SDK_ARM_C_COMPILER}")
  else()
    find_program(_robot_sdk_arm_c_compiler NAMES "${CPP_SDK_ARM_C_COMPILER}")
  endif()

  if(IS_ABSOLUTE "${CPP_SDK_ARM_CXX_COMPILER}")
    set(_robot_sdk_arm_cxx_compiler "${CPP_SDK_ARM_CXX_COMPILER}")
  else()
    find_program(_robot_sdk_arm_cxx_compiler NAMES "${CPP_SDK_ARM_CXX_COMPILER}")
  endif()

  if(NOT _robot_sdk_arm_c_compiler)
    message(FATAL_ERROR
      "未找到 arm 交叉编译器: ${CPP_SDK_ARM_C_COMPILER}\n"
      "可通过 -DCPP_SDK_ARM_C_COMPILER=/path/to/aarch64-linux-gnu-gcc 覆盖。")
  endif()

  if(NOT _robot_sdk_arm_cxx_compiler)
    message(FATAL_ERROR
      "未找到 arm 交叉编译器: ${CPP_SDK_ARM_CXX_COMPILER}\n"
      "可通过 -DCPP_SDK_ARM_CXX_COMPILER=/path/to/aarch64-linux-gnu-g++ 覆盖。")
  endif()

  set(CMAKE_SYSTEM_NAME Linux CACHE STRING "目标系统" FORCE)
  set(CMAKE_SYSTEM_PROCESSOR aarch64 CACHE STRING "目标处理器" FORCE)
  set(CMAKE_C_COMPILER "${_robot_sdk_arm_c_compiler}" CACHE FILEPATH
    "C 编译器" FORCE)
  set(CMAKE_CXX_COMPILER "${_robot_sdk_arm_cxx_compiler}" CACHE FILEPATH
    "C++ 编译器" FORCE)
endif()
