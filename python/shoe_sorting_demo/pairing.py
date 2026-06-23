from collections import defaultdict
from typing import Any, Dict, List, Sequence
import numpy as np
from bajie_sdk import BajieRobot, ObjectDetection, RGBDViewWithPose

def crop_rgb_patch(rgb_hwc: Any, bbox: Sequence[int]) -> Any:
    if len(bbox) != 4: raise ValueError(f"invalid bbox: {bbox}")
    arr = np.asarray(rgb_hwc)
    if arr.ndim != 3 or arr.shape[2] < 3: raise ValueError("rgb_hwc must be HxWxC with C>=3")
    h, w = int(arr.shape[0]), int(arr.shape[1])
    x1 = max(0, min(int(bbox[0]), w - 1))
    y1 = max(0, min(int(bbox[1]), h - 1))
    x2 = max(x1 + 1, min(int(bbox[2]), w))
    y2 = max(y1 + 1, min(int(bbox[3]), h))
    return np.ascontiguousarray(arr[y1:y2, x1:x2])

def pair_shoes(robot: BajieRobot, image: RGBDViewWithPose,
               shoe_items: Sequence[ObjectDetection]) -> Dict[str, int]:
    """标签优先配对 → pairwise 兜底，uuid→pair_id（含未配对鞋各自独立 id）."""
    groups: Dict[str, List[ObjectDetection]] = defaultdict(list)
    leftovers: List[ObjectDetection] = []
    for it in shoe_items:
        label = (it.name or "").strip()
        if label: groups[label].append(it)
        else: leftovers.append(it)
    pairs: List[List[str]] = []
    for items in groups.values():
        if len(items) == 2: pairs.append([items[0].uuid, items[1].uuid])
        else: leftovers.extend(items)

    if leftovers:
        pair_map: Dict[str, List[str]] = defaultdict(list)
        try:
            for pair_id, item_id in robot.eco_pairwise([(image.rgb_image.img, list(leftovers))]):
                pair_map[pair_id].append(item_id)
            for group in pair_map.values():
                if len(group) == 2: pairs.append(group)
        except Exception: pass
        
    next_id = 1
    mapping: Dict[str, int] = {}
    for pair in pairs:
        for uuid in pair: mapping[uuid] = next_id
        next_id += 1
    for it in shoe_items:
        if it.uuid not in mapping:
            mapping[it.uuid] = next_id
            next_id += 1
    return mapping
