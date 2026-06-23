#ifndef ROBOT_SDK_PUT_WHERE_H_
#define ROBOT_SDK_PUT_WHERE_H_

#include "export.h"
#include "types.h"
#include <memory>
#include <string>
#include <vector>

namespace robot_sdk {

/**
 * 推荐放置服务：根据场景图与物体图等推荐放置区域。
 * 默认基地址为 ``http://$ECO_ROBOT_HOST:9527``。
 * @return 成功时返回解析后的 PutWhereResponse；失败可能为 nullptr，需结合项目约定检查。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<PutWhereResponse> eco_PutWhere(
    const PutWhereRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<PutWhereResponse> eco_PutWhere(
    const PutWhereRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 简化版：成功时 final_answer 输出。
 */
ROBOT_SDK_PUBLIC bool eco_PutWhereSimple(
    const PutWhereRequest& req,
    std::string& final_answer,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_PutWhereSimple(
    const PutWhereRequest& req,
    std::string& final_answer,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

}  // namespace robot_sdk

#endif
