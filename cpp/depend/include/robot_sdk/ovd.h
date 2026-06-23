#ifndef ROBOT_SDK_OVD_H_
#define ROBOT_SDK_OVD_H_

#include "export.h"
#include "types.h"
#include <memory>
#include <string>

namespace robot_sdk {

/**
 * 开放词汇检测（OVD / SHOE）：POST /ovd 或 /shoe。
 * 默认基地址为 ``http://$ECO_ROBOT_HOST:30081``。
 *
 * OVD 端点会在请求前对 labels 做预处理：调用 yolo_label 服务进行父类扩充与模糊对齐，
 * yolo_label 默认基地址为 ``http://$ECO_ROBOT_HOST:30005``。
 * 如果通过 base_url 指定 OVD 服务地址，yolo_label 会复用该地址的 host 并切换到端口 30005。
 * 服务不可用时退化为本地父类扩充；随后补充子类对应父类并过滤到 OVD_LABELS 白名单。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<DetectObjectsResponse> eco_DetectObjects(
    const DetectObjectsRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<DetectObjectsResponse> eco_DetectObjects(
    const DetectObjectsRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_DetectObjectsSimple(
    const DetectObjectsRequest& req,
    std::vector<ObjectDetection>& items,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_DetectObjectsSimple(
    const DetectObjectsRequest& req,
    std::vector<ObjectDetection>& items,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

}  // namespace robot_sdk

#endif  // ROBOT_SDK_OVD_H_
