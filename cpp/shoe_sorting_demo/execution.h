#pragma once

#include <robot_sdk/robot_sdk.h>

#include <string>
#include <vector>

namespace shoe_sorting {

/**
 * 执行单步鞋子整理：抓取 → 放置（匹配 Python `execution.execute_pick_and_place`）。
 *
 * 抓取失败或放置失败均抛出 std::runtime_error（内容含失败描述），
 * 每步内部自动重试 3 次，用尽后异常向外传播。
 *
 * @param head_image          头部拍照结果
 * @param step                规划单步（from_bbox / to_bbox）
 * @param front_edge          前沿点列（3D，匹配 Python `front_edge` 参数）
 * @param object_name         物品名（如 "鞋子"）
 * @param head_image_ref_bbox 头图参考框（匹配 Python `head_image_ref_bbox`）
 */
void ExecutePickAndPlace(const robot_sdk::ImageQueryNotifyData& head_image,
                         const robot_sdk::ShoesIntentStep& step,
                         const std::vector<std::vector<double>>& front_edge,
                         const std::string& object_name,
                         const std::vector<int>& head_image_ref_bbox,
                         bool no_voice = false);

}  // namespace shoe_sorting
