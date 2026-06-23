#pragma once

#include <robot_sdk/robot_sdk.h>

#include <string>
#include <vector>

namespace shoe_sorting {

/// 单只检测到的鞋子。
struct DetectedShoe {
  std::vector<int> bbox;  // [xmin, ymin, xmax, ymax]
  int pair_id{0};          // 0 = 未配对；>0 = 配对的 1-indexed 组号
  std::string name;        // 物品名（OVD 返回）
  std::string id;          // 内部 UUID（Pairwise 匹配使用）
};

/**
 * 底层 OVD 检测（共享避免重复，对应 eco_DetectObjectsSimple）。
 * 失败时抛出 std::runtime_error。
 */
std::vector<robot_sdk::ObjectDetection> VLMDetect(
    const std::string& image_data,
    const std::vector<std::string>& placeholders);

/**
 * OVD 检测图像中的鞋子（对应 Python `shoe_sorting_demo.shoe_sort` 中的 `eco_detect_objects`）。
 *
 * @param image_data  头部图像 RGB base64
 * @param width       图像宽
 * @param height      图像高
 * @param placeholders 识别类别（如 {"鞋子"}）
 * @throws std::runtime_error OVD 失败或未检测到目标
 */
std::vector<DetectedShoe> DetectShoes(const std::string& image_data,
                                      const std::vector<std::string>& placeholders);

/**
 * 鞋子配对（对应 Python `pairing.pair_shoes`）。
 *
 * 算法（与 Python 完全对齐）：
 *   1. 先用 Pairwise 服务匹配
 *   2. 若无匹配结果，按 name 分组，同组内两两配对
 *   3. 若同组 >2 只（如 4 只同款），成对输出
 *   4. 奇数剩余视为未配对
 *
 * @param image_data 头部图像 RGB base64
 * @param shoes      [输入输出] 检测到的鞋子；输出时 pair_id 被填入
 * @throws std::runtime_error 无可配对的鞋子
 */
void PairShoes(const std::string& image_data, std::vector<DetectedShoe>& shoes);

}  // namespace shoe_sorting
