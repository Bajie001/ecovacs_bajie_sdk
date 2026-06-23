#ifndef ROBOT_SDK_POSE_TOOLS_H_
#define ROBOT_SDK_POSE_TOOLS_H_

#include "export.h"
#include "types.h"
#include <memory>
#include <string>
#include <vector>

namespace robot_sdk {

/**
 * 放置线/面投影：POST /proj_place_line_and_plane。
 * 默认基地址为 ``http://$ECO_ROBOT_HOST:7001``。
 *
 * ProjectPlaceLineAndPlaneView 通常可由 ImageQueryNotifyData 填充：
 * - depth_image.data 需为深度 PNG base64；
 * - camera_info_k 为 3x3 相机内参（9 个数）；
 * - tf_goal 使用 pose.position / pose.orientation 作为协议 tf 字段。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<ProjectPlaceLineAndPlaneResponse>
eco_ProjectPlaceLineAndPlane(
    const ProjectPlaceLineAndPlaneRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<ProjectPlaceLineAndPlaneResponse>
eco_ProjectPlaceLineAndPlane(
    const ProjectPlaceLineAndPlaneRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 简化版：成功时填充 rows；失败时 error_msg。
 */
ROBOT_SDK_PUBLIC bool eco_ProjectPlaceLineAndPlaneSimple(
    const ProjectPlaceLineAndPlaneRequest& req,
    std::vector<ProjectPlaceLineAndPlaneRow>& rows,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_ProjectPlaceLineAndPlaneSimple(
    const ProjectPlaceLineAndPlaneRequest& req,
    std::vector<ProjectPlaceLineAndPlaneRow>& rows,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

}  // namespace robot_sdk

#endif
