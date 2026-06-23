#include "planning.h"

#include <cmath>
#include <iostream>
#include <map>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "pairing.h"
#include "visualize.h"

namespace shoe_sorting {

namespace {
double QuatXAxisYaw(const robot_sdk::Orientation& q) {
  double rxx = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z;
  double ryx = 2.0 * q.x * q.y + 2.0 * q.w * q.z;
  return std::atan2(ryx, rxx);
}
}  // namespace

static auto BboxKey(const std::vector<int>& b) {
  return std::make_tuple(b[0], b[1], b[2], b[3]);
}

std::vector<std::vector<int>> HeadImageRefBboxes(
    const std::vector<robot_sdk::ShoesIntentStep>& steps) {
  using Key = std::tuple<int, int, int, int>;
  std::map<Key, Key> to_to_from;
  for (const auto& m : steps) to_to_from[BboxKey(m.to_bbox)] = BboxKey(m.from_bbox);
  std::vector<std::vector<int>> out;
  for (const auto& m : steps) {
    Key cur = BboxKey(m.from_bbox);
    for (size_t i = 0; i <= steps.size(); ++i) {
      auto it = to_to_from.find(cur);
      if (it == to_to_from.end()) break;
      cur = it->second;
    }
    out.push_back({std::get<0>(cur), std::get<1>(cur), std::get<2>(cur), std::get<3>(cur)});
  }
  return out;
}

std::vector<robot_sdk::ShoesIntentStep> GetShoeMoveSteps(
    const robot_sdk::ImageQueryNotifyData& head_image,
    std::vector<DetectedShoe>& shoes,
    const std::vector<std::vector<double>>& carrier_box,
    const std::vector<std::vector<double>>& front_edge,
    const std::string& vis_prefix,
    bool move_all) {
  const auto& image_data = head_image.rgb_image.data;
  bool save_vis = !vis_prefix.empty();

  // 1. 配对
  PairShoes(image_data, shoes);
  int paired = 0;
  for (const auto& s : shoes) if (s.pair_id > 0) ++paired;
  std::cout << "  配对: " << paired << "/" << shoes.size() << " 已配对"
            << (shoes.size() - paired) << " 未配对" << std::endl;

  if (save_vis) WriteRgbPng(vis_prefix + "_pairs.png", DrawPairs(DecodeImage(image_data), shoes));

  // 2. 放置区投影
  robot_sdk::ProjectPlaceLineAndPlaneRequest proj_req;
  {
    robot_sdk::ProjectPlaceLineAndPlaneView view;
    view.rgb_image = head_image.rgb_image;
    view.depth_image = head_image.depth_image;
    view.tf_goal = head_image.tf_goal;
    view.camera_info_k = head_image.camera_info_k;
    proj_req.views.push_back(std::move(view));
    for (const auto& pt : carrier_box)
      proj_req.carrier_box.push_back({static_cast<float>(pt[0]), static_cast<float>(pt[1]), static_cast<float>(pt[2])});
    for (const auto& pt : front_edge)
      proj_req.front_edge.push_back({static_cast<float>(pt[0]), static_cast<float>(pt[1]), static_cast<float>(pt[2])});
  }
  auto proj_resp = robot_sdk::eco_ProjectPlaceLineAndPlane(proj_req);
  if (!proj_resp || proj_resp->code != 0 || proj_resp->data.empty())
    throw std::runtime_error("投影摆放区失败");
  const auto& place_line = proj_resp->data[0].place_line;
  const auto& place_plane = proj_resp->data[0].place_plane;
  const auto& place_yaw = proj_resp->data[0].place_yaw;

  cv::Mat placement_vis;
  if (save_vis) {
    placement_vis = DrawPlacement(DecodeImage(image_data), place_line, place_plane);
    WriteRgbPng(vis_prefix + "_placement.png", placement_vis);
  }

  // 3. 步进规划
  robot_sdk::ShoesIntentPlanRequest req;
  for (const auto& shoe : shoes) {
    robot_sdk::ShoesIntentShoeItem item;
    item.bbox = shoe.bbox; item.pair_id = shoe.pair_id;
    req.shoes.push_back(item);
  }

  if (!move_all) {
    static const double kTiltThreshRad = 30.0 * 3.14159265358979323846 / 180.0;
    for (auto& item : req.shoes) {
      item.is_tilted = true;
      try {
        robot_sdk::BboxItem fb;
        fb.id = 0;
        fb.left_up_x = static_cast<float>(item.bbox[0]);
        fb.left_up_y = static_cast<float>(item.bbox[1]);
        fb.right_down_x = static_cast<float>(item.bbox[2]);
        fb.right_down_y = static_cast<float>(item.bbox[3]);
        robot_sdk::GetPoseData gp;
        gp.bbox = {fb}; gp.rgb_image = head_image.rgb_image;
        gp.depth_image = head_image.depth_image; gp.tf_map = head_image.tf_goal;
        robot_sdk::GetPoseNotifyData pr;
        if (robot_sdk::eco_GetPose(gp, pr, 6000).code == 0 && !pr.pose_results.empty()) {
          double diff = std::fmod(std::abs(QuatXAxisYaw(pr.pose_results[0].orientation) - place_yaw), 3.14159265358979323846);
          item.is_tilted = std::min(diff, 3.14159265358979323846 - diff) >= kTiltThreshRad;
        }
      } catch (const std::exception&) {}
    }
  }

  req.place_line = place_line;
  req.place_plane = place_plane;
  auto plan_result = robot_sdk::eco_ShoesIntentPlan(req, move_all);
  if (!plan_result || plan_result->code != 0)
    throw std::runtime_error("ShoesIntentPlan 失败: code=" + std::to_string(plan_result ? plan_result->code : -1));
  std::vector<robot_sdk::ShoesIntentStep> steps = plan_result->steps;
  std::cout << "  规划: " << steps.size() << " 步" << std::endl;

  if (save_vis) {
    cv::Mat base = placement_vis.empty() ? DecodeImage(image_data) : placement_vis;
    WriteRgbPng(vis_prefix + "_plan.png", DrawPlans(base, steps));
  }
  return steps;
}

}  // namespace shoe_sorting
