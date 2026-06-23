#ifndef ROBOT_SDK_EXPORT_H_
#define ROBOT_SDK_EXPORT_H_

/**
 * @file export.h
 * @brief 跨平台 DLL 导出/导入宏。编译 librobot_sdk 时定义 ROBOT_SDK_BUILDING；应用侧包含头文件时不定义。
 */

#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef ROBOT_SDK_BUILDING
#    define ROBOT_SDK_PUBLIC __declspec(dllexport)
#  else
#    define ROBOT_SDK_PUBLIC __declspec(dllimport)
#  endif
#else
#  define ROBOT_SDK_PUBLIC __attribute__((visibility("default")))
#endif

#endif
