"""
自定义跨视图匹配模块：6D pose 空间距离 + 长轴方向 + 视觉相似度联合打分。
替代 SDK 的 eco_match_obj_views 调用。
"""

from __future__ import annotations
import math
import numpy as np
from typing import Optional, List, Tuple, Sequence, Any, Dict

from bajie_sdk.types import ObjectDetection, RGBDViewWithPose
from bajie_sdk import BajieRobot

# ---- 配置默认值 ----
SHOE_MATCHING_CONFIG = {
    "spatial_sigma": 0.8,
    "direction_sigma_deg": 80.0,   # 增大使曲线更平，小角度区间区分度降低
    "w_spatial": 0.40,             # 纯空间距离权重（不含方向）
    "w_direction": 0.30,           # 方向独立权重
    "w_visual": 0.30,
    "combined_score_threshold": 0.10,
}


# ============================================================
# 数学工具
# ============================================================

def _quat_wxyz_to_rotmat(q) -> np.ndarray:
    """四元数 (w,x,y,z) → 3×3 旋转矩阵。"""
    x, y, z, w = float(q.x), float(q.y), float(q.z), float(q.w)
    return np.array([
        [1 - 2*y*y - 2*z*z, 2*x*y - 2*w*z, 2*x*z + 2*w*y],
        [2*x*y + 2*w*z, 1 - 2*x*x - 2*z*z, 2*y*z - 2*w*x],
        [2*x*z - 2*w*y, 2*y*z + 2*w*x, 1 - 2*x*x - 2*y*y],
    ], dtype=np.float64)


def _long_axis_xoy(R: np.ndarray, box_length) -> Optional[Tuple[float, float]]:
    """物体长轴在 XOY 平面投影的单位方向向量。

    三条轴旋转到世界系，排除 XOY 投影最小的（垂直轴），
    剩余中取 box_length 最大者归一化。
    """
    b = box_length
    projections = []
    for length, axis in [(b.x, (1, 0, 0)), (b.y, (0, 1, 0)), (b.z, (0, 0, 1))]:
        if length <= 0: continue
        wx, wy, _ = R @ axis
        projections.append((length, math.sqrt(wx * wx + wy * wy), wx, wy))
    if len(projections) < 2: return None
    projections.sort(key=lambda a: a[1])
    projections.pop(0)
    best = max(projections, key=lambda a: a[0])
    norm = best[1]
    return (best[2] / norm, best[3] / norm) if norm > 1e-9 else None


def cosine_similarity(a: list, b: list) -> float:
    """两个向量的余弦相似度 [0, 1]."""
    dot = sum(ai * bi for ai, bi in zip(a, b))
    norm_a = math.sqrt(sum(ai * ai for ai in a))
    norm_b = math.sqrt(sum(bi * bi for bi in b))
    if norm_a == 0.0 or norm_b == 0.0: return 0.0
    return dot / (norm_a * norm_b)


# ============================================================
# 特征提取（head / arm 共用）
# ============================================================

def _batch_extract_pose_and_emb(
    img: np.ndarray,
    bboxes: Sequence[Tuple[int, int, int, int]],
    view: RGBDViewWithPose,
    robot: BajieRobot,
    robot_x: float, robot_y: float, robot_yaw: float,
) -> List[Dict[str, Any]]:
    """批量提取 bbox 的 3D 位置、长轴方向、视觉 embedding。

    robot_x/y/yaw 为 map 系底盘位姿，用于将 base_footprint 系 pose 转到 map 系。
    """
    num = len(bboxes)
    crops = [img[b[1]:b[3], b[0]:b[2]] for b in bboxes]

    # map 系齐次变换（base_footprint → map）
    c, s = math.cos(robot_yaw), math.sin(robot_yaw)
    H = np.array([[c, -s, 0, robot_x], [s, c, 0, robot_y],
                  [0, 0, 1, 0], [0, 0, 0, 1]], dtype=np.float64)
    R_yaw = H[:3, :3]

    positions: list = [None] * num; directions: list = [None] * num; embeddings: list = [None] * num
    try:
        if poses := robot.eco_computeObjectPose(bboxes, view, timeout_sec=6.0):
            for i, obj_pose in enumerate(poses):
                if obj_pose is None: continue
                p, R_obj = obj_pose.position, _quat_wxyz_to_rotmat(obj_pose.orientation)
                if getattr(obj_pose, "frame_id", "") not in ("map", ""):
                    hp, R_obj = H @ [p.x, p.y, p.z, 1.0], R_yaw @ R_obj
                    positions[i] = (float(hp[0]), float(hp[1]), float(hp[2]))
                else:
                    positions[i] = (p.x, p.y, p.z)
                directions[i] = _long_axis_xoy(R_obj, obj_pose.box_length)
        embeddings = robot.eco_build_embeddings(crops, timeout=10.0)
    except Exception:
        pass

    return [{"pos": positions[i], "dir": directions[i], "emb": embeddings[i]}
            for i in range(num)]


# ============================================================
# 联合打分
# ============================================================

def _score_candidate(head_ref: Dict[str, Any], candidate: Dict[str, Any],
                     config: Dict[str, Any]) -> float:
    """计算 head 鞋参考与单个 arm 候选鞋的联合得分 [0, 1]，返回组合得分。"""
    sigma = config["spatial_sigma"]
    dir_sigma = math.radians(config["direction_sigma_deg"])
    w_spatial = config["w_spatial"]
    w_direction = config.get("w_direction", 0.0)
    w_visual = config["w_visual"]

    spatial_raw = 0.0
    dir_score = 1.0
    if head_ref["pos"] is not None and candidate["pos"] is not None:
        head_pos, cand_pos = head_ref["pos"], candidate["pos"]
        dx, dy, dz = head_pos[0] - cand_pos[0], head_pos[1] - cand_pos[1], head_pos[2] - cand_pos[2]
        spatial_dist = math.sqrt(dx * dx + dy * dy + dz * dz)
        spatial_raw = math.exp(-spatial_dist * spatial_dist / (2.0 * sigma * sigma))

        if head_ref["dir"] is not None and candidate["dir"] is not None:
            dot = abs(head_ref["dir"][0] * candidate["dir"][0] +
                      head_ref["dir"][1] * candidate["dir"][1])
            dot = max(-1.0, min(1.0, dot))
            angle = math.acos(dot)
            dir_score = math.exp(-angle * angle / (2.0 * dir_sigma * dir_sigma))

    visual_raw = 0.0
    if head_ref["emb"] is not None and candidate["emb"] is not None:
        visual_raw = cosine_similarity(head_ref["emb"], candidate["emb"])

    return w_spatial * spatial_raw + w_direction * dir_score + w_visual * visual_raw


# ============================================================
# 编排：自定义跨视图匹配
# ============================================================

def custom_match_obj_views(
    source_view: RGBDViewWithPose,
    ref_bbox: Tuple[int, int, int, int],
    target_view: RGBDViewWithPose,
    target_detections: Sequence[ObjectDetection],
    robot: BajieRobot,
    config: Optional[Dict[str, Any]] = None,
) -> Optional[Tuple[int, int, int, int]]:
    """跨视图匹配编排：提取特征 -> 联合打分 -> 选最佳候选。

    返回臂图中匹配到的 bbox；无可信匹配返回 None。
    """
    if config is None:
        config = SHOE_MATCHING_CONFIG

    rp = robot.eco_robotPosition()
    if not rp: return None
    robot_x, robot_y, robot_yaw = float(rp.x), float(rp.y), float(rp.yaw)

    # 提取 head 参考鞋的 3D 位姿 + embedding
    [head_ref] = _batch_extract_pose_and_emb(
        source_view.rgb_image.img, [ref_bbox], source_view, robot,
        robot_x, robot_y, robot_yaw)
    if head_ref["pos"] is None and head_ref["emb"] is None:
        return None

    # 提取臂图中所有候选鞋的信息
    if not target_detections:
        return None
    arm_bboxes = [(int(b[0]), int(b[1]), int(b[2]), int(b[3]))
                  for d in target_detections
                  for b in [getattr(d, "bbox", (0, 0, 0, 0))]]
    candidates = _batch_extract_pose_and_emb(
        target_view.rgb_image.img, arm_bboxes, target_view, robot,
        robot_x, robot_y, robot_yaw)

    # 对每个候选打分，选最高分
    best_bbox, best_score = None, -1.0
    for i, cand in enumerate(candidates):
        score = _score_candidate(head_ref, cand, config)
        if score > best_score:
            best_score, best_bbox = score, arm_bboxes[i]

    if best_bbox is None or best_score < config["combined_score_threshold"]:
        print(f"[WARN] 跨视图匹配失败，回退到臂检测 bbox", flush=True)
        for i, d in enumerate(target_detections):
            print(f"  arm[{i}]: bbox={getattr(d, 'bbox', 'N/A')} name={getattr(d, 'name', 'N/A')}", flush=True)
        if target_detections:
            fallback = getattr(target_detections[0], "bbox", None)
            if fallback:
                best_bbox = (int(fallback[0]), int(fallback[1]), int(fallback[2]), int(fallback[3]))
                print(f"[WARN] 使用臂检测结果: {best_bbox}", flush=True)
                return best_bbox
        return None
    return best_bbox
