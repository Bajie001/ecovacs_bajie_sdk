"""Shared helpers for the Bajie SDK beginner notebooks.

The notebooks intentionally keep robot operations explicit, while this module
collects display, status-printing, and connection boilerplate.
"""

from __future__ import annotations

from dataclasses import asdict, is_dataclass
from pprint import pprint
from typing import Any, Iterable, Optional


DEFAULT_WS_URL = "ws://10.88.41.120:9900/"


def connect_robot(ws_url: str = DEFAULT_WS_URL, timeout_sec: float = 8.0) -> Any:
    """Create and connect a BajieRobot instance."""
    from bajie_sdk import BajieRobot

    robot = BajieRobot(ws_url=ws_url)
    if not robot.Connect(timeout_sec=timeout_sec):
        raise RuntimeError(
            "Connect() failed. Check the Linux board IP, port 9900, network, and robot service."
        )
    return robot


def disconnect_safely(robot: Any) -> None:
    """Disconnect a robot object without masking notebook progress."""
    if robot is None:
        return
    try:
        robot.Disconnect()
    except Exception as exc:  # pragma: no cover - notebook convenience
        print(f"Disconnect skipped: {exc}")


def compact(obj: Any) -> Any:
    """Convert dataclass-like objects into printable dictionaries when possible."""
    if is_dataclass(obj):
        return asdict(obj)
    if hasattr(obj, "__dict__"):
        return {
            key: val
            for key, val in vars(obj).items()
            if not key.startswith("_") and key not in {"value"}
        }
    return obj


def print_mission_status(status: Any) -> None:
    """Print the fields beginners need first from a MissionStatus-like object."""
    if status is None:
        print("Mission returned None")
        return

    task_id = getattr(status, "task_id", "")
    error_info = getattr(status, "error_info", None)
    code = getattr(error_info, "code", None)
    message = (
        getattr(error_info, "message", None)
        or getattr(error_info, "msg", None)
        or getattr(error_info, "info", None)
    )
    print(f"task_id: {task_id}")
    print(f"error_info.code: {code}")
    print(f"error_info.message: {message}")
    if error_info is None:
        print("raw status:")
        pprint(compact(status))
    elif code in (0, "0", None, ""):
        print("interpretation: mission accepted/finished without an SDK-visible error code.")
    else:
        print("interpretation: mission reported an error. Check robot state, parameters, and service logs.")


def print_robot_status(robot: Any) -> None:
    """Print work state, battery, alarm, position, map, and furniture summary."""
    try:
        robot.eco_refreshRobotInfo(timeout_sec=5.0)
    except Exception as exc:
        print(f"refresh robot info failed: {exc}")

    work_state = robot.eco_robotWorkState()
    battery = robot.eco_robotBattery()
    alarm = robot.eco_robotAlarm()
    pos = robot.eco_robotPosition()
    mapinfo = robot.eco_robotMapInfo()
    furniture = robot.eco_robotFurniture()

    print("work_state:")
    pprint(compact(work_state))
    print("\nbattery:")
    pprint(compact(battery))
    print("\nalarm:")
    pprint(alarm)
    print("\nposition:")
    pprint(compact(pos))
    print("\nmap:")
    if mapinfo:
        print(
            f"{getattr(mapinfo, 'mname', '')} "
            f"({getattr(mapinfo, 'totalWidth', '?')}x{getattr(mapinfo, 'totalHeight', '?')})"
        )
    else:
        print(None)
    print("\nfurniture:")
    items = getattr(furniture, "info", None) if furniture else None
    if not items:
        print(None)
    else:
        for item in items[:20]:
            print(f"- {getattr(item, 'fname', '')} ({getattr(item, 'fid', '')})")
        if len(items) > 20:
            print(f"... and {len(items) - 20} more")


def summarize_view(view: Any) -> None:
    """Print key fields from an RGBDViewWithPose-like result."""
    if view is None:
        print("view: None")
        return
    rgb = getattr(getattr(view, "rgb_image", None), "img", None)
    depth = getattr(getattr(view, "depth_image", None), "img", None)
    print("rgb shape:", getattr(rgb, "shape", None), "dtype:", getattr(rgb, "dtype", None))
    print("depth shape:", getattr(depth, "shape", None), "dtype:", getattr(depth, "dtype", None))
    print("tf_goal present:", getattr(view, "tf_goal", None) is not None)


def summarize_detections(items: Iterable[Any], limit: int = 10) -> None:
    """Print a compact detection table."""
    items = list(items or [])
    print("detection count:", len(items))
    for idx, item in enumerate(items[:limit]):
        name = getattr(item, "name", None) or getattr(item, "label", None) or ""
        score = getattr(item, "score", None)
        bbox = getattr(item, "bbox", None)
        print(f"{idx}: name={name!r} score={score} bbox={bbox}")
    if len(items) > limit:
        print(f"... and {len(items) - limit} more")


def print_safety_gate(*, connected: bool, enabled: bool, action: str) -> None:
    """Explain why a robot-action cell will or will not run."""
    print(f"action: {action}")
    print(f"connected: {connected}")
    print(f"enabled: {enabled}")
    if not connected:
        print("next step: set CONNECT_ROBOT = True after confirming the robot IP and safety.")
    elif not enabled:
        print(f"next step: set the related ENABLE flag to True only after reading the safety notes.")
    else:
        print("ready: the cell is allowed to call the robot.")


def _rgb_array(view_or_image: Any) -> Any:
    if hasattr(view_or_image, "rgb_image"):
        return view_or_image.rgb_image.img
    return view_or_image


def show_rgb(view_or_image: Any, title: str = "RGB image") -> None:
    """Display an RGB image or RGBDViewWithPose in a notebook."""
    import matplotlib.pyplot as plt

    image = _rgb_array(view_or_image)
    plt.figure(figsize=(8, 5))
    plt.imshow(image)
    plt.title(title)
    plt.axis("off")
    plt.show()


def draw_bboxes(image: Any, items: Iterable[Any], title: str = "Detections") -> None:
    """Draw detection boxes. Accepts ObjectDetection/NamedBBox-like objects."""
    import matplotlib.pyplot as plt
    from matplotlib.patches import Rectangle

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.imshow(_rgb_array(image))
    ax.set_title(title)
    ax.axis("off")

    for idx, item in enumerate(items):
        bbox = getattr(item, "bbox", None)
        if bbox is None and hasattr(item, "to_named_bbox_for_grab"):
            bbox = item.to_named_bbox_for_grab().bbox
        if bbox is None:
            continue
        x1, y1, x2, y2 = [float(v) for v in bbox[:4]]
        name = getattr(item, "name", None) or getattr(item, "label", None) or f"item_{idx}"
        ax.add_patch(Rectangle((x1, y1), x2 - x1, y2 - y1, fill=False, linewidth=2))
        ax.text(x1, y1, str(name), color="white", bbox={"facecolor": "black", "alpha": 0.6})
    plt.show()


def require_enabled(flag: bool, name: str) -> None:
    """Stop a dangerous cell unless the user explicitly enabled it."""
    if not flag:
        raise RuntimeError(f"{name} is disabled. Read the cell, check the robot, then set the flag to True.")


def first_or_none(items: Optional[Iterable[Any]]) -> Optional[Any]:
    if not items:
        return None
    for item in items:
        return item
    return None
