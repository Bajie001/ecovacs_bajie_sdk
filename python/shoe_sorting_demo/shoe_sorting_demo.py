from __future__ import annotations

import argparse, math, sys, traceback, uuid
from datetime import datetime
from pathlib import Path
from typing import Sequence

import cv2

from bajie_sdk import (
    BajieRobot, CameraType, DetectObjectsRequest, ObjectDetection, OvdEndpoint,
    Position, SemanticMapAppIsCustom, SemanticMapManagerCmd,
    SemanticMapManagerObjectInfo, SemanticMapModelLevel, SemanticNavigationRequest,
)

try:
    from .execution import execute_pick_and_place, mission_retry
    from .planning import get_shoe_move_steps, head_image_ref_bboxes_for_move_steps
except ImportError:
    if (_d := str(Path(__file__).resolve().parent)) not in sys.path: sys.path.insert(0, _d)
    from execution import execute_pick_and_place, mission_retry
    from planning import get_shoe_move_steps, head_image_ref_bboxes_for_move_steps

_DEFAULT_CARRIER_BOX = [[1.2, 0.6, 0.0], [1.2, -0.6, 0.0], [0.6, -0.6, 0.0], [0.6, 0.6, 0.0]]
_DEFAULT_FRONT_EDGE = [[0.6, -0.6, 0.0], [0.6, 0.6, 0.0]]


def _say(robot: BajieRobot, text: str, *, no_voice: bool = False) -> None:
    print(f"[voice] {text}", flush=True)
    if no_voice:
        return
    try:
        robot.eco_robotSpeech(text)
    except Exception:
        pass


def _relative_to_map(coords: Sequence[Sequence[float]], pos: Position) -> list[list[float]]:
    cos_yaw, sin_yaw = math.cos(pos.yaw), math.sin(pos.yaw)
    return [[pos.x + cos_yaw * float(p[0]) - sin_yaw * float(p[1]),
             pos.y + sin_yaw * float(p[0]) + cos_yaw * float(p[1]),
             float(p[2]) if len(p) > 2 else 0.0] for p in coords]


def _resolve_organize_area(
    robot: BajieRobot, area_name: str | None,
    carrier_box: Sequence[Sequence[float]] | None,
    front_edge: Sequence[Sequence[float]] | None,
    *, no_voice: bool = False,
) -> tuple[str | None, list[list[float]], list[list[float]]]:
    if area_name:
        resp = robot.eco_manageSemanticMap(SemanticMapManagerCmd.QUERY,
            SemanticMapManagerObjectInfo(name=area_name))
        obj = next((o for o in resp.objects_info if o.name == area_name), None)
        if not obj or not obj.content or not obj.direction:
            _say(robot, f"语义区域{area_name},没找到", no_voice=no_voice)
            raise ValueError(f"语义区域 {area_name} 未找到")
        _say(robot, f"已找到语义区域{area_name}", no_voice=no_voice)
        return None, obj.content, obj.direction

    carrier = list(carrier_box or _DEFAULT_CARRIER_BOX)
    front = list(front_edge or _DEFAULT_FRONT_EDGE)

    _say(robot, "未指定整理区域，将使用机器正前方默认区域，请确保前方无障碍物", no_voice=no_voice)

    pos = robot.eco_robotPosition()
    if not pos: raise RuntimeError("无法获取机器人当前位置")
    map_carrier, map_front = _relative_to_map(carrier, pos), _relative_to_map(front, pos)
    fid = f"temp_{uuid.uuid4().hex[:8]}"
    robot.eco_manageSemanticMap(SemanticMapManagerCmd.ADD, SemanticMapManagerObjectInfo(
        id=fid, name=fid, model_name="custom_organize_zone",
        model_level=SemanticMapModelLevel.CUSTOM_ORGANIZE_ZONE,content=map_carrier, direction=map_front,
        app_is_custom=SemanticMapAppIsCustom.USER_MANUAL))
    return fid, map_carrier, map_front


def shoe_sort(
    *, area_name: str | None = None,
    carrier_box: Sequence[Sequence[float]] | None = None,
    front_edge: Sequence[Sequence[float]] | None = None,
    ws_url: str | None = None, vis_prefix: str | None = None,
    no_voice: bool = False) -> None:
    robot = None
    fid = None
    try:
        robot = BajieRobot(ws_url=ws_url, auto_connect=True, connect_timeout_sec=4.0)
        robot.eco_cancelAllMissions()
        robot.eco_prepareRobotPose()

        _say(robot, "开始整理鞋子", no_voice=no_voice)

        fid, map_carrier, map_front = _resolve_organize_area(robot, area_name, carrier_box, front_edge, no_voice=no_voice)
        nav = fid or area_name
        assert nav, "必须有语义区域名或已创建临时区域"

        for attempt in range(1, 4):
            _say(robot, f"第{attempt}/3次规划尝试", no_voice=no_voice)
            print(f"\n===== 规划尝试 {attempt}/3 =====", flush=True)
            mission_retry(attempts=3, op_name="navigate", robot=robot,
                          no_voice=no_voice, op_cn="导航",
                          fail_do=lambda: robot.eco_cancelAllMissions())(
                lambda: robot.eco_navigateToSemanticArea(
                    SemanticNavigationRequest(area_name=nav), timeout_sec=20.0))

            display_name = area_name or "当前区域"
            _say(robot, f"已到达{display_name}，开始检测鞋子", no_voice=no_voice)

            img = robot.eco_captureImages(CameraType.HEAD)
            items = list[ObjectDetection](robot.eco_detect_objects(
                DetectObjectsRequest(rgb_image=img.rgb_image.img, labels=["鞋子"], entry=OvdEndpoint.SHOE)).items)
            if not items: print("未检测到鞋子，任务结束", flush=True); break

            _say(robot, f"检测到{len(items)}只鞋子，开始配对", no_voice=no_voice)

            steps = get_shoe_move_steps(robot, img, items, carrier_box=map_carrier, front_edge=map_front,
                                        move_all=(attempt == 1), vis_prefix=f"{vis_prefix}_{attempt}_")
            if not steps: print("规划为空，任务结束", flush=True); break

            _say(robot, f"规划完成，共{len(steps)}次搬运", no_voice=no_voice)

            ordered = sorted(steps, key=lambda m: m.step_id)
            total = len(ordered)
            for move_step, head_ref in zip(ordered, head_image_ref_bboxes_for_move_steps(ordered), strict=True):
                _say(robot, f"正在整理第{move_step.step_id}/{total}只", no_voice=no_voice)
                try:
                    execute_pick_and_place(robot, "鞋子", img, move_step, map_front,
                                           head_image_ref_bbox=head_ref, no_voice=no_voice)
                except Exception as e:
                    print(f"pick_place 步骤 {move_step.step_id} 失败: {e}")
                    try:
                        robot.eco_cancelAllMissions()
                        robot.eco_prepareRobotPose()
                    except Exception as prepare_e:
                        print(f"取消任务/恢复姿势失败: {prepare_e}")
                    # 跳出当前搬运步骤的循环，让外层 for attempt 重新导航→检测→规划→执行
                    break

        _say(robot, "整理完成", no_voice=no_voice)

    except Exception as e:
        traceback.print_exc()
        if robot: _say(robot, f"整理失败: {str(e)[:50]}", no_voice=no_voice)
        print(f"失败: {e}")
        raise RuntimeError(str(e)) from e
    finally:
        if robot:
            robot.eco_cancelAllMissions()
            robot.eco_prepareRobotPose()
            if fid:
                try: robot.eco_manageSemanticMap(SemanticMapManagerCmd.DELETE,SemanticMapManagerObjectInfo(id=fid))
                except Exception: pass
            robot.Disconnect()


def main() -> int:
    parser = argparse.ArgumentParser(description="鞋子整理示例（重试版）")
    parser.add_argument("--ws-url", default=None, help="机器人 WebSocket 地址或 IP（留空由 SDK 自动从环境变量读取）")
    parser.add_argument("--area-name", default="", help="语义区域名（留空则整理机器人正前方区域）")
    parser.add_argument("--vis-out", default="", metavar="PREFIX",
        help="无扩展名路径前缀；会生成 PREFIX_pairs.png、PREFIX_plan.png、PREFIX_placement.png")
    parser.add_argument("--no-voice", action="store_true", help="禁用语音播报")
    args = parser.parse_args()
    shoe_sort(ws_url=args.ws_url, area_name=args.area_name or None, vis_prefix=args.vis_out or None,
              no_voice=args.no_voice)
    return 0


if __name__ == "__main__":
    try: main()
    except Exception as e:
        traceback.print_exc()
        print(f"shoe_sort failed: {e}")
