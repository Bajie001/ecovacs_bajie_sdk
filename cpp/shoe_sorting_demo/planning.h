#pragma once

#include <robot_sdk/robot_sdk.h>

#include <string>
#include <vector>

#include "pairing.h"

namespace shoe_sorting {

/**
 * 完整的鞋子整理方案生成（对应 Python `planning.get_shoe_move_steps`）。
 *
 * 内部依次调用：
 *   1. PairShoes                        — 配对（Pairwise + name-grouping）
 *   2. eco_ProjectPlaceLineAndPlane      — 放置区投影
 *   3. eco_ShoesIntentPlan              — 抓放规划
 *
 * @param head_image 头部图像数据（包含 RGB/深度、位姿、内参；匹配 Python `head_image`）
 * @param shoes      [输入输出] 检测到的鞋子列表；返回时 pair_id 已填入
 * @param carrier_box 载具框 4 个 3D 角点（匹配 Python `carrier_box`）
 * @param front_edge  前沿 2 个 3D 点（匹配 Python `front_edge`）
 * @param vis_prefix  可视化输出路径前缀；空则不落盘（匹配 Python `vis_prefix`）
 * @return 整理步骤列表
 * @throws std::runtime_error 配对/投影/规划任一失败
 */
std::vector<robot_sdk::ShoesIntentStep> GetShoeMoveSteps(
    const robot_sdk::ImageQueryNotifyData& head_image,
    std::vector<DetectedShoe>& shoes,
    const std::vector<std::vector<double>>& carrier_box,
    const std::vector<std::vector<double>>& front_edge,
    const std::string& vis_prefix,
    bool move_all = true);

/// 头图参考框链分辨率（对应 Python `planning.head_image_ref_bboxes_for_move_steps`）。
std::vector<std::vector<int>> HeadImageRefBboxes(
    const std::vector<robot_sdk::ShoesIntentStep>& steps);

}  // namespace shoe_sorting
