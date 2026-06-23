#include "pairing.h"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace shoe_sorting {

std::vector<robot_sdk::ObjectDetection> VLMDetect(
    const std::string& image_data,
    const std::vector<std::string>& placeholders) {
  robot_sdk::DetectObjectsRequest req;
  req.labels = placeholders;
  req.rgb_image = image_data;
  req.entry = robot_sdk::OvdEndpoint::SHOE;

  std::vector<robot_sdk::ObjectDetection> items;
  std::string err;
  if (!robot_sdk::eco_DetectObjectsSimple(req, items, err)) {
    throw std::runtime_error("OVD 检测失败: " + err);
  }
  return items;
}

std::vector<DetectedShoe> DetectShoes(
    const std::string& image_data,
    const std::vector<std::string>& placeholders) {
  auto items = VLMDetect(image_data, placeholders);
  if (items.empty()) {
    throw std::runtime_error("OVD 未检测到目标");
  }

  std::vector<DetectedShoe> shoes;
  for (size_t i = 0; i < items.size(); ++i) {
    DetectedShoe shoe;
    if (items[i].bbox.size() < 4) {
      throw std::runtime_error("OVD 返回无效检测框");
    }
    shoe.bbox = items[i].bbox;
    shoe.pair_id = 0;
    shoe.name = items[i].name;
    shoe.id = items[i].uuid.empty() ? "shoe_" + std::to_string(i) : items[i].uuid;
    shoes.push_back(std::move(shoe));
  }
  return shoes;
}

void PairShoes(const std::string& image_data, std::vector<DetectedShoe>& shoes) {
  if (shoes.empty()) throw std::runtime_error("无可配对的鞋子");

  int next_pair_id = 1;
  // 阶段1: name 分组（仅恰好 2 只配对）
  std::map<std::string, std::vector<size_t>> groups;
  std::vector<size_t> leftovers;
  for (size_t i = 0; i < shoes.size(); ++i) {
    shoes[i].pair_id = 0;
    if (shoes[i].name.empty()) { leftovers.push_back(i); }
    else { groups[shoes[i].name].push_back(i); }
  }
  for (const auto& [_, indices] : groups) {
    if (indices.size() == 2) {
      shoes[indices[0]].pair_id = next_pair_id;
      shoes[indices[1]].pair_id = next_pair_id;
      ++next_pair_id;
    } else {
      for (size_t idx : indices) leftovers.push_back(idx);
    }
  }
  // 阶段2: leftovers 走 pairwise
  if (leftovers.size() >= 2) {
    robot_sdk::PairwiseSceneImage scene;
    scene.image = image_data;
    for (size_t idx : leftovers) {
      robot_sdk::PairwiseItemBBox item;
      item.bbox = shoes[idx].bbox;
      item.id = shoes[idx].id;
      item.item = shoes[idx].name.empty() ? "鞋子" : shoes[idx].name;
      scene.items.push_back(item);
    }
    std::vector<robot_sdk::PairwiseMatchedItem> matched;
    std::string pw_err;
    robot_sdk::PairwiseRequest pw_req;
    pw_req.images = {scene};
    if (robot_sdk::eco_PairwiseMatchSimple(pw_req, matched, pw_err)) {
      for (size_t idx : leftovers) shoes[idx].pair_id = 0;
      std::map<std::string, std::vector<std::string>> pw_map;
      for (const auto& m : matched) pw_map[m.pair_id].push_back(m.id);
      for (const auto& [_, ids] : pw_map) {
        if (ids.size() != 2) continue;
        for (size_t idx : leftovers) {
          if (shoes[idx].id == ids[0] || shoes[idx].id == ids[1])
            shoes[idx].pair_id = next_pair_id;
        }
        ++next_pair_id;
      }
    }
  }
  // 阶段3: 仍未配对的 → 独立 pair_id
  for (auto& s : shoes) {
    if (s.pair_id == 0) { s.pair_id = next_pair_id; ++next_pair_id; }
  }
}

}  // namespace shoe_sorting
