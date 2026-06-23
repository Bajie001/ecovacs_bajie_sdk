#ifndef ROBOT_SDK_ROBOT_INFO_H_
#define ROBOT_SDK_ROBOT_INFO_H_

#include "export.h"
#include "types.h"

#include <functional>
#include <string>
#include <vector>

namespace robot_sdk {

/**
 * 机器状态变化上报回调类型。参数为增量结构 RobotInfoPartial（仅含本次 JSON 中出现的字段）。
 * 连接 server 后，server 会主动推送 event/report/robot_info；SDK 只负责在本地注册回调并分发。
 * 在 SDK 内部工作线程中调用：请勿长时间阻塞。
 */
using RobotInfoReportCallback = std::function<void(const RobotInfoPartial&)>;
using RobotInfoOneshotCallback =
    std::function<void(const ErrorCode&, const RobotInfoPartial&)>;

/**
 * 注册本地 robot_info 上报回调。不会向服务端发送 subscribe；连接成功后 server 主动上报。
 * event/report/robot_info 不按 body.task_id 匹配，凡已注册的订阅队列都会收到同一条上报。
 * @param out_registration_id 输出：本地注册 ID，用于后续 eco_UnregisterRobotInfoCallback。
 * @param on_report 非空回调。
 * @return code==0 表示本地回调注册成功；非 0 见 msg。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_RegisterRobotInfoCallback(
    std::string& out_registration_id,
    RobotInfoReportCallback on_report);

/**
 * 取消本地 robot_info 回调注册。不会向服务端发送 unsubscribe。
 * @param registration_id 与 eco_RegisterRobotInfoCallback 返回的 out_registration_id 一致。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_UnregisterRobotInfoCallback(
    const std::string& registration_id);

/**
 * 单次拉取机器状态（event oneshot）。可按 topics 指定关心的字段。
 * @param topics：
    "pos",  //机器当前坐标
    "furniture", //当前环境语义地图
    "battery", //机器当前电量
    "workState", //工作状态
    "alarm"， //异常状态
    "mapinfo"// slam图
 * @param out 输出：仍为增量语义，仅含服务端返回的字段。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotInfoOneshot(
    const std::vector<std::string>& topics,
    RobotInfoPartial& out);

/**
 * 异步单次拉取机器状态。成功发起后立即返回并写出 out_task_id，完成/超时/取消后在后台线程调用 on_done。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotInfoOneshotAsync(
    const std::vector<std::string>& topics,
    std::string& out_task_id,
    RobotInfoOneshotCallback on_done);

/**
 * 手动刷新 SDK 内部 RobotInfo 缓存中的指定字段。
 * @param topics 如 "battery"、"pos"、"workState" 等；为空时刷新全量。
 * @return code==0 表示 oneshot 拉取成功且已合并进缓存。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_RefreshRobotInfo(
    const std::vector<std::string>& topics);

/**
 * 获取 SDK 内部维护的最新机器状态缓存。
 * Client 建立连接后会自动 oneshot 拉取一次全量 RobotInfo 并缓存；后续 robot_info 上报会按增量字段更新缓存。
 * @param out 输出：最新全量缓存快照。
 * @return code==0 表示已取得缓存；非 0 表示缓存尚未初始化或内部状态不可用。
 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotInfo(RobotInfoData& out);

/** 获取最新缓存中的 workState。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotWorkState(WorkState& out);

/** 获取最新缓存中的 battery。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotBattery(BatteryInfo& out);

/** 获取最新缓存中的 alarm。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotAlarm(std::vector<int>& out);

/** 获取最新缓存中的 pos。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotPosition(RobotPos& out);

/** 获取最新缓存中的 mapinfo。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotMapInfo(MapInfo& out);

/** 获取最新缓存中的 furniture。 */
ROBOT_SDK_PUBLIC ErrorCode eco_RobotFurniture(FurnitureInfo& out);

}  // namespace robot_sdk

#endif  // ROBOT_SDK_ROBOT_INFO_H_
