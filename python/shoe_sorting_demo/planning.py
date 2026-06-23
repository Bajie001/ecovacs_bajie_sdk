from __future__ import annotations

import math,sys
from typing import Dict, List, Sequence, Tuple
from pathlib import Path

from bajie_sdk import (
    BajieRobot,
    MovePlanStep,
    ObjectDetection,
    PairedBBox,
    Quatf,
    RGBDViewWithPose,
)
try:
    from .pairing import pair_shoes
    from .visualize_plans import (draw_pairs_on_rgb,draw_placement_on_rgb,draw_plans_on_rgb,write_rgb_png)
except ImportError:
    if (_d := str(Path(__file__).resolve().parent)) not in sys.path: sys.path.insert(0, _d)
    from pairing import pair_shoes
    from visualize_plans import (draw_pairs_on_rgb,draw_placement_on_rgb,draw_plans_on_rgb,write_rgb_png)


def _bbox_key(bbox: Sequence[int]) -> tuple[int, int, int, int]:
    return (int(bbox[0]), int(bbox[1]), int(bbox[2]), int(bbox[3]))


def head_image_ref_bboxes_for_move_steps(
    move_steps: Sequence[MovePlanStep],
) -> List[Tuple[int, int, int, int]]:
    """为每一步规划给出头图上的**参考框**（与当步 ``from_bbox`` 同尺、但取「链首」位置）.

    规划会按模拟摆放改写每步 ``from_bbox``，而执行时头图不刷新；``eco_match_obj_views`` 需在**首帧头图**
    上裁切。只根据 ``move_steps`` 判断**是否连续移动**（本步 ``from_bbox`` 与上一步 ``to_bbox`` 四元组相同）：

    * 新链的第一步：该步 ``from_bbox`` 即该物体在头图中的**原本**位置；
    * 连续步：建 ``{ to_bbox: from_bbox }``（每步一条）；对当前步的 ``from``，若在字典中作为 ``to`` 出现则
      ``cur = d[cur]`` 循环直到找不到，``cur`` 即链头 ``from``。
    """
    if not move_steps:
        return []
    steps = sorted(move_steps, key=lambda s: s.step_id)
    to_to_from: Dict[Tuple[int, int, int, int], Tuple[int, int, int, int]] = {
        _bbox_key(m.to_bbox): _bbox_key(m.from_bbox) for m in steps
    }
    n = len(steps)
    out: List[Tuple[int, int, int, int]] = []
    for m in steps:
        cur = _bbox_key(m.from_bbox)
        for _ in range(n + 1):
            if cur not in to_to_from:
                break
            cur = to_to_from[cur]
        else:
            raise ValueError("to→from 出现环，无法归约到链头")
        out.append(cur)
    return out


def _quat_x_axis_yaw(q: Quatf) -> float:
    """从四元数提取局部 x 轴在地平面投影的 yaw（弧度）。"""
    x, y, z, w = float(q.x), float(q.y), float(q.z), float(q.w)
    rxx = 1.0 - 2.0 * y * y - 2.0 * z * z
    ryx = 2.0 * x * y + 2.0 * w * z
    return math.atan2(ryx, rxx)


def get_shoe_move_steps(
    robot: BajieRobot, head_image: RGBDViewWithPose,
    shoe_items: Sequence[ObjectDetection], *,
    carrier_box: Sequence[Sequence[float]],
    front_edge: Sequence[Sequence[float]],
    move_all: bool = True, vis_prefix: str | None = None,
    tilt_thresh_deg: float = 30.0,
) -> List[MovePlanStep]:
    """配对/线面投影/步进规划。move_all=False 时仅整理摆歪(is_tilted)的鞋子。"""
    pair_map = pair_shoes(robot, head_image, shoe_items)

    from bajie_sdk.vision_http import project_place_line_and_plane
    proj = project_place_line_and_plane(head_image, carrier_box, front_edge)
    if not proj: raise ValueError("投影失败：放置区不在视野范围内")
    row = proj[0]

    paired_bbox = [PairedBBox(
        bbox=(int(it.bbox[0]), int(it.bbox[1]), int(it.bbox[2]), int(it.bbox[3])),
        pair_id=pair_map.get(str(it.uuid), 0),
    ) for it in shoe_items]

    if not move_all:
        thresh = math.radians(tilt_thresh_deg)
        for pb in paired_bbox:
            pb.is_tilted = True
            try:
                pose = robot.eco_computeObjectPose([pb.bbox], head_image, timeout_sec=6.0)[0]
                diff = abs(_quat_x_axis_yaw(pose.orientation) - row.place_yaw) % math.pi
                pb.is_tilted = min(diff, math.pi - diff) >= thresh
            except Exception:
                pass

    rgb, plc_rgb = head_image.rgb_image.img, None
    base = str(vis_prefix).rstrip() if vis_prefix and str(vis_prefix).strip() else None
    if base:
        write_rgb_png(f"{base}_pairs.png",  draw_pairs_on_rgb(rgb, shoe_items, pair_map))
        plc_rgb = draw_placement_on_rgb(rgb, row.place_line, row.tmp_palce_area)
        write_rgb_png(f"{base}_placement.png", plc_rgb)

    plan = robot.eco_shoe_placement_plan(
        head_image, carrier_box, front_edge, paired_bbox,
        move_all=move_all, timeout=20.0)
    steps = plan[0] if isinstance(plan, tuple) else plan
    if not steps: return []

    if plc_rgb is not None:
        write_rgb_png(f"{base}_plan.png", draw_plans_on_rgb(plc_rgb, steps))
    return list(steps)
