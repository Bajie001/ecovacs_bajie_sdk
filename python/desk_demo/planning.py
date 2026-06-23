#!/usr/bin/env python3
"""
桌面意图规划模块 (Plan Workflow)

使用 bajie_sdk ``eco_vlm_desk_sort_plan``（``VlmPlanRequest``：图像 + 用户语句 + 感知列表），
替代直连 model_service 的 ``/desk_intent/plan/v2`` HTTP。
"""

import logging
from typing import Any, Dict, List, Optional, Tuple

from bajie_sdk import BajieRobot, NamedBBox, VlmPlanRequest

from errors import RobotMissionError
from grab_items import build_memory_input_for_plan, load_grab_list
from ref_utils import new_task_id, rgb_image_dict_to_rgb_ndarray, write_json
from retry_utils import ERR_PLAN_FAILED, classify_error_code, error_tag, retry_run

logger = logging.getLogger(__name__)

PLAN_SDK_TIMEOUT_SEC = 360.0


def _perception_dicts_to_named_bbox(perception: List[Dict[str, Any]]) -> List[NamedBBox]:
    """将落盘 / HTTP 时代的 ``{name, bbox[, id]}`` 转为 ``NamedBBox``。"""
    out: List[NamedBBox] = []
    for i, p in enumerate(perception):
        if not isinstance(p, dict):
            continue
        bbox_raw = p.get("bbox")
        if not isinstance(bbox_raw, (list, tuple)) or len(bbox_raw) != 4:
            continue
        tid = p.get("id", i)
        try:
            tid_i = int(tid)
        except (TypeError, ValueError):
            tid_i = i
        name = str(p.get("name") or "")
        bx = tuple(int(x) for x in bbox_raw)
        out.append(NamedBBox(id=tid_i, name=name, bbox=bx))
    return out


def desk_intent_plan(
    robot: BajieRobot,
    image_data: Dict[str, Any],
    perception: List[Dict[str, Any]],
    user_input: str = "",
    output_dir: str = "outputs",
    judge_input: Optional[str] = None,
    memory_input: str = "",
    grab_items_path: str = "",
) -> Tuple[bool, Optional[Dict[str, Any]]]:
    """调用 ``eco_vlm_desk_sort_plan`` 进行桌面意图规划。

    Args:
        robot: 已连接的 BajieRobot。
        image_data: 头部相机图像数据（包含 rgb_image 字段）。
        perception: 感知结果列表（含 name、bbox；可选 id）。
        user_input: 用户意图补充信息（如 "只整理文具，其他不动"）。
        output_dir: 输出目录。
        judge_input: 上轮评判 reason，传给 SDK ``judge_input``。
        memory_input: 用户记忆经验（不含可抓清单；清单由 grab_items.md 自动前缀）。
        grab_items_path: 可抓清单 md 路径，空则用 desk_demo/grab_items.md。

    Returns:
        (success, plan_result)。plan_result 含 intent_steps；另含 plan、user_input_from_service、
        plan_backend、plan_response_format 等字段。
    """
    task_id = new_task_id()

    logger.info("=" * 50)
    logger.info("桌面意图规划 (eco_vlm_desk_sort_plan)")
    logger.info("  用户输入: %s", user_input)
    logger.info("  感知物体数: %s", len(perception))
    logger.info("=" * 50)

    try:
        rgb_arr = rgb_image_dict_to_rgb_ndarray(image_data.get("rgb_image"))
        if rgb_arr is None:
            logger.error("%s rgb_image 无法转为 ndarray，无法规划", error_tag(ERR_PLAN_FAILED))
            return False, None

        named = _perception_dicts_to_named_bbox(perception)
        ji = str(judge_input) if judge_input is not None else ""

        grab_list = load_grab_list(grab_items_path.strip() or None)
        memory_sent = build_memory_input_for_plan(
            user_memory_experience=memory_input,
            grab_list=grab_list,
        )
        if memory_input.strip():
            logger.info("  用户 memory_input(经验): %s", memory_input.strip()[:200])
        logger.info("  实际 memory_input(len=%s): %s", len(memory_sent), memory_sent[:300])

        req = VlmPlanRequest(
            image=rgb_arr,
            user_input=str(user_input or ""),
            perception=named,
            judge_input=ji,
            memory_input=memory_sent,
        )
        logger.info("调用 eco_vlm_desk_sort_plan timeout_sec=%s", PLAN_SDK_TIMEOUT_SEC)
        vlm_resp = retry_run(
            lambda: robot.eco_vlm_desk_sort_plan(req, timeout=PLAN_SDK_TIMEOUT_SEC),
            name="eco_vlm_desk_sort_plan",
        )

        intent_steps: List[Dict[str, Any]] = []
        for step in vlm_resp.steps:
            step_dict: Dict[str, Any] = {
                "step_id": step.step_id,
                "from_bbox": list(step.from_bbox),
                "to_bbox": list(step.to_bbox),
                "is_container": bool(step.is_container),
            }
            if step.reason:
                step_dict["reason"] = step.reason
            if step.placement_type:
                step_dict["placement_type"] = step.placement_type
            if step.container_name is not None:
                step_dict["container_name"] = step.container_name
            intent_steps.append(step_dict)
            logger.info(
                "  步骤 %s: from_bbox=%s, to_bbox=%s, is_container=%s",
                step_dict["step_id"],
                step_dict["from_bbox"],
                step_dict["to_bbox"],
                step_dict["is_container"],
            )

        if not intent_steps:
            logger.warning("%s 规划结果为空，未生成任何步骤", error_tag(ERR_PLAN_FAILED))
            return False, None

        plan_output: Dict[str, Any] = {
            "task_id": task_id,
            "status": "success",
            "user_input": user_input,
            "intent_steps": intent_steps,
            "steps_count": len(intent_steps),
            "plan_response_format": "vlm_sdk",
            "plan_backend": "eco_vlm_desk_sort_plan",
        }
        if getattr(vlm_resp, "reason", ""):
            plan_output["plan"] = vlm_resp.reason
        if getattr(vlm_resp, "user_input", ""):
            plan_output["user_input_from_service"] = vlm_resp.user_input

        output_file = f"{output_dir}/plan_result.json"
        write_json(output_file, plan_output)
        logger.info("规划结果已保存到: %s", output_file)
        logger.info("规划完成，共 %s 个步骤", len(intent_steps))

        return True, plan_output

    except Exception as e:
        code = classify_error_code("eco_vlm_desk_sort_plan", e)
        logger.exception("%s 桌面意图规划执行异常: %s", error_tag(code), e)
        raise RobotMissionError(f"{error_tag(code)} 桌面意图规划执行异常: {e}") from e
