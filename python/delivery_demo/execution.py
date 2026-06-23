from __future__ import annotations

import time

from bajie_sdk import (
    BajieRobot,
    CameraType,
    DetectObjectsRequest,
    MissionStatus,
    ObjectPose3D,
    PutWhereSummaryRequest,
    RobotArmCtrlMode,
    SemanticNavigationRequest,
)

from errors import RobotMissionError, ValidationError
from planning import safe_mission_call


def assert_status_ok(step: str, status: MissionStatus) -> None:
    if status.error_info.code != 0:
        raise RobotMissionError.from_error_info(step, status.error_info)


def is_object_drop_error(msg: str) -> bool:
    return ("code=36897" in msg) or ("物体掉落" in msg)


def _sleep() -> None:
    time.sleep(0.01)


def reset_arm_home(robot: BajieRobot) -> None:
    _sleep()
    status = safe_mission_call(
        "robot_arm_home",
        lambda: robot.eco_setRobotArmMode(RobotArmCtrlMode.ARM_HOME, timeout_sec=120.0),
    )
    assert_status_ok("robot_arm_home", status)


def hand_observe(robot: BajieRobot, pose: ObjectPose3D) -> None:
    _sleep()
    status = safe_mission_call(
        "hand_observe",
        lambda: robot.eco_lookto(pose, camera_type=CameraType.ARM, timeout_sec=180.0),
    )
    assert_status_ok("hand_observe", status)


def arm_capture_and_pick(robot: BajieRobot, *, pick_index: int) -> None:
    _sleep()
    arm_image = robot.eco_captureImages(CameraType.ARM, timeout_sec=120.0)
    assert_status_ok("image_query[arm]", arm_image)

    detect_resp = robot.eco_detect_objects(
        DetectObjectsRequest(rgb_image=arm_image.rgb_image.img, labels=["玩具"]),
        timeout=15.0,
    )
    if not detect_resp.items:
        raise ValidationError("OVD did not detect toy target")

    idx = max(0, min(pick_index, len(detect_resp.items) - 1))
    target = detect_resp.items[idx]
    print(f"识别目标: name={target.name}, bbox={target.bbox}")

    _sleep()
    try:
        status = safe_mission_call(
            "accurate_grab",
            lambda: robot.eco_pick(arm_image, target.to_named_bbox_for_grab(), timeout_sec=300.0),
        )
        assert_status_ok("accurate_grab", status)
        return
    except RobotMissionError as exc:
        if is_object_drop_error(str(exc)):
            print("精准抓取失败（36897/物体掉落），本次抓取尝试失败。")
        raise


def navigate_to_table(robot: BajieRobot, *, table_area_name: str) -> None:
    _sleep()
    status = safe_mission_call(
        "semantic_navigation",
        lambda: robot.eco_navigateToSemanticArea(
            SemanticNavigationRequest(area_name=table_area_name),
            timeout_sec=180.0,
        ),
    )
    assert_status_ok("semantic_navigation", status)


def head_place_flow(
    robot: BajieRobot,
) -> None:
    _sleep()
    head_image = robot.eco_captureImages(CameraType.HEAD, timeout_sec=120.0)
    assert_status_ok("image_query[head]", head_image)

    place_ref = robot.eco_put_where_summary(
        PutWhereSummaryRequest(
            carrier="",
            carrier_direct="",
            image=head_image.rgb_image.img,
            obj_image=head_image.rgb_image.img,
            placeholder="玩具",
            summary="将玩具放到桌面上",
        ),
        timeout=30.0,
    )

    _sleep()
    place_status = safe_mission_call(
        "semantic_place",
        lambda: robot.eco_place_with_view(head_image, place_ref, recapture_place=False, timeout_sec=300.0),
    )
    assert_status_ok("semantic_place", place_status)
