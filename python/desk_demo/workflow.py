#!/usr/bin/env python3
"""
桌面整理工作流

完整工作流程:
1. 重定位 (eco_relocate)
2. 准备姿态 (eco_prepareRobotPose)
3. 感知1 (perception_workflow)
   - 语义导航到桌子
   - 升高机身
   - （可选）多角度采样 + eco_vlm_suggest_angle + eco_setRobotHead
   - 头部相机拍照
   - eco_vlm_perception 感知物体
4. 规划 (eco_vlm_desk_sort_plan)
5. 执行工作流 (execution_workflow)
   - 计算6D姿态
   - 循环处理每个物品:
     - 位置自适应 -> 手臂拍照 -> bbox -> 抓取 -> 放置
6. 感知2 (perception_workflow)
7. 评判 (eco_vlm_judge)
8. 若评判不通过 && 循环次数 < max_loops:
   - 使用感知2的结果重新规划 -> 执行 -> 感知N -> 评判
9. 结束姿态 (eco_finishRobotPose)
"""

from __future__ import annotations

import asyncio
import logging
import os
from typing import Dict, Optional

from bajie_sdk import BajieRobot, RGBDViewWithPose, RobotArmCtrlMode

from errors import RobotMissionError
from execution import run_execution_workflow
from judge import run_judge_workflow
# from memory.api_trace_logger import append_api_trace
from models import DeskConfig
from perception import run_perception_workflow
from planning import desk_intent_plan
from ref_utils import ensure_dir, write_json
from retry_utils import (
    ERR_ARM_HOME_FAILED,
    ERR_CONNECT_FAILED,
    ERR_EXECUTION_FAILED,
    ERR_FINISH_POSE_FAILED,
    ERR_JUDGE_FAILED,
    ERR_PERCEPTION_FAILED,
    ERR_PLAN_FAILED,
    ERR_PREPARE_POSE_FAILED,
    ERR_RELOCATE_FAILED,
    error_tag,
    mission_failed,
    retry_run,
)
from speech import (
    SPEECH_DONE,
    SPEECH_EXECUTION,
    SPEECH_JUDGING,
    SPEECH_PLANNING,
    SPEECH_PREPARE_POSE,
    SPEECH_RELOCATING,
    speak,
)

logger = logging.getLogger(__name__)

OUTPUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "outputs")


def _check_status(status, operation: str) -> None:
    code = getattr(status.error_info, "code", 0)
    msg = getattr(status.error_info, "message", "")
    if code != 0:
        raise RobotMissionError(f"{operation} failed: code={code}, msg={msg}")


def _arm_home(robot: BajieRobot) -> None:
    logger.info("=" * 50)
    logger.info("执行后收回机械臂")
    logger.info("=" * 50)

    def _home() -> None:
        st = robot.eco_setRobotArmMode(RobotArmCtrlMode.ARM_HOME, timeout_sec=30.0)
        fail = mission_failed(st)
        if fail:
            raise RobotMissionError(f"{error_tag(ERR_ARM_HOME_FAILED)} 机械臂归位失败: {fail}")

    retry_run(_home, name="eco_setRobotArmMode(ARM_HOME)")


def desk_workflow(
    *,
    ws_url: Optional[str] = None,
    cfg: DeskConfig,
    output_dir: str = OUTPUT_DIR,
) -> None:
    """运行完整的桌面整理工作流（支持评判循环）"""
    ensure_dir(output_dir)
    # append_api_trace(
    #     "workflow_boot",
    #     output_dir=output_dir,
    #     ok=True,
    #     data={
    #         "area_name": cfg.area_name,
    #         "area_id": cfg.area_id,
    #         "user_input": cfg.user_input,
    #         "img_w": cfg.img_w,
    #         "img_h": cfg.img_h,
    #         "max_loops": cfg.max_loops,
    #         "ws_url": ws_url or "",
    #     },
    # )

    logger.info("=" * 60)
    logger.info("开始执行桌面整理工作流")
    logger.info("=" * 60)
    logger.info(f"  目标区域: {cfg.area_name}")
    logger.info(f"  用户输入: {cfg.user_input}")
    logger.info(f"  图片尺寸: {cfg.img_w}x{cfg.img_h}")
    logger.info(f"  输出目录: {output_dir}")
    logger.info(f"  最大循环次数: {cfg.max_loops}")
    logger.info("=" * 60)

    robot = BajieRobot(ws_url=ws_url)

    def _connect() -> None:
        if not robot.Connect(timeout_sec=cfg.connect_timeout_sec):
            detail = robot.LastWebSocketError() or "unknown"
            raise RobotMissionError(f"{error_tag(ERR_CONNECT_FAILED)} connect failed: {detail}")

    retry_run(_connect, name="websocket_connect")
    # append_api_trace("robot_connect", output_dir=output_dir, ok=True, data={"detail": "connected"})

    # 保存最终工作流汇总
    workflow_summary = {
        "status": "success",
        "area_name": cfg.area_name,
        "user_input": cfg.user_input,
        "loops": [],
        "final_judge": None,
    }

    try:
        # ========== 步骤1: 重定位 ==========
        logger.info("=" * 50)
        logger.info("步骤1: 重定位")
        logger.info("=" * 50)
        speak(robot, SPEECH_RELOCATING)
        def _relocate():
            st = robot.eco_relocate(timeout_sec=120.0)
            _check_status(st, f"{error_tag(ERR_RELOCATE_FAILED)} eco_relocate")
            return st

        status = retry_run(_relocate, name="eco_relocate")
        # append_api_trace(
        #     "eco_relocate",
        #     output_dir=output_dir,
        #     ok=(relocate_code == 0),
        #     data={
        #         "code": relocate_code,
        #         "msg": getattr(status.error_info, "message", ""),
        #         "task_id": getattr(status, "task_id", ""),
        #         "request_uuid": getattr(status, "request_uuid", ""),
        #     },
        # )
        logger.info("重定位完成")

        # ========== 步骤2: 准备姿态 ==========
        logger.info("=" * 50)
        logger.info("步骤2: 准备姿态")
        logger.info("=" * 50)
        speak(robot, SPEECH_PREPARE_POSE)
        def _prepare():
            st = robot.eco_prepareRobotPose(timeout_sec=120.0)
            fail = mission_failed(st)
            if fail:
                raise RobotMissionError(f"{error_tag(ERR_PREPARE_POSE_FAILED)} eco_prepareRobotPose: {fail}")
            return st

        status = retry_run(_prepare, name="eco_prepareRobotPose")
        prep_code = getattr(status.error_info, "code", 0)
        # append_api_trace(
        #     "eco_prepareRobotPose",
        #     output_dir=output_dir,
        #     ok=(prep_code == 0),
        #     data={
        #         "code": prep_code,
        #         "msg": getattr(status.error_info, "message", ""),
        #         "task_id": getattr(status, "task_id", ""),
        #         "request_uuid": getattr(status, "request_uuid", ""),
        #     },
        # )
        logger.info("准备姿态完成")

        # ========== 感知1（初始） ==========
        loop_dir = os.path.join(output_dir, "loop_0")
        ensure_dir(loop_dir)
        logger.info("=" * 50)
        logger.info("步骤3: 感知1 (初始)")
        logger.info(f"  输出子目录: {loop_dir}")
        logger.info("=" * 50)
        cached_head_angle_rad = None
        success, image_before, head_view = asyncio.get_event_loop().run_until_complete(
            run_perception_workflow(
                robot,
                area_name=cfg.area_name,
                area_id=cfg.area_id,
                camera_type=2,
                img_w=cfg.img_w,
                img_h=cfg.img_h,
                output_dir=loop_dir,
                enable_vlm_head_angle_suggest=cfg.enable_vlm_head_angle_suggest,
                head_sweep_angles_rad=cfg.head_sweep_angles_rad,
                head_sweep_sleep_sec=cfg.head_sweep_sleep_sec,
                vlm_suggest_angle_timeout_sec=cfg.vlm_suggest_angle_timeout_sec,
                reuse_head_angle_rad=cached_head_angle_rad,
            )
        )
        # append_api_trace(
        #     "perception_before",
        #     output_dir=output_dir,
        #     ok=bool(success and image_before and head_view),
        #     data={
        #         "loop": 0,
        #         "output_dir": loop_dir,
        #         "perception_count": len((image_before or {}).get("perception", [])),
        #     },
        # )
        if not success or not image_before or not head_view:
            raise RobotMissionError(f"{error_tag(ERR_PERCEPTION_FAILED)} 感知1失败")

        perception = image_before.get("perception", [])
        logger.info(f"感知1完成，识别到 {len(perception)} 个物体")

        # 记录原感知1图像，用于后续评判对比
        first_image_before = image_before

        current_loop = 0
        is_tidy = False
        last_plan_result: Optional[Dict] = None
        judge_input = None
        cached_head_angle_rad = image_before.get("head_angle_applied_rad")

        while current_loop < cfg.max_loops:
            current_loop += 1
            loop_dir = os.path.join(output_dir, f"loop_{current_loop - 1}")
            ensure_dir(loop_dir)

            logger.info("=" * 60)
            logger.info(f"第 {current_loop}/{cfg.max_loops} 轮处理")
            logger.info(f"  输出子目录: {loop_dir}")
            logger.info("=" * 60)

            # ========== 规划 ==========
            logger.info("=" * 50)
            logger.info(f"步骤4: 意图规划 (第 {current_loop} 轮)")
            logger.info("=" * 50)
            speak(robot, SPEECH_PLANNING)
            success, plan_result = desk_intent_plan(
                robot,
                image_data=image_before,
                perception=perception,
                user_input=cfg.user_input,
                output_dir=loop_dir,
                judge_input=judge_input,
                memory_input=cfg.memory_input,
                grab_items_path=cfg.grab_items_path,
            )
            # append_api_trace(
            #     "desk_intent_plan",
            #     output_dir=output_dir,
            #     ok=bool(success and plan_result),
            #     data={
            #         "loop": current_loop,
            #         "output_dir": loop_dir,
            #         "intent_steps": len((plan_result or {}).get("intent_steps", [])),
            #     },
            # )
            if not success or not plan_result:
                raise RobotMissionError(f"{error_tag(ERR_PLAN_FAILED)} 意图规划失败")

            last_plan_result = plan_result
            intent_steps = plan_result.get("intent_steps", [])
            logger.info(f"获取到 {len(intent_steps)} 个处理步骤")
            for step in intent_steps:
                logger.info(
                    f"  步骤 {step.get('step_id')}: "
                    f"from_bbox={step.get('from_bbox')}, "
                    f"to_bbox={step.get('to_bbox')}, "
                    f"is_container={step.get('is_container')}"
                )

            if not intent_steps:
                raise RobotMissionError(f"{error_tag(ERR_PLAN_FAILED)} 没有需要处理的物品")

            # ========== 执行 ==========
            logger.info("=" * 50)
            logger.info(f"步骤5: 执行工作流 (第 {current_loop} 轮)")
            logger.info("=" * 50)
            speak(robot, SPEECH_EXECUTION)
            success = run_execution_workflow(
                robot,
                head_view=head_view,
                intent_steps=intent_steps,
                output_dir=loop_dir,
                stop_before_grab=cfg.stop_before_grab,
            )
            # append_api_trace(
            #     "execution_workflow",
            #     output_dir=output_dir,
            #     ok=bool(success),
            #     data={
            #         "loop": current_loop,
            #         "output_dir": loop_dir,
            #         "intent_steps": len(intent_steps),
            #         "stop_before_grab": bool(cfg.stop_before_grab),
            #     },
            # )
            if not success:
                raise RobotMissionError(f"{error_tag(ERR_EXECUTION_FAILED)} 执行工作流失败")

            if cfg.stop_before_grab:
                logger.info("已按配置 stop_before_grab=true，跳过后续步骤")
                return

            _arm_home(robot)

            # ========== 感知2（整理后） ==========
            next_loop_dir = os.path.join(output_dir, f"loop_{current_loop}")
            ensure_dir(next_loop_dir)
            logger.info("=" * 50)
            logger.info(f"步骤6: 感知2 (第 {current_loop} 轮后)")
            logger.info(f"  输出子目录: {next_loop_dir}")
            logger.info("=" * 50)
            success, image_after, head_view_after = asyncio.get_event_loop().run_until_complete(
                run_perception_workflow(
                    robot,
                    area_name=cfg.area_name,
                    area_id=cfg.area_id,
                    camera_type=2,
                    img_w=cfg.img_w,
                    img_h=cfg.img_h,
                    output_dir=next_loop_dir,
                    enable_vlm_head_angle_suggest=cfg.enable_vlm_head_angle_suggest,
                    head_sweep_angles_rad=cfg.head_sweep_angles_rad,
                    head_sweep_sleep_sec=cfg.head_sweep_sleep_sec,
                    vlm_suggest_angle_timeout_sec=cfg.vlm_suggest_angle_timeout_sec,
                    reuse_head_angle_rad=cached_head_angle_rad,
                )
            )
            # append_api_trace(
            #     "perception_after",
            #     output_dir=output_dir,
            #     ok=bool(success and image_after),
            #     data={
            #         "loop": current_loop,
            #         "output_dir": next_loop_dir,
            #         "perception_count": len((image_after or {}).get("perception", [])),
            #     },
            # )
            if not success or not image_after:
                raise RobotMissionError(f"{error_tag(ERR_PERCEPTION_FAILED)} 感知2失败")

            # ========== 评判 ==========
            logger.info("=" * 50)
            logger.info(f"步骤7: 评判 (第 {current_loop} 轮)")
            logger.info("=" * 50)
            speak(robot, SPEECH_JUDGING)
            success, judge_result = run_judge_workflow(
                robot,
                before_image_data=first_image_before,
                after_image_data=image_after,
                plan_result=plan_result,
                user_input=cfg.user_input,
                output_dir=loop_dir,
                timeout_sec=150.0,
            )
            # append_api_trace(
            #     "judge_workflow",
            #     output_dir=output_dir,
            #     ok=bool(success and judge_result),
            #     data={
            #         "loop": current_loop,
            #         "output_dir": loop_dir,
            #         "is_tidy": bool((judge_result or {}).get("is_tidy", False)),
            #         "score": (judge_result or {}).get("score"),
            #         "reason": (judge_result or {}).get("reason", ""),
            #     },
            # )
            if not success or not judge_result:
                raise RobotMissionError(f"{error_tag(ERR_JUDGE_FAILED)} 评判失败")

            is_tidy = judge_result.get("is_tidy", False)
            loop_summary = {
                "loop": current_loop,
                "plan_steps": len(intent_steps),
                "judge": judge_result,
                "output_dir": loop_dir,
            }
            workflow_summary["loops"].append(loop_summary)
            workflow_summary["final_judge"] = judge_result

            if is_tidy:
                logger.info("=" * 50)
                logger.info(f"评判通过 (第 {current_loop} 轮)")
                logger.info(f"  score: {judge_result.get('score')}")
                logger.info(f"  reason: {judge_result.get('reason')}")
                logger.info("=" * 50)
                break
            else:
                judge_input = judge_result.get('reason')
                logger.info("=" * 50)
                logger.info(f"评判未通过 (第 {current_loop} 轮)")
                logger.info(f"  score: {judge_result.get('score')}")
                logger.info(f"  reason: {judge_result.get('reason')}")
                logger.info("=" * 50)
                if current_loop >= cfg.max_loops:
                    logger.warning("已达到最大循环次数，退出循环")
                    break
                # 使用感知2的结果作为下一轮规划的输入（头图与 perception 同步更新）
                image_before = image_after
                perception = image_after.get("perception", [])
                if head_view_after is not None:
                    head_view = head_view_after
                    logger.info(
                        "下一轮将使用感知2的 %s 个物体结果重新规划，并同步更新 head_view",
                        len(perception),
                    )
                else:
                    logger.warning(
                        "感知2未返回 head RGBDViewWithPose，下一轮规划/执行 head_view 可能与 perception 不一致"
                    )
                    logger.info(f"下一轮将使用感知2的 {len(perception)} 个物体结果重新规划")

        # ========== 结束姿态 ==========
        logger.info("=" * 50)
        logger.info("步骤8: 结束姿态")
        logger.info("=" * 50)
        def _finish():
            st = robot.eco_finishRobotPose(timeout_sec=120.0)
            fail = mission_failed(st)
            if fail:
                raise RuntimeError(fail)
            return st

        try:
            status = retry_run(_finish, name="eco_finishRobotPose")
            end_code = getattr(status.error_info, "code", 0)
        except Exception as e:
            logger.warning("%s 结束姿态失败（已重试）: %s", error_tag(ERR_FINISH_POSE_FAILED), e)
            end_code = -1
        # append_api_trace(
        #     "eco_finishRobotPose",
        #     output_dir=output_dir,
        #     ok=(end_code == 0),
        #     data={"code": end_code},
        # )
        logger.info("结束姿态完成")

        speak(robot, SPEECH_DONE)

        # 保存汇总
        summary_file = os.path.join(output_dir, "workflow_summary.json")
        write_json(summary_file, workflow_summary)

        logger.info("=" * 60)
        logger.info("桌面整理工作流执行完毕")
        logger.info(f"  总轮数: {current_loop}")
        logger.info(f"  评判结果: {'通过' if is_tidy else '未通过 (已达最大重试)'}")
        logger.info(f"  汇总文件: {summary_file}")
        logger.info("=" * 60)
        # append_api_trace(
        #     "workflow_complete",
        #     output_dir=output_dir,
        #     ok=True,
        #     data={"loops": current_loop, "is_tidy": bool(is_tidy), "summary_file": summary_file},
        # )

    finally:
        # append_api_trace("robot_disconnect", output_dir=output_dir, ok=True, data={})
        robot.Disconnect()
