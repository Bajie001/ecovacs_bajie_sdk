#include "execution.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include <robot_sdk/math_utils.h>

#include "matching.h"
#include "pairing.h"

namespace shoe_sorting {
namespace {

static void Say(const std::string& text, bool no_voice = false) {
    std::cout << "[voice] " << text << std::endl;
    if (no_voice) return;
    robot_sdk::eco_Speech(text);
}

/// 相机射线与父系 z=0 平面求交（匹配 Python _place_position_pixel_ray_ground_z0）。
static robot_sdk::Position PlacePositionPixelRayGroundZ0(
    const robot_sdk::PoseStamped& tf_goal,
    const std::vector<double>& camera_info_k,
    double u, double v) {
  double fx = camera_info_k[0], fy = camera_info_k[4];
  double cx = camera_info_k[2], cy = camera_info_k[5];

  double d_cam_x = (u - cx) / fx;
  double d_cam_y = (v - cy) / fy;
  double d_cam_z = 1.0;

  std::vector<double> R = robot_sdk::QuatToRotMat(tf_goal.pose.orientation);
  double tx = static_cast<double>(tf_goal.pose.position.x);
  double ty = static_cast<double>(tf_goal.pose.position.y);
  double tz = static_cast<double>(tf_goal.pose.position.z);

  double d_parent_x = R[0] * d_cam_x + R[3] * d_cam_y + R[6] * d_cam_z;
  double d_parent_y = R[1] * d_cam_x + R[4] * d_cam_y + R[7] * d_cam_z;
  double d_parent_z = R[2] * d_cam_x + R[5] * d_cam_y + R[8] * d_cam_z;

  double s = -tz / d_parent_z;
  robot_sdk::Position pos;
  pos.x = static_cast<float>(tx + s * d_parent_x);
  pos.y = static_cast<float>(ty + s * d_parent_y);
  pos.z = 0.0f;
  return pos;
}

// ======== PickOnce（对应 Python eco_pick_with_arm_review_head_image_ref） ========

static void PickOnce(const robot_sdk::ImageQueryNotifyData& head_image,
                     const std::vector<int>& from_bbox,
                     const std::string& object_name,
                     const std::vector<int>& head_image_ref_bbox) {
  // 1. eco_lookto（首次步 / 后续步，对应 Python 的 bbox / pose 重载）
  bool is_first =
      head_image_ref_bbox.size() >= 4 && from_bbox.size() >= 4 &&
      head_image_ref_bbox[0] == from_bbox[0] &&
      head_image_ref_bbox[1] == from_bbox[1] &&
      head_image_ref_bbox[2] == from_bbox[2] &&
      head_image_ref_bbox[3] == from_bbox[3];
  {
    if (is_first) {
      robot_sdk::BboxItem fb;
      fb.id = 0;
      fb.left_up_x = static_cast<float>(from_bbox[0]);
      fb.left_up_y = static_cast<float>(from_bbox[1]);
      fb.right_down_x = static_cast<float>(from_bbox[2]);
      fb.right_down_y = static_cast<float>(from_bbox[3]);

      robot_sdk::GetPoseData gp;
      gp.bbox = {fb};
      gp.rgb_image = head_image.rgb_image;
      gp.depth_image = head_image.depth_image;
      gp.tf_map = head_image.tf_goal;

      robot_sdk::GetPoseNotifyData result;
      robot_sdk::ErrorCode ec = robot_sdk::eco_GetPose(gp, result);
      if (ec.code != 0 || result.pose_results.empty()) {
        throw std::runtime_error("GetPose 无结果: code=" + std::to_string(ec.code));
      }

      const auto& p = result.pose_results[0];
      robot_sdk::BoxData box;
      box.position = p.position;
      box.orientation = p.orientation;
      box.box_length = p.box_length;
      box.frame_id = p.frame_id;

      ec = robot_sdk::eco_LookTo(robot_sdk::LookToTarget::HAND, box);
      if (ec.code != 0) {
        std::cout << "    臂对准非致命: code=" << ec.code << "（继续）" << std::endl;
      }
    } else {
      robot_sdk::BoxData box;
      double u = (head_image_ref_bbox[0] + head_image_ref_bbox[2]) * 0.5;
      double v = (head_image_ref_bbox[1] + head_image_ref_bbox[3]) * 0.5;
      try {
        box.position = robot_sdk::PixelToPoint3D(head_image, u, v);
        box.box_length = {0.12f, 0.10f, 0.05f};
        box.frame_id = head_image.tf_goal.header.frame_id;
        robot_sdk::ErrorCode ec2 = robot_sdk::eco_LookTo(robot_sdk::LookToTarget::HAND, box);
        if (ec2.code != 0) {
          throw std::runtime_error("臂对准失败: code=" + std::to_string(ec2.code) +
                                   ", msg=" + ec2.msg);
        }
      } catch (const std::exception& e) {
        std::cout << "    后续步对准失败: " << e.what() << std::endl;
      }
    }
  }

  // 2. eco_captureImages(CameraType.ARM)
  std::cout << "    臂图拍照..." << std::endl;
  robot_sdk::ImageQueryNotifyData arm_image;
  {
    robot_sdk::ImageQueryData q;
    q.camera_type = robot_sdk::CameraType::ARM;
    robot_sdk::ThrowOnError(robot_sdk::eco_ImageQuery(q, arm_image), "臂图拍照");
  }

  // 3. eco_detect_objects(ARM, OVD SHOE)
  std::cout << "    OVD 检测臂图..." << std::endl;
  auto arm_items = VLMDetect(arm_image.rgb_image.data,
                             {object_name});
  if (arm_items.empty()) {
    throw std::runtime_error("臂图未检测到目标");
  }

  // 4. 自定义跨视图匹配（对应 Python custom_match_obj_views）
  robot_sdk::RobotPos rp;
  if (robot_sdk::eco_RobotPosition(rp).code != 0) {
    throw std::runtime_error("获取机器人位置失败，无法进行跨视图匹配");
  }
  auto matched_bbox = shoe_sorting::CustomMatchObjViews(
      head_image, head_image_ref_bbox, arm_image, arm_items, rp);
  if (matched_bbox.size() < 4) {
    throw std::runtime_error("跨视图匹配失败");
  }

  // 5. eco_pick（bbox.name += "鞋" 触发鞋子抓取模式）
  std::string pick_name = object_name + "鞋";
  std::cout << "    eco_pick: name=" << pick_name << std::endl;

  robot_sdk::BoundingBox bbox;
  bbox.name = pick_name;
  bbox.left_up_x = static_cast<float>(matched_bbox[0]);
  bbox.left_up_y = static_cast<float>(matched_bbox[1]);
  bbox.right_down_x = static_cast<float>(matched_bbox[2]);
  bbox.right_down_y = static_cast<float>(matched_bbox[3]);

  robot_sdk::AccurateGrabData data;
  data.rgb_image = arm_image.rgb_image;
  data.depth_image = arm_image.depth_image;
  data.tf_goal = arm_image.tf_goal;
  data.bbox = bbox;

  robot_sdk::ThrowOnError(robot_sdk::eco_AccurateGrab(data), "抓取");
}

}  // namespace

// ======== ExecutePickAndPlace（对应 Python execute_pick_and_place） ========

void ExecutePickAndPlace(const robot_sdk::ImageQueryNotifyData& head_image,
                         const robot_sdk::ShoesIntentStep& step,
                         const std::vector<std::vector<double>>& front_edge,
                         const std::string& object_name,
                         const std::vector<int>& head_image_ref_bbox,
                         bool no_voice) {
  // ---- 1. 抓取（3 次重试，对应 Python mission_retry(attempts=3, op_name="pick")） ----
  Say("开始抓取", no_voice);                                     // 节点 E1
  std::cout << "    抓取: [" << step.from_bbox[0] << "," << step.from_bbox[1]
            << "," << step.from_bbox[2] << "," << step.from_bbox[3] << "]..."
            << std::endl;
  for (int attempt = 1;; ++attempt) {
    try {
      PickOnce(head_image, step.from_bbox, object_name, head_image_ref_bbox);
      break;
    } catch (const std::exception& e) {
      std::cout << "    抓取尝试 " << attempt << "/3 失败: " << e.what() << std::endl;
      if (attempt < 3) {
        robot_sdk::eco_StopAll(10*1000);
        Say("抓取未成功，正在重试，别着急", no_voice);            // 节点 E2
      }
      if (attempt >= 3) throw;
    }
  }
  Say("抓取成功，开始放置", no_voice);                           // 节点 E3
  std::cout << "    抓取成功" << std::endl;

  // ---- 2. 放置（匹配 Python 放置部分） ----
  double u = (step.to_bbox[0] + step.to_bbox[2]) * 0.5;
  double v = (step.to_bbox[1] + step.to_bbox[3]) * 0.5;

  robot_sdk::Position place_pos;
  try {
    place_pos = robot_sdk::PixelToPoint3D(head_image, u, v);
  } catch (const std::exception&) {
    place_pos = PlacePositionPixelRayGroundZ0(
        head_image.tf_goal, head_image.camera_info_k, u, v);
    std::cout << "    深度回退: 射线 ∩ z=0 → (" << place_pos.x << ", "
              << place_pos.y << ", " << place_pos.z << ")" << std::endl;
  }

  robot_sdk::Orientation orientation =
      robot_sdk::PlaceQuatFromFrontEdge(head_image.tf_goal, front_edge);
  robot_sdk::BoxLength box_length{0.12f, 0.10f, 0.05f};
  const std::string& frame_id = head_image.tf_goal.header.frame_id;

  for (int attempt = 1;; ++attempt) {
    std::cout << "    放置尝试 " << attempt << "/3: (" << place_pos.x << ", "
              << place_pos.y << ", " << place_pos.z << ")..." << std::endl;

    robot_sdk::AccuratePlaceData data;
    data.position = place_pos;
    data.orientation = orientation;
    data.box_length = box_length;
    data.frame_id = frame_id;

    robot_sdk::ErrorCode ec = robot_sdk::eco_AccuratePlace(data);
    if (ec.code == 0) break;

    std::string err = "放置失败: code=" + std::to_string(ec.code) + ", msg=" + ec.msg;
    std::cout << "    放置尝试 " << attempt << "/3 失败: " << err << std::endl;
    if (attempt < 3)
      Say("放置未成功，正在重试，请稍候", no_voice);              // 节点 E4
    if (attempt >= 3) throw std::runtime_error(err);
  }
  Say("放置成功", no_voice);                                     // 节点 E5
  std::cout << "    放置成功" << std::endl;
}

}  // namespace shoe_sorting
