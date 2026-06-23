#ifndef ROBOT_SDK_DESK_INTENT_H_
#define ROBOT_SDK_DESK_INTENT_H_

#include "export.h"
#include "types.h"
#include <memory>
#include <string>
#include <vector>

namespace robot_sdk {
/**
 * 桌面观测角度推荐：多张多角度图片 → 桌面视野最大的头部摄像机角度。
 * POST ``/v038/test/desk_intent/pre_perception/v1``，默认基地址为 ``http://$ECO_ROBOT_HOST:8555``。
 * @return 成功时 code == 0；业务失败时 code != 0；网络/解析失败时 nullptr。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentPrePerceptionResponse> eco_DeskIntentPrePerception(
    const DeskIntentPrePerceptionRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentPrePerceptionResponse> eco_DeskIntentPrePerception(
    const DeskIntentPrePerceptionRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 简化版：成功时填充 angle；失败时 error_msg。
 */
ROBOT_SDK_PUBLIC bool eco_DeskIntentPrePerceptionSimple(
    const DeskIntentPrePerceptionRequest& req,
    double& angle,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_DeskIntentPrePerceptionSimple(
    const DeskIntentPrePerceptionRequest& req,
    double& angle,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 桌面感知：单张桌面图片 → 桌面及物品名称与 bbox。
 * POST ``/v038/test/desk_intent/perception/v1``，默认基地址为 ``http://$ECO_ROBOT_HOST:8555``。
 * @return 成功时 code == 0；业务失败时 code != 0；网络/解析失败时 nullptr。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentPerceptionResponse> eco_DeskIntentPerception(
    const DeskIntentPerceptionRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentPerceptionResponse> eco_DeskIntentPerception(
    const DeskIntentPerceptionRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 简化版：成功时填充 objects；失败时 error_msg。
 */
ROBOT_SDK_PUBLIC bool eco_DeskIntentPerceptionSimple(
    const DeskIntentPerceptionRequest& req,
    std::vector<DeskIntentPerceptionObject>& objects,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_DeskIntentPerceptionSimple(
    const DeskIntentPerceptionRequest& req,
    std::vector<DeskIntentPerceptionObject>& objects,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 桌面物品摆放意图：图片 + 用户需求 + 感知结果 + 可选评估/记忆 → 带语义抓放步骤。
 * POST ``/v038/test/desk_intent/plan/v2``，默认基地址为 ``http://$ECO_ROBOT_HOST:8555``。
 * @return 成功时 code == 0；业务失败时 code != 0；网络/解析失败时 nullptr。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentPlanResponse> eco_DeskIntentPlan(
    const DeskIntentPlanRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentPlanResponse> eco_DeskIntentPlan(
    const DeskIntentPlanRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 简化版：成功时填充 steps；失败时 error_msg。
 */
ROBOT_SDK_PUBLIC bool eco_DeskIntentPlanSimple(const DeskIntentPlanRequest& req,
    std::vector<DeskIntentPlanStep>& steps,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_DeskIntentPlanSimple(const DeskIntentPlanRequest& req,
    std::vector<DeskIntentPlanStep>& steps,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 桌面整理结果判定：整理后图片 + 用户原始需求 → 是否符合整理需求、原因与评分。
 * POST ``/v038/test/judge/action/v2``，默认基地址为 ``http://$ECO_ROBOT_HOST:8555``。
 * @return 成功时 code == 0；业务失败时 code != 0；网络/解析失败时 nullptr。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentJudgeActionResponse> eco_DeskIntentJudgeAction(
    const DeskIntentJudgeActionRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentJudgeActionResponse> eco_DeskIntentJudgeAction(
    const DeskIntentJudgeActionRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 简化版：成功时填充 is_tidy / reason / score；失败时 error_msg。
 */
ROBOT_SDK_PUBLIC bool eco_DeskIntentJudgeActionSimple(
    const DeskIntentJudgeActionRequest& req,
    bool& is_tidy,
    std::string& reason,
    double& score,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_DeskIntentJudgeActionSimple(
    const DeskIntentJudgeActionRequest& req,
    bool& is_tidy,
    std::string& reason,
    double& score,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 头部可见 bbox 映射到手部相机图像中的 bbox。
 * POST ``/v038/test/desk_intent/match``，默认基地址为 ``http://$ECO_ROBOT_HOST:8555``。
 * @return 成功时 code == 0 且 ``bbox`` 为 4 元；业务失败时 code != 0；网络/解析失败时 nullptr。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentMatchResponse> eco_DeskIntentMatch(
    const DeskIntentMatchRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<DeskIntentMatchResponse> eco_DeskIntentMatch(
    const DeskIntentMatchRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 简化版：成功时 out_bbox 为 4 元 ``[x1,y1,x2,y2]``；失败时 error_msg。
 */
ROBOT_SDK_PUBLIC bool eco_DeskIntentMatchSimple(const DeskIntentMatchRequest& req,
    std::vector<int>& out_bbox,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_DeskIntentMatchSimple(const DeskIntentMatchRequest& req,
    std::vector<int>& out_bbox,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

}  // namespace robot_sdk

#endif
