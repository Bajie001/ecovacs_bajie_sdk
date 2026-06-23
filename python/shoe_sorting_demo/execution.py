from __future__ import annotations

from typing import Callable, List, Sequence, Tuple

import numpy as np

from bajie_sdk import (
    BajieRobot,
    CameraType,
    DetectObjectsRequest,
    MissionStatus,
    MovePlanStep,
    NamedBBox,
    ObjectDetection,
    ObjectPose3D,
    OvdEndpoint,
    Quatf,
    RGBDViewWithPose,
    Vec3f,
)
try:
    from .matching import custom_match_obj_views, _quat_wxyz_to_rotmat
except ImportError:
    from matching import custom_match_obj_views, _quat_wxyz_to_rotmat


def _normalize_or_raise(v: np.ndarray, *, err_msg: str) -> np.ndarray:
    n = float(np.linalg.norm(v))
    if n < 1e-9:
        raise ValueError(err_msg)
    return (v / n).astype(np.float64, copy=False)


def _rotmat_to_quatf(rot_mat: np.ndarray) -> Quatf:
    m = np.asarray(rot_mat, dtype=np.float64).reshape(3, 3)
    decision = np.empty(4, dtype=np.float64)
    decision[0] = m[0, 0]
    decision[1] = m[1, 1]
    decision[2] = m[2, 2]
    decision[3] = m[0, 0] + m[1, 1] + m[2, 2]
    choice = int(np.argmax(decision))
    quat = np.empty(4, dtype=np.float64)

    if choice != 3:
        i = choice
        j = (i + 1) % 3
        k = (j + 1) % 3
        quat[i] = 1.0 - decision[3] + 2.0 * m[i, i]
        quat[j] = m[j, i] + m[i, j]
        quat[k] = m[k, i] + m[i, k]
        quat[3] = m[k, j] - m[j, k]
    else:
        quat[0] = m[2, 1] - m[1, 2]
        quat[1] = m[0, 2] - m[2, 0]
        quat[2] = m[1, 0] - m[0, 1]
        quat[3] = 1.0 + decision[3]

    norm = float(np.sqrt(np.dot(quat, quat)))
    if not np.isfinite(norm) or norm < 1e-15:
        return Quatf(0.0, 0.0, 0.0, 1.0)
    quat /= norm
    return Quatf(float(quat[0]), float(quat[1]), float(quat[2]), float(quat[3]))


def _front_edge_horizontal_x_axis(
    head_image: RGBDViewWithPose,
    front_edge: Sequence[Sequence[float]],
) -> np.ndarray:
    if len(front_edge) < 2:
        raise ValueError("front_edge 至少需要两个点")
    p0 = np.asarray(front_edge[0], dtype=np.float64)
    p1 = np.asarray(front_edge[-1], dtype=np.float64)
    if p0.shape[0] < 2 or p1.shape[0] < 2:
        raise ValueError("front_edge 点至少需要两个坐标分量")

    edge = p1[:2] - p0[:2]
    if float(np.linalg.norm(edge)) < 1e-9:
        raise ValueError("front_edge 两端点过近，无法构造方向")

    z_axis = np.array([0.0, 0.0, 1.0], dtype=np.float64)
    edge_3d = np.array([edge[0], edge[1], 0.0], dtype=np.float64)
    x_axis = _normalize_or_raise(np.cross(z_axis, edge_3d), err_msg="front_edge 退化，无法构造水平法向")

    cam_x = _quat_wxyz_to_rotmat(head_image.tf_goal.orientation)[:, 2]
    cam_x = np.array([cam_x[0], cam_x[1], 0.0], dtype=np.float64)
    cam_norm = float(np.linalg.norm(cam_x))
    if cam_norm >= 1e-9 and float(np.dot(x_axis, cam_x / cam_norm)) < 0.0:
        x_axis = -x_axis
    return x_axis


def _place_quat_from_front_edge(
    head_image: RGBDViewWithPose,
    front_edge: Sequence[Sequence[float]],
) -> Quatf:
    x_axis = _front_edge_horizontal_x_axis(head_image, front_edge)
    z_axis = np.array([0.0, 0.0, 1.0], dtype=np.float64)
    y_axis = np.cross(z_axis, x_axis)
    y_axis /= float(np.linalg.norm(y_axis))
    x_axis = np.cross(y_axis, z_axis)
    x_axis /= float(np.linalg.norm(x_axis))
    return _rotmat_to_quatf(np.column_stack([x_axis, y_axis, z_axis]))

def bbox_center(bbox:tuple[int, int, int, int]) -> Tuple[float, float]:
    """目标框几何中心 ``(u, v)``（像素，浮点）。"""
    x1, y1, x2, y2 = bbox
    return ((x1 + x2) * 0.5, (y1 + y2) * 0.5)


def _say(robot: BajieRobot, text: str, *, no_voice: bool = False) -> None:
    print(f"[voice] {text}", flush=True)
    if no_voice:
        return
    try:
        robot.eco_robotSpeech(text)
    except Exception:
        pass


def _mission_succeeded(status: MissionStatus) -> bool:
    return int(status.error_info.code) == 0


def _place_position_pixel_ray_ground_z0(
    head_image: RGBDViewWithPose,
    uv: Tuple[float, float],
) -> Vec3f:
    """无有效深度时的回退：由像素与 ``tf_goal`` 反算放置点，相机射线与父系 **z=0** 平面求交，高度取 0。

    与 ``pixel_to_point_3d`` 共用内参语义：相机系 ``(x,y,z)`` 中 ``z`` 沿光轴向前；父系为
    ``tf_goal.header.frame_id``，``tf_goal.position`` 为相机原点在父系中的位置。
    """
    k = head_image.camera_info_k
    u, v = float(uv[0]), float(uv[1])
    fx, fy = float(k[0, 0]), float(k[1, 1])
    cx, cy = float(k[0, 2]), float(k[1, 2])
    if min(fx, fy) <= 0:
        raise ValueError("_place_position_pixel_ray_ground_z0: invalid intrinsics (fx/fy)")
    d_cam = np.array([(u - cx) / fx, (v - cy) / fy, 1.0], dtype=np.float64)
    R = _quat_wxyz_to_rotmat(head_image.tf_goal.orientation)
    t = np.array(
        [
            head_image.tf_goal.position.x,
            head_image.tf_goal.position.y,
            head_image.tf_goal.position.z,
        ],
        dtype=np.float64,
    )
    d_parent = R @ d_cam
    dz = float(d_parent[2])
    if abs(dz) < 1e-9:
        raise ValueError("_place_position_pixel_ray_ground_z0: ray parallel to z=0 (dz≈0)")
    s = -float(t[2]) / dz
    if s < 0:
        raise ValueError("_place_position_pixel_ray_ground_z0: intersection behind camera (s<0)")
    p = t + s * d_parent
    return Vec3f(x=float(p[0]), y=float(p[1]), z=0.0)


def mission_retry(*, attempts: int = 3, op_name: str = "mission",
                  robot: BajieRobot | None = None, no_voice: bool = False,
                  op_cn: str = "操作",
                  fail_do: Callable[[], None] | None = None,
                  ) -> Callable[[Callable[[], MissionStatus]], MissionStatus]:
    """对无参、返回 ``MissionStatus`` 的调用做重试（装饰器工厂用法）。

    用法：``mission_retry(attempts=3, op_name="pick")(lambda: robot.foo(...))``。
    单次失败：``error_info.code != 0`` 或调用抛 ``Exception``，均重试；用尽则 ``RuntimeError``。
    """

    def run(fn: Callable[[], MissionStatus]) -> MissionStatus:
        last_failure = ""
        for attempt in range(1, attempts + 1):
            try:
                status = fn()
            except Exception as exc:
                last_failure = "attempt %s/%s exception: %s" % (attempt, attempts, exc)
                print(
                    "[mission_retry] %s %s/%s failed (exception): %s"
                    % (op_name, attempt, attempts, exc),
                    flush=True,
                )
                if attempt < attempts and robot:
                    _say(robot, f"{op_cn}未成功，正在重试，别着急", no_voice=no_voice)
                if fail_do and attempt < attempts:
                    try: fail_do()
                    except Exception: pass
                continue
            if _mission_succeeded(status):
                print(
                    "[mission_retry] %s ok (attempt %s/%s)" % (op_name, attempt, attempts),
                    flush=True,
                )
                return status
            last_failure = "attempt %s/%s error_info=%s" % (attempt, attempts, status.error_info)
            print(
                "[mission_retry] %s %s/%s failed (mission status)" % (op_name, attempt, attempts),
                flush=True,
            )
            if attempt < attempts and robot:
                _say(robot, f"{op_cn}未成功，正在重试，别着急", no_voice=no_voice)
            if fail_do and attempt < attempts:
                try: fail_do()
                except Exception: pass
        print("[mission_retry] %s giving up after %s attempts" % (op_name, attempts), flush=True)
        raise RuntimeError("%s failed after %s attempts: %s" % (op_name, attempts, last_failure))

    return run


def eco_pick_with_arm_review_head_image_ref(
    robot: BajieRobot,
    view: RGBDViewWithPose,
    object_bbox: NamedBBox,
    head_image_ref_bbox: tuple[int, int, int, int],
    *,
    timeout_sec: float = 100.0,
) -> MissionStatus:
    """臂对准（``eco_lookto``）→ 臂图重检 → 跨视匹配 → ``eco_pick``。

    ``head_image_ref_bbox`` 为链首参考框；``object_bbox.bbox`` 为当步规划 ``from``。二者相等时为
    链上首步，对头图 bbox 做 ``lookto``；否则用链首框中心提 3D 再 ``lookto``。裁切匹配始终用
    ``head_image_ref_bbox``。
    """
    if head_image_ref_bbox == object_bbox.bbox:  # 首次移动
        _ = robot.eco_lookto(
            object_bbox.bbox,
            view=view,
            camera_type=CameraType.ARM,
            timeout_sec=120,
        )
    else:  # 链上后续步（from 可能为虚拟，用链首 3D 对准）
        look_pose = ObjectPose3D(
            frame_id=view.tf_goal.header.frame_id,
            position=view.pixel_to_point_3d(bbox_center(head_image_ref_bbox), True, 5),
            box_length=Vec3f(0.12, 0.1, 0.05),
        )
        _ = robot.eco_lookto(look_pose, camera_type=CameraType.ARM, timeout_sec=120)
    arm_image = robot.eco_captureImages(CameraType.ARM)
    arm_image.save_images_to_local("arm")
    arm_items: list[ObjectDetection] = list(
        robot.eco_detect_objects(
            DetectObjectsRequest(
                rgb_image=arm_image.rgb_image.img,
                labels=[object_bbox.name] if object_bbox.name else ["object"],
                entry=OvdEndpoint.SHOE,
            )
        ).items
    )
    matched_bbox = custom_match_obj_views(view, head_image_ref_bbox,
                                          arm_image, arm_items, robot)
    if matched_bbox is None:
        print(f"[WARN] 跨视图匹配失败，跳过鞋 {head_image_ref_bbox}", flush=True)
        for i, item in enumerate(arm_items):
            print(f"  arm[{i}]: bbox={getattr(item, 'bbox', 'N/A')} name={getattr(item, 'name', 'N/A')}", flush=True)
        raise RuntimeError("跨视图匹配失败")
    bbox = NamedBBox(id=0, name="shoe", bbox=matched_bbox)
    bbox.name += "鞋"  # 启动鞋子抓取模式
    status = robot.eco_pick(arm_image, bbox, timeout_sec=timeout_sec)
    return status


def execute_pick_and_place(
    robot: BajieRobot,
    label: str,
    head_image: RGBDViewWithPose,
    move: MovePlanStep,
    front_edge: Sequence[Sequence[float]],
    *,
    head_image_ref_bbox: tuple[int, int, int, int],
    no_voice: bool = False,
) -> None:
    """单步：臂图跨视抓（内含 ``lookto``）→ 按规划放。

    ``head_image_ref_bbox`` 是规划链在**首帧头图**上归约出的参考框，``move.from_bbox`` 是规划在
    当前步给出的位置（多步时可能被模拟改写）。

    * **首次移动**（``head_image_ref_bbox == move.from_bbox``）：同一条链在头图上的头一步，用头图
      bbox 给 ``eco_lookto``，与未拆链的 SDK 行为一致。
    * **链上后续步**（与 ``from`` 不相等，含「同物体第二次及以后」）：当步 ``from`` 与头图不对应，
      用 **链首** ``head_image_ref_bbox`` 的像素中心提 3D 点再 ``lookto``，避免用虚拟 from 对准。

    上述对准与抓取均在 :func:`eco_pick_with_arm_review_head_image_ref` 内完成；裁切始终用链首
    ``head_image_ref_bbox``。抓取、放置各自最多尝试 3 次；若 SDK 直接抛异常或返回非成功
    ``MissionStatus``，均计为一次失败并重试，用尽后 ``RuntimeError``。
    """
    print(
        "[execute_pick_and_place] begin label=%r first_step=%s to_center=%s"
        % (label, head_image_ref_bbox == move.from_bbox, move.to_center),
        flush=True,
    )

    _say(robot, "开始抓取", no_voice=no_voice)
    _ = mission_retry(attempts=3, op_name="pick",
                      robot=robot, no_voice=no_voice, op_cn="抓取",
                      fail_do=lambda: robot.eco_cancelAllMissions())(
        lambda: eco_pick_with_arm_review_head_image_ref(
            robot,
            head_image,
            object_bbox=NamedBBox(name=label, bbox=move.from_bbox),
            head_image_ref_bbox=head_image_ref_bbox,
            timeout_sec=120.0,
        )
    )

    _say(robot, "抓取成功，开始放置", no_voice=no_voice)

    # 规划 to 中心 + 前沿定水平朝向，6D 放置（优先深度反投影，失败则射线 ∩ z=0）
    try:
        place_position = head_image.pixel_to_point_3d(move.to_center, True, 5)
    except ValueError:
        place_position = _place_position_pixel_ray_ground_z0(head_image, move.to_center)
        print(place_position, flush=True)
    place_pose = ObjectPose3D(
        frame_id=head_image.tf_goal.header.frame_id,
        position=place_position,
        orientation=_place_quat_from_front_edge(head_image, front_edge),
        box_length=Vec3f(0.12, 0.1, 0.05),
    )
    _ = mission_retry(attempts=3, op_name="place_3D",
                      robot=robot, no_voice=no_voice, op_cn="放置")(
        lambda: robot.eco_place_3D(place_pose, timeout_sec=120.0)
    )

    _say(robot, "放置成功", no_voice=no_voice)
    print("[execute_pick_and_place] done label=%r" % (label,), flush=True)
