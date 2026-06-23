#pragma once

#include <string>

/** 固定文案语音播报（eco_Speech 非阻塞），失败不中断主流程。对齐 python/desk_demo/speech.py */
namespace speech {

inline constexpr const char kRelocating[] = u8"正在重定位";
inline constexpr const char kPreparePose[] = u8"正在准备姿态";
inline constexpr const char kNavigating[] = u8"正在导航";
inline constexpr const char kRaising[] = u8"正在升高";
inline constexpr const char kHeadAngle[] = u8"正在调整头部角度";
inline constexpr const char kPerceiving[] = u8"正在识别物品";
inline constexpr const char kPlanning[] = u8"正在规划";
inline constexpr const char kExecution[] = u8"开始整理";
inline constexpr const char kGrabbing[] = u8"正在抓取";
inline constexpr const char kPlacing[] = u8"正在放置";
inline constexpr const char kJudging[] = u8"正在评判";
inline constexpr const char kDone[] = u8"整理完成";

void Speak(const std::string& text);

}  // namespace speech
