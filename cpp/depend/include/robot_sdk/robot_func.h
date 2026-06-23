#ifndef ROBOT_SDK_ROBOT_FUNC_H_
#define ROBOT_SDK_ROBOT_FUNC_H_

#include "export.h"
#include "robot_info.h"
#include "types.h"
#include <atomic>
#include <functional>
#include <string>
#include <vector>

namespace robot_sdk {

/** mission 类任务默认超时时间（毫秒）；传入 timeout_ms 须 >= 0，0 表示无限等待；通过 SDK 暂停任务时，暂停期间不计入任务完成超时。 */
constexpr int kDefaultTaskCompletionTimeoutMs = 180000;

/** 仅等待任务应答的默认超时时间（毫秒）；传入 timeout_ms 须 >= 0，0 表示无限等待。 */
constexpr int kDefaultTaskResponseTimeoutMs = 10000;

/**
 * 设置协作式取消标志指针。库内任务等待循环会轮询该标志；置为 true 时尽快结束等待并返回（如 code=4）。
 * @param flag 指向外部 atomic<bool>，可为 nullptr 表示不启用。通常与 Ctrl+C 信号处理配合。
 */
ROBOT_SDK_PUBLIC void eco_SetCancellationFlag(std::atomic<bool>* flag);

/**
 * 暂停/恢复/取消某任务，发送指令成功则返回success。
 * 通过 eco_PauseMission/eco_ResumeMission 控制的任务，SDK 等待完成时的 timeout_ms 会随暂停/恢复冻结和继续。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_PauseMission(const std::string& task_id);
ROBOT_SDK_PUBLIC ErrorCode eco_ResumeMission(const std::string& task_id);
ROBOT_SDK_PUBLIC ErrorCode eco_CancelMission(const std::string& task_id);

// 控制机器语音播报
ROBOT_SDK_PUBLIC ErrorCode eco_Speech(const std::string& text);

using RobotTaskCallback = std::function<void(const ErrorCode&)>;
using SemanticMapManagerCallback =
    std::function<void(const ErrorCode&, const SemanticMapManagerNotifyData&)>;
using ImageQueryCallback = std::function<void(const ErrorCode&, const ImageQueryNotifyData&)>;
using GetPoseCallback = std::function<void(const ErrorCode&, const GetPoseNotifyData&)>;
using SearchCallback = std::function<void(const ErrorCode&, const BoxData&)>;

// --- 3.2 mission：导航类 ---

/** 自动建图（auto_map_create）。阻塞直至 mission finish 或超时/取消。 */
ROBOT_SDK_PUBLIC ErrorCode eco_AutoMapCreate(
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_AutoMapCreateAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 语义建图（semantic_map_create），需传入区域列表。 */
ROBOT_SDK_PUBLIC ErrorCode eco_SemanticMapCreate(
    const SemanticMapCreateData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_SemanticMapCreateAsync(
    const SemanticMapCreateData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 底盘移动：move_distance（米）、move_angle（弧度）。 */
ROBOT_SDK_PUBLIC ErrorCode eco_ChassisMove(
    const ChassisMoveData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_ChassisMoveAsync(
    const ChassisMoveData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/**
 * 语义地图管理（semantic_map_manager）：增删改查等。
 * @param notify_out 非空时从任务返回中解析 objects_info；优先读取 finish，并兼容旧 notify。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_SemanticMapManager(
    const SemanticMapManagerData& data,
    SemanticMapManagerNotifyData* notify_out,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_SemanticMapManagerAsync(
    const SemanticMapManagerData& data,
    std::string& out_task_id,
    SemanticMapManagerCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 定点导航（point_navigation），x/y 米、yaw 弧度。 */
ROBOT_SDK_PUBLIC ErrorCode eco_PointNavigation(
    const PointNavigationData& nav_data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_PointNavigationAsync(
    const PointNavigationData& nav_data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 语义导航（semantic_navigation）。 */
ROBOT_SDK_PUBLIC ErrorCode eco_SemanticNavigation(
    const SemanticNavigationData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_SemanticNavigationAsync(
    const SemanticNavigationData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 下桩（dock_down）。 */
ROBOT_SDK_PUBLIC ErrorCode eco_DockDown(
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_DockDownAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 重定位（map_relocation）。 */
ROBOT_SDK_PUBLIC ErrorCode eco_MapRelocation(
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_MapRelocationAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 回充（recharge）。 */
ROBOT_SDK_PUBLIC ErrorCode eco_Recharge(
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_RechargeAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

// --- 传感器 / 感知 ---

/**
 * 获取图像（image_query）。
 * @param data camera_type：CameraType::ARM 手臂相机，CameraType::HEAD 头部相机。
 * @param result 任务返回中的 RGB/深度/tf/内参等；优先读取 finish，并兼容旧 notify。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_ImageQuery(
    const ImageQueryData& data,
    ImageQueryNotifyData& result,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_ImageQuery(
    CameraType camera_type,
    ImageQueryNotifyData& result,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_ImageQueryAsync(
    const ImageQueryData& data,
    std::string& out_task_id,
    ImageQueryCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_ImageQueryAsync(
    CameraType camera_type,
    std::string& out_task_id,
    ImageQueryCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/**
 * 物品 6D pose（get_pose）：根据 bbox 与图像、tf_map 等计算。
 * @param notify_out 任务返回中的 pose_results 列表；优先读取 finish，并兼容旧 notify。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_GetPose(
    const GetPoseData& data,
    GetPoseNotifyData& notify_out,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_GetPoseAsync(
    const GetPoseData& data,
    std::string& out_task_id,
    GetPoseCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

// --- 高层控制 ---

/** 找人（find_person）。 */
ROBOT_SDK_PUBLIC ErrorCode eco_FindPerson(
    const FindPersonData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_FindPersonAsync(
    const FindPersonData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/**
 * 寻找物体（search）。result 来自任务返回中的单组 6D 信息；优先读取 finish，并兼容旧 notify。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_Search(
    const SearchData& search_data,
    BoxData& result,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_SearchAsync(
    const SearchData& search_data,
    std::string& out_task_id,
    SearchCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 机身准备姿态（robot_prepare_pose），任务开始时调用。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotPreparePose(
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_RobotPreparePoseAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 机身结束姿态（robot_ending_pose）。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotEndingPose(
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_RobotEndingPoseAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 机身高度控制（robot_height_ctrl），开机后建议先执行一次 0 位控制以标定初始位置。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotHeightCtrl(
    const RobotHeightCtrlData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_RobotHeightCtrlAsync(
    const RobotHeightCtrlData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 机身头部控制（robot_head_ctrl），开机后建议先执行一次 0 位控制以标定初始位置。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotHeadCtrl(
    const RobotHeadCtrlData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_RobotHeadCtrlAsync(
    const RobotHeadCtrlData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 机身手臂控制（robot_arm_ctrl）：归位、夹爪开合、开箱姿态、维护姿态等。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotArmCtrl(
    const RobotArmCtrlData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_RobotArmCtrlAsync(
    const RobotArmCtrlData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 机械臂 pose 控制（robot_pose_ctrl）：机械臂到达指定 pose。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotPoseCtrl(
    const RobotPoseCtrlData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_RobotPoseCtrlAsync(
    const RobotPoseCtrlData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 观测对准（hand_observe / head_observe），通过 target 指定手部或头部。 */
ROBOT_SDK_PUBLIC ErrorCode eco_LookTo(
    LookToTarget target,
    const BoxData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_LookToAsync(
    LookToTarget target,
    const BoxData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 精准抓取（accurate_grab），需图像与 bbox。 */
ROBOT_SDK_PUBLIC ErrorCode eco_AccurateGrab(
    const AccurateGrabData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_AccurateGrabAsync(
    const AccurateGrabData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 精准放置（accurate_place）。object 可空，仅填位姿时 item 留空。 */
ROBOT_SDK_PUBLIC ErrorCode eco_AccuratePlace(
    const AccuratePlaceData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_AccuratePlaceAsync(
    const AccuratePlaceData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 推荐放置（semantic_place），头部相机与 meta_data。 */
ROBOT_SDK_PUBLIC ErrorCode eco_SemanticPlace(
    const SemanticPlaceData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_SemanticPlaceAsync(
    const SemanticPlaceData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/**
 * 抓取（臂视角复核）：先按参考视角 bbox 做手部观测，再用手臂相机重拍、OVD 重检、
 * 跨视角匹配，最后执行 accurate_grab。
 * @param view 参考视角图像，通常为头部相机 eco_ImageQuery(CameraType::HEAD) 结果。
 * @param object_bbox 参考视角目标框；name 用作手臂图 OVD label，空串时回退 "object"。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_PickWithArmReview(
    const ImageQueryNotifyData& view,
    const BoundingBox& object_bbox,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_PickWithArmReviewAsync(
    const ImageQueryNotifyData& view,
    const BoundingBox& object_bbox,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/**
 * 放置：按头部视角像素点/框执行语义推荐放，或重投影为 3D 后执行 accurate_place。
 * @param view 头部相机 RGBD 视角。
 * @param place_ref 长度 2 表示像素点 [x,y]；长度 4 表示像素框 [x1,y1,x2,y2]，内部取中心点。
 * @param recapture_place false：semantic_place；true：像素反投影后 accurate_place。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_PlaceWithView(
    const ImageQueryNotifyData& view,
    const std::vector<float>& place_ref,
    bool recapture_place = false,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_PlaceWithViewAsync(
    const ImageQueryNotifyData& view,
    const std::vector<float>& place_ref,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_PlaceWithViewAsync(
    const ImageQueryNotifyData& view,
    const std::vector<float>& place_ref,
    bool recapture_place,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/**
 * 容器内推荐放：取 carrier_bbox 前 4 项作为容器框，按 semantic_place 的“里”协议执行。
 * @param view 头部相机 RGBD 视角。
 * @param carrier_bbox 容器框，至少 4 个数，按 [x1,y1,x2,y2] 使用。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_PlaceIn(
    const ImageQueryNotifyData& view,
    const std::vector<float>& carrier_bbox,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_PlaceInAsync(
    const ImageQueryNotifyData& view,
    const std::vector<float>& carrier_bbox,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);

/** 取消所有任务（stop_all）。 */
ROBOT_SDK_PUBLIC ErrorCode eco_StopAll(
    int timeout_ms = kDefaultTaskResponseTimeoutMs);
ROBOT_SDK_PUBLIC ErrorCode eco_StopAllAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskResponseTimeoutMs);

}  // namespace robot_sdk

#endif
