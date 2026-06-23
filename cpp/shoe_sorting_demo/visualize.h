#pragma once

#include <opencv2/core.hpp>

#include <string>
#include <vector>

#include "pairing.h"

namespace shoe_sorting {

/// 将 base64 图像数据解码为 OpenCV BGR Mat（自动识别 JPEG/PNG）。
cv::Mat DecodeImage(const std::string& base64_data);

/// 保存 BGR Mat 为 PNG。
void WriteRgbPng(const std::string& path, const cv::Mat& bgr);

/// 在 BGR 图像上绘制鞋子配对结果（对应 Python draw_pairs_on_rgb）。
cv::Mat DrawPairs(const cv::Mat& bgr, const std::vector<DetectedShoe>& shoes);

/// 在 BGR 图像上绘制摆放区投影（对应 Python draw_placement_on_rgb）。
cv::Mat DrawPlacement(const cv::Mat& bgr,
                      const std::vector<std::vector<int>>& place_line,
                      const std::vector<std::vector<int>>& place_plane);

/// 在 BGR 图像上绘制规划步骤（对应 Python draw_plans_on_rgb）。
cv::Mat DrawPlans(const cv::Mat& bgr,
                  const std::vector<robot_sdk::ShoesIntentStep>& steps);

}  // namespace shoe_sorting
