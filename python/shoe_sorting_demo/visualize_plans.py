from __future__ import annotations
from collections import defaultdict

from pathlib import Path
from typing import Sequence

import cv2
import numpy as np

from bajie_sdk import MovePlanStep, ObjectDetection

 
def _rgb_image_copy(rgb: np.ndarray) -> np.ndarray:
    arr = np.asarray(rgb, dtype=np.uint8)
    if arr.ndim != 3 or arr.shape[2] < 3:
        raise ValueError("rgb 须为 H×W×3 uint8 图像")
    return arr[:, :, :3].copy()


def _bbox_center(bbox: Sequence[int]) -> tuple[int, int]:
    x1, y1, x2, y2 = [int(v) for v in bbox[:4]]
    return (int(round((x1 + x2) * 0.5)), int(round((y1 + y2) * 0.5)))


def _draw_labeled_box(
    image: np.ndarray,
    bbox: Sequence[int],
    *,
    color: tuple[int, int, int],
    label: str,
    thickness: int = 2,
) -> None:
    x1, y1, x2, y2 = [int(v) for v in bbox[:4]]
    cv2.rectangle(image, (x1, y1), (x2, y2), color, thickness)
    (tw, th), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.55, 1)
    tx, ty = x1, max(y1 - 6, th + 4)
    cv2.rectangle(image, (tx - 2, ty - th - 4), (tx + tw + 2, ty + 2), (0, 0, 0), -1)
    cv2.putText(
        image,
        label,
        (tx, ty),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.55,
        color,
        1,
        cv2.LINE_AA,
    )


def write_rgb_png(path: str | Path, rgb: np.ndarray) -> None:
    p = Path(path)
    print(p)
    p.parent.mkdir(parents=True, exist_ok=True)
    ok = cv2.imwrite(str(p), cv2.cvtColor(rgb , cv2.COLOR_RGB2BGR))
    if not ok:
        raise OSError(f"无法写入图片: {p}")


def draw_pairs_on_rgb(
    rgb: np.ndarray,
    shoe_items: Sequence[ObjectDetection],
    pair_map: Dict[str, int],
) -> np.ndarray:
    """在头部 RGB 上绘制鞋子成双结果（pair_map = uuid→pair_id），返回 RGB。"""
    image = _rgb_image_copy(rgb)
    by_uuid = {str(item.uuid): item for item in shoe_items}
    palette = [
        (255, 200, 0), (80, 220, 80), (0, 180, 255), (160, 80, 255),
        (255, 160, 80), (255, 80, 180), (180, 230, 0), (0, 200, 200),
    ]
    # 按 pair_id 分组
    groups: Dict[int, List[str]] = defaultdict(list)
    for uuid, pid in pair_map.items():
        groups[pid].append(uuid)
    single_color = (140, 140, 140)
    for pid, uuids in groups.items():
        members = [by_uuid.get(uid) for uid in uuids]
        members = [m for m in members if m is not None]
        if len(members) == 1:
            _draw_labeled_box(image, members[0].bbox, color=single_color, label="single", thickness=1)
            continue
        color = palette[(pid - 1) % len(palette)]
        centers = []
        for item in members:
            _draw_labeled_box(image, item.bbox, color=color, label=f"P{pid}")
            center = _bbox_center(item.bbox)
            centers.append(center)
            cv2.circle(image, center, 4, color, -1, lineType=cv2.LINE_AA)
        if len(centers) >= 2:
            cv2.line(image, centers[0], centers[1], color, 2, lineType=cv2.LINE_AA)
    return image


def draw_plans_on_rgb(
    rgb: np.ndarray,
    moves: Sequence[MovePlanStep],
) -> np.ndarray:
    """在 RGB 上绘制每步的抓取框(绿)、落点框(橙)与中心箭头（输入可已含线面投影），返回 RGB。"""
    image = _rgb_image_copy(rgb)
    col_pick = (0, 200, 0)
    col_place = (255, 128, 0)
    col_arrow = (0, 255, 255)
    for move in moves:
        x1, y1, x2, y2 = map(int, move.from_bbox)
        tx1, ty1, tx2, ty2 = map(int, move.to_bbox)
        cv2.rectangle(image, (x1, y1), (x2, y2), col_pick, 2)
        cv2.rectangle(image, (tx1, ty1), (tx2, ty2), col_place, 2)
        p0 = (int(round(move.from_center[0])), int(round(move.from_center[1])))
        p1 = (int(round(move.to_center[0])), int(round(move.to_center[1])))
        cv2.arrowedLine(
            image, p0, p1, col_arrow, 1, line_type=cv2.LINE_AA, tipLength=0.06
        )
        label = f"#{move.step_id}"
        (tw, th), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.55, 1)
        tx, ty = x1, max(y1 - 6, th + 4)
        cv2.rectangle(image, (tx - 2, ty - th - 4), (tx + tw + 2, ty + 2), (0, 0, 0), -1)
        cv2.putText(
            image,
            label,
            (tx, ty),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.55,
            (255, 255, 255),
            1,
            cv2.LINE_AA,
        )
    return image


def draw_placement_on_rgb(
    rgb: np.ndarray,
    place_line: Sequence[Sequence[int]],
    place_plane: Sequence[Sequence[int]],
) -> np.ndarray:
    """在头部 RGB 上绘制线面投影：``place_line`` 为折线(绿)，``place_plane`` 为半透明面+轮廓(品红)，返回 RGB。"""
    line_pts = [list(map(int, p)) for p in place_line if len(p) >= 2]
    if len(line_pts) >= 2:
        pl = np.array(line_pts, dtype=np.int32).reshape(-1, 1, 2)
        cv2.polylines(
            rgb, [pl], isClosed=False, color=(0, 255, 0), thickness=2, lineType=cv2.LINE_AA
        )
    poly_pts = [list(map(int, p)) for p in place_plane if len(p) >= 2]
    if len(poly_pts) >= 3:
        pp = np.array(poly_pts, dtype=np.int32).reshape(-1, 1, 2)
        overlay = rgb.copy()
        cv2.fillPoly(overlay, [pp], (200, 50, 200), lineType=cv2.LINE_AA)
        cv2.addWeighted(overlay, 0.3, rgb, 0.7, 0, dst=rgb)
        cv2.polylines(
            rgb, [pp], isClosed=True, color=(120, 0, 200), thickness=2, lineType=cv2.LINE_AA
        )
    return rgb
