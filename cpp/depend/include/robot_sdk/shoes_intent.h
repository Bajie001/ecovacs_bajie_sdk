#ifndef ROBOT_SDK_SHOES_INTENT_H_
#define ROBOT_SDK_SHOES_INTENT_H_

#include "export.h"
#include "types.h"
#include <memory>
#include <string>
#include <vector>

namespace robot_sdk {

/**
 * 鞋子整理方案：move_all=true POST /v038/test/shoes_intent/plan；
 * move_all=false POST /v038/test/shoes_intent/check。
 * 默认基地址为 ``http://$ECO_ROBOT_HOST:8555``。
 * @return 成功时 code == 0，业务失败时 code != 0；网络/解析失败时 nullptr。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<ShoesIntentPlanResponse> eco_ShoesIntentPlan(
    const ShoesIntentPlanRequest& req,
    bool move_all,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<ShoesIntentPlanResponse> eco_ShoesIntentPlan(
    const ShoesIntentPlanRequest& req,
    bool move_all,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 简化版：成功时填充 steps；后端返回 final_bboxes 时同步填充；失败时 error_msg。
 */
ROBOT_SDK_PUBLIC bool eco_ShoesIntentPlanSimple(
    const ShoesIntentPlanRequest& req,
    bool move_all,
    std::vector<ShoesIntentStep>& steps,
    std::vector<ShoesIntentFinalBbox>& final_bboxes,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_ShoesIntentPlanSimple(
    const ShoesIntentPlanRequest& req,
    bool move_all,
    std::vector<ShoesIntentStep>& steps,
    std::vector<ShoesIntentFinalBbox>& final_bboxes,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

}  // namespace robot_sdk

#endif
