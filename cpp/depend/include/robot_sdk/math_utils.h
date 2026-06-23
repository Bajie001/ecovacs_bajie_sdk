#ifndef ROBOT_SDK_MATH_UTILS_H_
#define ROBOT_SDK_MATH_UTILS_H_

#include "export.h"
#include "types.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace robot_sdk {

/// Quaternion (x,y,z,w) → 3×3 旋转矩阵（列主序，9 元素）。
ROBOT_SDK_PUBLIC std::vector<double> QuatToRotMat(const robot_sdk::Orientation& q);

/// 3×3 旋转矩阵（列主序 9 元素）→ Quaternion (x,y,z,w)。
/// 匹配 Python execution._rotmat_to_quatf。
ROBOT_SDK_PUBLIC robot_sdk::Orientation RotMatToQuat(const std::vector<double>& rot_mat);

/// front_edge → 水平 X 轴方向向量（父系）。
/// 匹配 Python execution._front_edge_horizontal_x_axis。
ROBOT_SDK_PUBLIC std::vector<double> FrontEdgeHorizontalXAxis(
    const robot_sdk::PoseStamped& tf_goal,
    const std::vector<std::vector<double>>& front_edge);

/// front_edge → 放置姿态四元数。
/// 匹配 Python execution._place_quat_from_front_edge。
ROBOT_SDK_PUBLIC robot_sdk::Orientation PlaceQuatFromFrontEdge(
    const robot_sdk::PoseStamped& tf_goal,
    const std::vector<std::vector<double>>& front_edge);

/// 像素 → 3D（深度优先，匹配 Python pixel_to_point_3d）。
/// 先读深度值，若有效则用内参反投影；失败时抛 std::runtime_error。
ROBOT_SDK_PUBLIC robot_sdk::Position PixelToPoint3D(
    const robot_sdk::ImageQueryNotifyData& head_image, double u, double v);

/// ErrorCode 非 0 时抛出 std::runtime_error（含 desc + code + msg）。
inline void ThrowOnError(const robot_sdk::ErrorCode& ec, const std::string& desc) {
  if (ec.code == 0) return;
  throw std::runtime_error(desc + " 失败: code=" + std::to_string(ec.code) +
                           ", msg=" + ec.msg);
}

/// Base64 解码。
ROBOT_SDK_PUBLIC std::vector<uint8_t> Base64Decode(const std::string& input);

/// Base64 编码。
ROBOT_SDK_PUBLIC std::string Base64Encode(const std::vector<uint8_t>& input);

}  // namespace robot_sdk

#endif
