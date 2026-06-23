#include "visualize.h"

#include <cmath>
#include <string>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <robot_sdk/math_utils.h>

namespace shoe_sorting {

cv::Mat DecodeImage(const std::string& base64_data) {
  std::vector<uint8_t> raw = robot_sdk::Base64Decode(base64_data);
  return cv::imdecode(raw, cv::IMREAD_COLOR);
}

void WriteRgbPng(const std::string& path, const cv::Mat& bgr) {
  cv::imwrite(path, bgr);
}

// ---- Bbox 中心像素 ----
static void BboxCenter(const std::vector<int>& bbox, int& cx, int& cy) {
  cx = static_cast<int>(std::round((bbox[0] + bbox[2]) * 0.5));
  cy = static_cast<int>(std::round((bbox[1] + bbox[3]) * 0.5));
}

// ---- 绘制带标签的矩形框（对应 Python _draw_labeled_box） ----
static void DrawLabeledBox(cv::Mat& image, const std::vector<int>& bbox,
                           const cv::Scalar& color, const std::string& label,
                           int thickness = 2) {
  cv::rectangle(image, cv::Point(bbox[0], bbox[1]),
                cv::Point(bbox[2], bbox[3]), color, thickness);
  int baseline = 0;
  cv::Size ts = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.55, 1, &baseline);
  int tx = bbox[0];
  int ty = std::max(bbox[1] - 6, ts.height + 4);
  cv::rectangle(image, cv::Point(tx - 2, ty - ts.height - 4),
                cv::Point(tx + ts.width + 2, ty + 2), cv::Scalar(0, 0, 0), -1);
  cv::putText(image, label, cv::Point(tx, ty), cv::FONT_HERSHEY_SIMPLEX,
              0.55, color, 1, cv::LINE_AA);
}

// ---- 绘制配对结果 ----
cv::Mat DrawPairs(const cv::Mat& bgr, const std::vector<DetectedShoe>& shoes) {
  cv::Mat image = bgr.clone();

  // 调色板（匹配 Python palette）
  static const cv::Scalar kPalette[] = {
      {0, 200, 255},    // BGR: blue-ish (Python: 255,200,0 = RGB)
      {80, 220, 80},    // green
      {255, 180, 0},    // orange
      {255, 80, 160},   // purple-ish
      {80, 160, 255},   // light blue
      {180, 80, 255},   // pink
      {0, 230, 180},    // teal
      {200, 200, 0},    // yellow
  };
  static const int kPaletteSize = sizeof(kPalette) / sizeof(kPalette[0]);

  // 按 pair_id 分组（pair_id 对应 Python 1-indexed pair index）
  // Python 中 pairs 是 [[uuid1, uuid2], ...]，pair_id(index+1)
  // 这里 pair_id > 0 的鞋用同色
  for (const auto& shoe : shoes) {
    if (shoe.pair_id <= 0) continue;
    cv::Scalar color = kPalette[(shoe.pair_id - 1) % kPaletteSize];
    DrawLabeledBox(image, shoe.bbox, color,
                   "P" + std::to_string(shoe.pair_id));
  }

  // 画中心点 + 连线（成对出现在同色的两个框之间）
  // 遍历所有 pair_id > 0 的鞋，每组配对画连线
  for (int pid = 1; pid <= 64; ++pid) {  // 最多 64 组
    std::vector<cv::Point> centers;
    for (const auto& shoe : shoes) {
      if (shoe.pair_id == pid) {
        int cx, cy;
        BboxCenter(shoe.bbox, cx, cy);
        centers.emplace_back(cx, cy);
      }
    }
    if (centers.size() < 2) continue;
    cv::Scalar color = kPalette[(pid - 1) % kPaletteSize];
    for (const auto& cp : centers) {
      cv::circle(image, cp, 4, color, -1, cv::LINE_AA);
    }
    cv::line(image, centers[0], centers[1], color, 2, cv::LINE_AA);
  }

  // 未配对的画灰色 "single"
  cv::Scalar single_color(140, 140, 140);
  for (const auto& shoe : shoes) {
    if (shoe.pair_id > 0) continue;
    DrawLabeledBox(image, shoe.bbox, single_color, "single", 1);
  }

  return image;
}

// ---- 绘制线面投影 ----
cv::Mat DrawPlacement(const cv::Mat& bgr,
                      const std::vector<std::vector<int>>& place_line,
                      const std::vector<std::vector<int>>& place_plane) {
  cv::Mat image = bgr.clone();

  // place_line: 折线（绿）
  if (place_line.size() >= 2) {
    std::vector<cv::Point> pts;
    for (const auto& p : place_line) {
      if (p.size() >= 2) pts.emplace_back(p[0], p[1]);
    }
    if (pts.size() >= 2) {
      cv::polylines(image, pts, false, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
    }
  }

  // place_plane: 半透明面（品红）+ 轮廓
  if (place_plane.size() >= 3) {
    std::vector<cv::Point> pts;
    for (const auto& p : place_plane) {
      if (p.size() >= 2) pts.emplace_back(p[0], p[1]);
    }
    if (pts.size() >= 3) {
      cv::Mat overlay = image.clone();
      cv::fillPoly(overlay, std::vector<std::vector<cv::Point>>{pts},
                   cv::Scalar(200, 50, 200), cv::LINE_AA);
      cv::addWeighted(overlay, 0.3, image, 0.7, 0, image);
      cv::polylines(image, pts, true, cv::Scalar(120, 0, 200), 2, cv::LINE_AA);
    }
  }

  return image;
}

// ---- 绘制规划步骤 ----
cv::Mat DrawPlans(const cv::Mat& bgr,
                  const std::vector<robot_sdk::ShoesIntentStep>& steps) {
  cv::Mat image = bgr.clone();
  cv::Scalar col_pick(0, 200, 0);     // 绿色抓取
  cv::Scalar col_place(0, 128, 255);  // 橙色落点
  cv::Scalar col_arrow(255, 255, 0);  // 青色箭头

  for (const auto& step : steps) {
    // from_bbox (pick)
    cv::rectangle(image, cv::Point(step.from_bbox[0], step.from_bbox[1]),
                  cv::Point(step.from_bbox[2], step.from_bbox[3]),
                  col_pick, 2);
    // to_bbox (place)
    cv::rectangle(image, cv::Point(step.to_bbox[0], step.to_bbox[1]),
                  cv::Point(step.to_bbox[2], step.to_bbox[3]),
                  col_place, 2);

    // 中心箭头
    int fx, fy, tx, ty;
    BboxCenter(step.from_bbox, fx, fy);
    BboxCenter(step.to_bbox, tx, ty);
    cv::arrowedLine(image, cv::Point(fx, fy), cv::Point(tx, ty),
                    col_arrow, 1, cv::LINE_AA, 0, 0.06);

    // 标签
    std::string label = "#" + std::to_string(step.step_id);
    int baseline = 0;
    cv::Size ts = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.55, 1, &baseline);
    int lx = step.from_bbox[0];
    int ly = std::max(step.from_bbox[1] - 6, ts.height + 4);
    cv::rectangle(image, cv::Point(lx - 2, ly - ts.height - 4),
                  cv::Point(lx + ts.width + 2, ly + 2), cv::Scalar(0, 0, 0), -1);
    cv::putText(image, label, cv::Point(lx, ly), cv::FONT_HERSHEY_SIMPLEX,
                0.55, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
  }

  return image;
}

}  // namespace shoe_sorting
