from __future__ import annotations

from typing import Optional

from bajie_sdk import BajieRobot, ObjectPose3D

from errors import DeliveryDemoError, RobotMissionError
from execution import (
    arm_capture_and_pick,
    hand_observe,
    head_place_flow,
    navigate_to_table,
    reset_arm_home,
)
from models import DeliveryConfig
from planning import is_no_object_error, safe_mission_call, search_toy

MAX_GRAB_RETRIES = 3
MAX_PLACE_RETRIES = 3
TABLE_PLACE_HEIGHT_M = 0.44


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


def _raise_to_table_height(robot: BajieRobot) -> None:
    status = safe_mission_call(
        "set_robot_height",
        lambda: robot.eco_setRobotHeight(TABLE_PLACE_HEIGHT_M, timeout_sec=120.0),
    )
    if status.error_info.code != 0:
        raise RobotMissionError.from_error_info("set_robot_height", status.error_info)


def _speak(robot: BajieRobot, text: str) -> None:
    print(f"语音播报: {text}")
    try:
        robot.eco_robotSpeech(text)
    except Exception:
        pass


def _failure_speech_text(exc: DeliveryDemoError) -> str:
    if isinstance(exc, RobotMissionError) and exc.code is not None:
        msg = exc.msg or "无"
        return f"任务失败，故障码 {exc.code}，失败原因 {msg}"
    return f"任务失败，失败原因 {exc}"


def _grab_with_retries(robot: BajieRobot, cfg: DeliveryConfig, toy_pose: ObjectPose3D) -> None:
    last_error: Optional[DeliveryDemoError] = None
    for retry in range(1, MAX_GRAB_RETRIES + 1):
        try:
            print("观测前手臂归位")
            reset_arm_home(robot)
            print("步骤2: 手部观测玩具")
            hand_observe(robot, toy_pose)
            print("步骤3: 手部拍照")
            print("步骤4: OVD 识别玩具")
            print("步骤5: 抓取玩具")
            arm_capture_and_pick(robot, pick_index=cfg.pick_index)
            return
        except DeliveryDemoError as exc:
            last_error = exc
            print(f"第{retry}次抓取失败: {exc}")

    if isinstance(last_error, RobotMissionError):
        raise RobotMissionError(
            "连续三次抓取失败,任务结束",
            code=last_error.code,
            msg=last_error.msg,
        ) from last_error
    raise RobotMissionError("连续三次抓取失败,任务结束")


def _place_with_retries(robot: BajieRobot, cfg: DeliveryConfig) -> None:
    last_error: Optional[DeliveryDemoError] = None
    for retry in range(1, MAX_PLACE_RETRIES + 1):
        try:
            print("步骤8: 头部拍照")
            print("步骤9: PutWhere 推荐放置区域")
            print("步骤10: 语义放置")
            head_place_flow(robot)
            return
        except DeliveryDemoError as exc:
            last_error = exc
            print(f"第{retry}次放置失败: {exc}")
            if retry < MAX_PLACE_RETRIES:
                print(f"第{retry}次放置失败，重新导航至桌子")
                navigate_to_table(robot, table_area_name=cfg.table_area_name)

    if isinstance(last_error, RobotMissionError):
        raise RobotMissionError(
            "连续三次放置失败,任务结束",
            code=last_error.code,
            msg=last_error.msg,
        ) from last_error
    raise RobotMissionError("连续三次放置失败,任务结束")


def run_once(robot: BajieRobot, cfg: DeliveryConfig) -> bool:
    print("步骤1: 原地寻找玩具")
    try:
        toy_pose = search_toy(
            robot,
            search_pose_index=cfg.search_pose_index,
        )
    except DeliveryDemoError as exc:
        if is_no_object_error(str(exc)):
            print("未搜索到玩具：执行结束姿态 eco_finishRobotPose")
            _finish_pose(robot)
            return False
        raise

    _grab_with_retries(robot, cfg, toy_pose)
    print(f"步骤6: 语义导航到桌子（{cfg.table_area_name}）")
    navigate_to_table(robot, table_area_name=cfg.table_area_name)
    print(f"步骤7: 升高机身至 {TABLE_PLACE_HEIGHT_M:.2f}m")
    _raise_to_table_height(robot)
    _place_with_retries(robot, cfg)
    return True


def delivery_demo(*, ws_url: Optional[str], cfg: DeliveryConfig) -> None:
    robot = BajieRobot(ws_url=ws_url or None)
    if not robot.Connect(timeout_sec=cfg.connect_timeout_sec):
        detail = robot.LastWebSocketError() or "unknown"
        raise RobotMissionError(f"connect failed: {detail}")

    print("=== 玩具配送 Python 示例（地面 -> 桌面） ===")
    try:
        try:
            print("步骤0: 重定位")
            _relocate(robot)
            should_continue = run_once(robot, cfg)
            if should_continue:
                print("配送演示完成")
                _speak(robot, "任务完成")
            else:
                print("无可配送玩具，已执行结束姿态。")
                _speak(robot, "任务完成")
        except DeliveryDemoError as exc:
            _speak(robot, _failure_speech_text(exc))
            raise
    finally:
        robot.Disconnect()
