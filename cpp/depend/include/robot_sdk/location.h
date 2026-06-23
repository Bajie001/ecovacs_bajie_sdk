#ifndef ROBOT_SDK_LOCATION_H_
#define ROBOT_SDK_LOCATION_H_

#include "export.h"
#include "types.h"
#include <memory>
#include <string>
#include <vector>

namespace robot_sdk {

/**
 * VLM 位置服务：根据场景图与占位词等，返回物品列表（与通过 Client 下发的技能独立）。
 * 默认基地址为 ``http://$ECO_ROBOT_HOST:9527``。
 * @param req 含 placeholder、图像尺寸与 base64 图像等。
 * @return 成功时返回解析后的 LocationResponse；失败可能为 nullptr，需结合项目约定检查。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<LocationResponse> eco_VLMLocation(
    const LocationRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<LocationResponse> eco_VLMLocation(
    const LocationRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 简化版：将物品与未知物品填入输出向量，不返回完整 Response 包装。
 */
ROBOT_SDK_PUBLIC bool eco_VLMLocationSimple(
    const LocationRequest& req,
    std::vector<ItemData>& items,
    std::vector<ItemData>& unknown_items,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_VLMLocationSimple(
    const LocationRequest& req,
    std::vector<ItemData>& items,
    std::vector<ItemData>& unknown_items,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

}  // namespace robot_sdk

#endif
