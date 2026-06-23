from __future__ import annotations

import time
from typing import Optional

from bajie_sdk import BajieRobot, ObjectPose3D

from errors import RobotMissionError, ToyStorageError
from execution import arm_capture_and_pick, hand_observe, head_place_flow, reset_arm_home
from models import ToyStorageConfig
from planning import is_no_object_error, safe_mission_call, search_pose, to_filter_box

MAX_GRAB_RETRIES = 3
MAX_PLACE_RETRIES = 3


def _relocate(robot: BajieRobot) -> None:
    status = safe_mission_call(
        "relocate",
        lambda: robot.eco_relocate(timeout_sec=120.0),
    )
    if status.error_info.code != 0:
        raise RobotMissionError.from_error_info("relocate", status.error_info)


def _finish_pose(robot: BajieRobot) -> None:
    status = safe_mission_call(
        "finish_robot_pose",
        lambda: robot.eco_finishRobotPose(timeout_sec=120.0),
    )
    if status.error_info.code != 0:
        raise RobotMissionError.from_error_info("finish_robot_pose", status.error_info)


def _speak(robot: BajieRobot, text: str) -> None:
    print(f"语音播报: {text}")
    try:
        robot.eco_robotSpeech(text)
    except Exception:
        pass


def _failure_speech_text(exc: ToyStorageError) -> str:
    if isinstance(exc, RobotMissionError) and exc.code is not None:
        msg = exc.msg or "无"
        return f"任务失败，故障码 {exc.code}，失败原因 {msg}"
    return f"任务失败，失败原因 {exc}"


def _grab_with_retries(robot: BajieRobot, cfg: ToyStorageConfig, toy_pose: ObjectPose3D) -> None:
    last_error: Optional[ToyStorageError] = None
    for retry in range(1, MAX_GRAB_RETRIES + 1):
        try:
            print("观测前手臂归位")
            reset_arm_home(robot)
            print("步骤3: 手部观测玩具")
            hand_observe(robot, toy_pose)
            print("步骤4: 手部拍照")
            print("步骤5: OVD识别玩具")
            print("步骤6: 精准抓取")
            arm_capture_and_pick(robot, pick_index=cfg.pick_index)
            return
        except ToyStorageError as exc:
            last_error = exc
            print(f"第{retry}次抓取失败: {exc}")

    if isinstance(last_error, RobotMissionError):
        raise RobotMissionError(
            "连续三次抓取失败,任务结束",
            code=last_error.code,
            msg=last_error.msg,
        ) from last_error
    raise RobotMissionError("连续三次抓取失败,任务结束")


def _place_with_retries(robot: BajieRobot, cfg: ToyStorageConfig, basket_pose: ObjectPose3D) -> None:
    last_error: Optional[ToyStorageError] = None
    for retry in range(1, MAX_PLACE_RETRIES + 1):
        try:
            print("步骤7: 头部观测收纳筐")
            print("步骤8: 头部拍照")
            print("步骤9: 调用 put_where")
            print("步骤10: 语义放置")
            head_place_flow(
                robot,
                basket_pose=basket_pose,
                carrier=cfg.carrier,
                carrier_direct=cfg.carrier_direct,
            )
            return
        except ToyStorageError as exc:
            last_error = exc
            print(f"第{retry}次放置失败: {exc}")

    if isinstance(last_error, RobotMissionError):
        raise RobotMissionError(
            "连续三次放置失败,任务结束",
            code=last_error.code,
            msg=last_error.msg,
        ) from last_error
    raise RobotMissionError("连续三次放置失败,任务结束")


def run_one_round(robot: BajieRobot, cfg: ToyStorageConfig) -> bool:
    print("步骤1: 搜索收纳筐")
    basket_pose = search_pose(
        robot,
        item="收纳筐",
        area_name=cfg.area_name,
        search_pose_index=cfg.search_pose_index,
    )

    print("步骤2: 搜索玩具（过滤筐内）")
    try:
        toy_pose = search_pose(
            robot,
            item="玩具",
            area_name=cfg.area_name,
            filter_boxes=[to_filter_box(basket_pose)],
            search_pose_index=cfg.search_pose_index,
        )
    except ToyStorageError as exc:
        if is_no_object_error(str(exc)):
            print("未搜索到玩具：执行结束姿态 eco_finishRobotPose")
            _finish_pose(robot)
            return False
        raise

    _grab_with_retries(robot, cfg, toy_pose)
    _place_with_retries(robot, cfg, basket_pose)
    return True


def toy_storage(*, ws_url: Optional[str], cfg: ToyStorageConfig, max_rounds: int) -> None:
    robot = BajieRobot(ws_url=ws_url or None)
    if not robot.Connect(timeout_sec=cfg.connect_timeout_sec):
        detail = robot.LastWebSocketError() or "unknown"
        raise RobotMissionError(f"connect failed: {detail}")

    print("=== 玩具收纳 Python 示例 ===")
    print("按 Ctrl+C 停止")

    try:
        try:
            print("步骤0: 重定位")
            _relocate(robot)
            i = 0
            while True:
                if max_rounds > 0 and i >= max_rounds:
                    break
                i += 1
                print(f"\n--- round {i} ---")
                should_continue = run_one_round(robot, cfg)
                if not should_continue:
                    print("所有玩具已完成收纳，流程结束。")
                    break
                print(f"round {i} success")
                time.sleep(cfg.loop_sleep_sec)
            _speak(robot, "任务完成")
        except ToyStorageError as exc:
            _speak(robot, _failure_speech_text(exc))
            raise
    finally:
        robot.Disconnect()
