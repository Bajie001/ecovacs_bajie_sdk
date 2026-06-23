from __future__ import annotations

from typing import Optional, Sequence

from bajie_sdk import AreaInfo, BajieRobot, ObjectInfo, ObjectPose3D, SearchRequest, SearchResponse

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


def select_pose(poses: Sequence[ObjectPose3D], index: int) -> ObjectPose3D:
    if not poses:
        raise ValidationError("search returned empty pose list")
    idx = index
    if idx < 0:
        idx = len(poses) + idx
    idx = max(0, min(idx, len(poses) - 1))
    return poses[idx]


def to_filter_box(pose: ObjectPose3D) -> ObjectPose3D:
    return ObjectPose3D(
        position=pose.position,
        orientation=pose.orientation,
        box_length=pose.box_length,
        frame_id=pose.frame_id,
    )


def search_pose(
    robot: BajieRobot,
    *,
    item: str,
    area_name: str = "",
    filter_boxes: Optional[Sequence[ObjectPose3D]] = None,
    search_pose_index: int = -1,
    timeout_sec: float = 120.0,
) -> ObjectPose3D:
    resp = safe_mission_call(
        f"search[{item}]",
        lambda: robot.eco_findObject(
            SearchRequest(
                object=ObjectInfo(obj_name=item),
                area=AreaInfo(area_name=area_name),
                filter_boxes=list(filter_boxes or []),
            ),
            timeout_sec=timeout_sec,
        ),
    )
    assert_ok(f"search[{item}]", resp)
    return select_pose(resp.results, search_pose_index)
