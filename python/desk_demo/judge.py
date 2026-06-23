#!/usr/bin/env python3
"""
评判工作流 (Judge Workflow)

使用 bajie_sdk ``eco_vlm_judge``（VLM 整理效果评估），替代直连 model_service HTTP。
"""

import logging
from typing import Any, Dict, Optional, Tuple

from bajie_sdk import BajieRobot

from ref_utils import new_task_id, rgb_image_dict_to_rgb_ndarray, write_json
from retry_utils import ERR_JUDGE_FAILED, error_tag, retry_run

logger = logging.getLogger(__name__)


def _resolve_user_input_for_judge(user_input: str, plan_result: Dict[str, Any]) -> str:
    text = (user_input or "").strip()
    if text:
        return text
    text = (plan_result.get("user_input") or "").strip()
    if text:
        return text
    text = (plan_result.get("user_input_from_service") or "").strip()
    if text:
        return text
    return "桌面整理"


def run_judge_workflow(
    robot: BajieRobot,
    before_image_data: Dict[str, Any],
    after_image_data: Dict[str, Any],
    plan_result: Dict[str, Any],
    *,
    user_input: str = "",
    output_dir: str = "outputs",
    timeout_sec: float = 150.0,
) -> Tuple[bool, Optional[Dict[str, Any]]]:
    """调用 ``eco_vlm_judge`` 对整理结果进行评判。

    说明：SDK 接口仅需「整理后图像 + user_input」。参数 ``before_image_data`` 仅为与旧 HTTP
    接口对齐保留，当前不参与 SDK 调用（若后续 SDK 扩展对比能力可再接）。

    Args:
        robot: 已连接的 BajieRobot。
        before_image_data: 整理前图像 dict（兼容保留）。
        after_image_data: 整理后图像 dict（须含 ``rgb_image``）。
        plan_result: 规划结果（用于解析默认 ``user_input``、写入摘要）。
        user_input: 用户意图；若为空则从 ``plan_result`` 推断。
        output_dir: 输出目录。
        timeout_sec: SDK 超时（秒）。

    Returns:
        (success, judge_result)，含 is_tidy、reason、score。
    """
    task_id = new_task_id()

    logger.info("=" * 50)
    logger.info("评判工作流 (eco_vlm_judge)")
    logger.info("=" * 50)

    try:
        _ = before_image_data  # 与旧 HTTP 入参对齐；SDK eco_vlm_judge 当前不使用整理前图

        tidy_rgb = rgb_image_dict_to_rgb_ndarray(after_image_data.get("rgb_image"))
        if tidy_rgb is None:
            logger.error("%s 整理后 rgb_image 无法转为 ndarray，无法进行评判", error_tag(ERR_JUDGE_FAILED))
            return False, None

        ui = _resolve_user_input_for_judge(user_input, plan_result)
        logger.info("调用 eco_vlm_judge，user_input=%r timeout_sec=%s", ui, timeout_sec)
        result = retry_run(
            lambda: robot.eco_vlm_judge(tidy_rgb, ui, timeout=timeout_sec),
            name="eco_vlm_judge",
        )

        is_tidy = bool(result.is_tidy)
        reason = str(result.reason or "")
        score = float(result.score if result.score is not None else 0.0)

        judge_output: Dict[str, Any] = {
            "task_id": task_id,
            "status": "success",
            "is_tidy": is_tidy,
            "reason": reason,
            "score": score,
            "judge_backend": "eco_vlm_judge",
            "vlm_user_input": ui,
            "intent_steps_count": len(plan_result.get("intent_steps", []) or []),
        }

        output_file = f"{output_dir}/judge_result.json"
        write_json(output_file, judge_output)
        logger.info(f"评判完成，is_tidy={is_tidy}, score={score}")
        logger.info(f"评判原因: {reason}")
        logger.info(f"评判结果已保存到: {output_file}")

        return True, judge_output

    except Exception as e:
        logger.exception("%s 评判工作流执行异常: %s", error_tag(ERR_JUDGE_FAILED), e)
        return False, None
