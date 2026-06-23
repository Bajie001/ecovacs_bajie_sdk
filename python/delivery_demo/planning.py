from __future__ import annotations

from bajie_sdk import BajieRobot, ObjectInfo, ObjectPose3D, SearchRequest, SearchResponse

from errors import RobotMissionError, ValidationError


def safe_mission_call(step: str, fn):
    try:
        return fn()
    except RuntimeError as exc:
        raise RobotMissionError(f"{step} failed: {exc}", msg=str(exc)) from exc


def assert_ok(step: str, resp: SearchResponse) -> None:
    if resp.error_info.code != 0:
        raise RobotMissionError.from_error_info(step, resp.error_info)


def is_no_object_error(msg: str) -> bool:
    return ("code=102404" in msg) or ("无法找到物体" in msg)


def select_pose(poses: list[ObjectPose3D], index: int) -> ObjectPose3D:
    if not poses:
        raise ValidationError("search returned empty pose list")
    idx = index
    if idx < 0:
        idx = len(poses) + idx
    idx = max(0, min(idx, len(poses) - 1))
    return poses[idx]


def search_toy(
    robot: BajieRobot,
    *,
    search_pose_index: int,
    timeout_sec: float = 120.0,
) -> ObjectPose3D:
    resp = safe_mission_call(
        "search[toy]",
        lambda: robot.eco_findObject(
            SearchRequest(object=ObjectInfo(obj_name="玩具")),
            timeout_sec=timeout_sec,
        ),
    )
    assert_ok("search[toy]", resp)
    return select_pose(resp.results, search_pose_index)
