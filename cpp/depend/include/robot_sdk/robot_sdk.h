/**
 * @file robot_sdk.h
 * @brief C++ SDK 对外推荐总头文件：导出符号、数据结构、客户端、机器人技能、位置与放置服务。
 *
 * 使用方式：\#include <robot_sdk/robot_sdk.h>，链接 librobot_sdk（或 robot_sdk 目标），C++17。
 *
 * @note 当前 SDK 版本为 1.1.0。构建产物的真实库文件为 librobot_sdk.so.1.1.0，
 *       同时保留 librobot_sdk.so 链接名，使用方仍可通过 -lrobot_sdk 或 CMake 目标链接。
 */
#ifndef ROBOT_SDK_ROBOT_SDK_H_
#define ROBOT_SDK_ROBOT_SDK_H_

#include "export.h"
#include "types.h"
#include "client.h"
#include "robot_func.h"
#include "location.h"
#include "ovd.h"
#include "put_where.h"
#include "shoes_intent.h"
#include "desk_intent.h"
#include "img_match.h"
#include "math_utils.h"
#include "pose_tools.h"

#endif
