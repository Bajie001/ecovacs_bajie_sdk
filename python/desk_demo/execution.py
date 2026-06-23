#!/usr/bin/env python3
"""
执行工作流 (Execution Workflow)
"""

import logging
import os
from typing import Any, Dict, List, Optional, Sequence, Tuple

from bajie_sdk import (
    BajieRobot,
    CameraType,
    DeskIntentMatchRequest,
    NamedBBox,
    ObjectPose3D,
    RGBDViewWithPose,
    Quatf,
    Vec3f,
)

from ref_utils import ensure_dir, new_task_id, write_json
from retry_utils import (
    ERR_BBOX_MATCH_FAILED,
    ERR_FALLBACK_PLACE_FAILED,
    ERR_FROM_POSE_MISSING,
    ERR_GRAB_FAILED,
    ERR_IMAGE_QUERY_FAILED,
    ERR_PLACE_FAILED,
    ERR_PLACE_IN_FAILED,
    ERR_POSE_FAILED,
    error_tag,
    mission_failed,
    retry_run,
)
from speech import SPEECH_GRABBING, SPEECH_PLACING, speak

logger = logging.getLogger(__name__)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "outputs")


def _get_pose_result_dict(item: ObjectPose3D) -> Dict[str, Any]:
    return {
        "id": item.id,
        "position": {"x": item.position.x, "y": item.position.y, "z": item.position.z},
        "orientation": {"x": item.orientation.x, "y": item.orientation.y, "z": item.orientation.z, "w": item.orientation.w},
        "box_length": {"x": item.box_length.x, "y": item.box_length.y, "z": item.box_length.z},
        "frame_id": item.frame_id,
    }


def get_pose(
    robot: BajieRobot,
    bboxes: Any,
    view: RGBDViewWithPose,
    output_dir: str = "outputs",
) -> Tuple[bool, Dict[str, Any] | None]:
    """计算6D姿态

    Args:
        robot: 已连接的 BajieRobot 实例
        bboxes: bbox列表，每个元素包含 id, left_up_x, left_up_y, right_down_x, right_down_y
        view: 头部相机 RGBDViewWithPose
        output_dir: 输出目录

    Returns:
        (success, result_dict) 其中 result_dict 包含 task_id 和 pose_results
    """
    task_id = new_task_id()

    logger.info(f"6D姿态计算入参: task_id={task_id}")
    logger.info(f"  bbox数量: {len(bboxes) if bboxes else 0}")

    try:
        named_bbox = []
        for x in (bboxes or []):
            if not isinstance(x, dict):
                continue
            bid = int(x.get("id", 0))
            x1 = int(x.get("left_up_x", 0))
            y1 = int(x.get("left_up_y", 0))
            x2 = int(x.get("right_down_x", 0))
            y2 = int(x.get("right_down_y", 0))
            named_bbox.append(NamedBBox(id=bid, name=f"bbox_{bid}", bbox=(x1, y1, x2, y2)))

        if not named_bbox:
            logger.error("%s bbox 列表为空", error_tag(ERR_POSE_FAILED))
            return False, None

        def _compute():
            pose_results_raw = robot.eco_computeObjectPose(
                named_bbox=named_bbox,
                view=view,
                timeout_sec=180.0,
            )
            pose_results: List[Dict[str, Any]] = []
            for item in pose_results_raw:
                try:
                    pose_results.append(_get_pose_result_dict(item))
                except Exception:
                    continue
            if not pose_results:
                raise RuntimeError("no pose results from eco_computeObjectPose")
            return pose_results_raw, pose_results

        _, pose_results = retry_run(_compute, name="eco_computeObjectPose")

        output_file = os.path.join(output_dir, "step5_get_pose.json")
        write_json(output_file, {
            "task_id": task_id,
            "status": "success",
            "pose_results": pose_results,
        })
        logger.info(f"6D姿态计算完成，输出已保存到: {output_file}")

        return True, {"task_id": task_id, "pose_results": pose_results}

    except Exception as e:
        logger.exception("%s 6D姿态计算执行异常: %s", error_tag(ERR_POSE_FAILED), e)
        return False, None


def pose_adaptive(
    robot: BajieRobot,
    frame_id: str,
    position: Any,
    orientation: Any,
    box_length: Any,
    output_dir: str = "outputs",
) -> Tuple[bool, Dict[str, Any] | None]:
    """自适应观测（手部版本）

    Args:
        robot: 已连接的 BajieRobot 实例
        frame_id: 坐标系ID
        position: 位置字典
        orientation: 方向字典
        box_length: 包围盒尺寸字典
        output_dir: 输出目录

    Returns:
        (success, result_dict)
    """
    task_id = new_task_id()
    try:
        pos = position if isinstance(position, dict) else {}
        ori = orientation if isinstance(orientation, dict) else {}
        box = box_length if isinstance(box_length, dict) else {}

        pose = ObjectPose3D(
            id=0,
            frame_id=str(frame_id or "map"),
            position=Vec3f(
                x=float(pos.get("x", 0.0) or 0.0),
                y=float(pos.get("y", 0.0) or 0.0),
                z=float(pos.get("z", 0.0) or 0.0),
            ),
            orientation=Quatf(
                x=float(ori.get("x", 0.0) or 0.0),
                y=float(ori.get("y", 0.0) or 0.0),
                z=float(ori.get("z", 0.0) or 0.0),
                w=float(ori.get("w", 1.0) or 1.0),
            ),
            box_length=Vec3f(
                x=float(box.get("x", 0.0) or 0.0),
                y=float(box.get("y", 0.0) or 0.0),
                z=float(box.get("z", 0.0) or 0.0),
            ),
        )
        def _look():
            st = robot.eco_lookto(pose, camera_type=CameraType.ARM, timeout_sec=180.0)
            fail = mission_failed(st)
            if fail:
                raise RuntimeError(fail)
            return st

        retry_run(_look, name="eco_lookto")

        output_file = os.path.join(output_dir, "step6_pose_adaptive.json")
        write_json(output_file, {
            "task_id": task_id,
            "frame_id": frame_id,
            "position": position,
            "orientation": orientation,
            "box_length": box_length,
            "status": "success",
        })
        logger.info(f"位置自适应完成，输出已保存到: {output_file}")

        return True, {"task_id": task_id}

    except Exception as e:
        logger.exception("位置自适应执行异常: %s", e)
        return False, None


def run_pose_adaptive_from_pose(
    robot: BajieRobot,
    pose_data: Dict[str, Any],
    output_dir: str = "outputs",
) -> bool:
    frame_id = pose_data.get("frame_id", "map")
    position = pose_data.get("position")
    orientation = pose_data.get("orientation")
    box_length = pose_data.get("box_length")

    success, _ = pose_adaptive(robot, frame_id, position, orientation, box_length, output_dir)
    return success


def accurate_grab(
    robot: BajieRobot,
    arm_view: RGBDViewWithPose,
    bbox: Any,
    item_name: str = "",
    output_dir: str = "outputs",
) -> Tuple[bool, Dict[str, Any] | None]:
    """物品抓取

    Args:
        robot: 已连接的 BajieRobot 实例
        arm_view: 手臂相机 RGBDViewWithPose
        bbox: 物品bbox（列表 [x1,y1,x2,y2] 或字典）
        item_name: 物品名称
        output_dir: 输出目录

    Returns:
        (success, result_dict)
    """
    task_id = new_task_id()
    try:
        if isinstance(bbox, list):
            bb = [int(b) if i < 4 else 0 for i, b in enumerate(bbox)]
            named_bbox = NamedBBox(id=0, name=str(item_name or ""), bbox=tuple(bb[:4]))
        else:
            bbox_dict = dict(bbox or {})
            bb = bbox_dict.get("bbox", [0, 0, 0, 0])
            named_bbox = NamedBBox(
                id=int(bbox_dict.get("id", 0)),
                name=str(item_name or bbox_dict.get("name", "")),
                bbox=tuple(int(bb[i]) if i < len(bb) else 0 for i in range(4)),
            )

        def _pick():
            st = robot.eco_pick(arm_view, named_bbox, timeout_sec=300.0)
            fail = mission_failed(st)
            if fail:
                raise RuntimeError(fail)
            return st

        retry_run(_pick, name="eco_pick")

        ensure_dir(output_dir)
        output_file = os.path.join(output_dir, "step7_accurate_grab.json")
        write_json(output_file, {
            "task_id": task_id,
            "item_name": item_name,
            "bbox": bbox,
            "status": "success",
        })
        logger.info(f"物品抓取完成，输出已保存到: {output_file}")

        return True, {"task_id": task_id, "item_name": item_name}

    except Exception as e:
        logger.exception("物品抓取执行异常: %s", e)
        return False, None


def run_accurate_grab(
    robot: BajieRobot,
    arm_view: RGBDViewWithPose,
    item_bbox: Any,
    item_name: str = "",
    output_dir: str = "outputs",
) -> bool:
    success, _ = accurate_grab(robot, arm_view, item_bbox, item_name, output_dir)
    return success


def accurate_place(
    robot: BajieRobot,
    frame_id: str,
    position: Any,
    orientation: Any,
    box_length: Any,
    output_dir: str = "outputs",
) -> Tuple[bool, Dict[str, Any] | None]:
    """精准放置

    Args:
        robot: 已连接的 BajieRobot 实例
        frame_id: 坐标系ID
        position: 位置字典
        orientation: 方向字典
        box_length: 包围盒尺寸字典
        output_dir: 输出目录

    Returns:
        (success, result_dict)
    """
    task_id = new_task_id()
    try:
        pos = position if isinstance(position, dict) else {}
        ori = orientation if isinstance(orientation, dict) else {}
        box = box_length if isinstance(box_length, dict) else {}

        pose = ObjectPose3D(
            id=0,
            frame_id=str(frame_id or "map"),
            position=Vec3f(
                x=float(pos.get("x", 0.0) or 0.0),
                y=float(pos.get("y", 0.0) or 0.0),
                z=float(pos.get("z", 0.0) or 0.0),
            ),
            orientation=Quatf(
                x=float(ori.get("x", 0.0) or 0.0),
                y=float(ori.get("y", 0.0) or 0.0),
                z=float(ori.get("z", 0.0) or 0.0),
                w=float(ori.get("w", 1.0) or 1.0),
            ),
            box_length=Vec3f(
                x=float(box.get("x", 0.0) or 0.0),
                y=float(box.get("y", 0.0) or 0.0),
                z=float(box.get("z", 0.0) or 0.0),
            ),
        )
        def _place():
            st = robot.eco_place_3D(pose, timeout_sec=300.0)
            fail = mission_failed(st)
            if fail:
                raise RuntimeError(fail)
            return st

        retry_run(_place, name="eco_place_3D")

        output_file = os.path.join(output_dir, "step8_accurate_place.json")
        write_json(output_file, {
            "task_id": task_id,
            "frame_id": frame_id,
            "position": position,
            "orientation": orientation,
            "box_length": box_length,
            "status": "success",
        })
        logger.info(f"精准放置完成，输出已保存到: {output_file}")

        return True, {"task_id": task_id}

    except Exception as e:
        logger.exception("精准放置执行异常: %s", e)
        return False, None


def run_accurate_place(
    robot: BajieRobot,
    pose_data: Dict[str, Any],
    output_dir: str = "outputs",
) -> bool:
    frame_id = pose_data.get("frame_id", "map")
    position = pose_data.get("position")
    orientation = pose_data.get("orientation")
    box_length = pose_data.get("box_length")

    success, _ = accurate_place(robot, frame_id, position, orientation, box_length, output_dir)
    return success


def call_item_bbox_match(
    robot: BajieRobot,
    arm_view: RGBDViewWithPose,
    head_view: RGBDViewWithPose,
    bbox: Sequence[Any],
) -> Tuple[bool, List[int]]:
    """调用 SDK ``eco_vlm_match``：头部图上的 bbox → 手臂图像素框。"""
    bb = list(bbox) if isinstance(bbox, (list, tuple)) else []
    if len(bb) != 4:
        raise ValueError(f"bbox must be [x1,y1,x2,y2], got: {bbox}")

    logger.info("调用 eco_vlm_match 进行 bbox 匹配")
    out = retry_run(
        lambda: robot.eco_vlm_match(
            DeskIntentMatchRequest(
                arm_image=arm_view.rgb_image.img,
                base_image=head_view.rgb_image.img,
                bbox=(int(bb[0]), int(bb[1]), int(bb[2]), int(bb[3])),
            ),
            timeout=180.0,
        ),
        name="eco_vlm_match",
    )
    return True, [int(out[0]), int(out[1]), int(out[2]), int(out[3])]


def run_item_bbox_match(
    robot: BajieRobot,
    arm_view: RGBDViewWithPose,
    head_view: RGBDViewWithPose,
    from_bbox: Sequence[Any],
    output_dir: str = "outputs",
) -> Tuple[bool, List[int] | None]:
    """运行物品 bbox 获取（``eco_vlm_match``，入参与 SDK 文档一致为 RGB ndarray）。"""
    try:
        _ = arm_view.rgb_image.img
        _ = head_view.rgb_image.img
    except Exception as e:
        logger.error("%s 读取头部/手臂 RGB 图像失败: %s", error_tag(ERR_BBOX_MATCH_FAILED), e)
        return False, None

    try:
        success, item_bbox = call_item_bbox_match(robot, arm_view, head_view, from_bbox)
    except Exception as e:
        logger.exception("物品bbox获取执行异常: %s", e)
        return False, None

    if success and item_bbox:
        output_file = os.path.join(output_dir, "step11_item_bbox.json")
        write_json(output_file, {"status": "success", "bbox": item_bbox})
        logger.info(f"物品bbox获取完成，输出已保存到: {output_file}")
        logger.info(f"物品bbox: {item_bbox}")

    return success, item_bbox if success else None


def arm_image_query(
    robot: BajieRobot,
    output_dir: str = OUTPUT_DIR,
) -> Tuple[bool, Optional[RGBDViewWithPose]]:
    """手臂相机拍照

    Returns:
        (success, arm_view) 其中 arm_view 为原始 RGBDViewWithPose
    """
    task_id = new_task_id()
    logger.info("=" * 50)
    logger.info("手臂相机拍照")
    logger.info("=" * 50)

    try:
        def _cap():
            resp = robot.eco_captureImages(CameraType.ARM, timeout_sec=120.0)
            fail = mission_failed(resp)
            if fail:
                raise RuntimeError(fail)
            return resp

        resp = retry_run(_cap, name="eco_captureImages(ARM)")

        output_file = os.path.join(output_dir, "step4_image_query_arm.json")
        write_json(output_file, {"task_id": task_id, "camera_type": 1, "status": "success"})
        logger.info(f"手臂相机拍照成功，输出已保存到: {output_file}")

        return True, resp

    except Exception as e:
        logger.exception("手臂相机拍照执行异常: %s", e)
        return False, None


# ============================================================
# 高层组合函数
# ============================================================

def run_step5_with_intent(
    robot: BajieRobot,
    head_view: RGBDViewWithPose,
    intent_steps: List[Dict[str, Any]],
    output_dir: str = "outputs",
) -> Tuple[bool, List[Dict[str, Any]] | None, Dict[str, int] | None]:
    """根据意图生成结果构建bbox并计算6D姿态"""
    bboxes = []
    bbox_id = 1
    step_id_to_bbox_id_map = {}

    for step_item in intent_steps:
        step_id = step_item.get("step_id")
        from_bbox = step_item.get("from_bbox")
        to_bbox = step_item.get("to_bbox")
        if from_bbox:
            bboxes.append({
                "id": bbox_id,
                "left_up_x": from_bbox[0],
                "left_up_y": from_bbox[1],
                "right_down_x": from_bbox[2],
                "right_down_y": from_bbox[3],
            })
            step_id_to_bbox_id_map[f"{step_id}_from"] = bbox_id
            bbox_id += 1
        if to_bbox:
            bboxes.append({
                "id": bbox_id,
                "left_up_x": to_bbox[0],
                "left_up_y": to_bbox[1],
                "right_down_x": to_bbox[2],
                "right_down_y": to_bbox[3],
            })
            step_id_to_bbox_id_map[f"{step_id}_to"] = bbox_id
            bbox_id += 1

    logger.info(f"bbox_id映射: {step_id_to_bbox_id_map}")

    map_file = os.path.join(output_dir, "step5_bbox_id_map.json")
    write_json(map_file, step_id_to_bbox_id_map)

    success, result = get_pose(robot, bboxes, head_view, output_dir)
    if not success or not result:
        return False, None, None

    pose_results = result.get("pose_results", [])
    logger.info(f"获取到 {len(pose_results)} 个姿态结果")

    return True, pose_results, step_id_to_bbox_id_map


def process_item(
    robot: BajieRobot,
    step_item: Dict[str, Any],
    head_view: RGBDViewWithPose,
    pose_results: List[Dict[str, Any]],
    bbox_id_map: Dict[str, int],
    output_dir: str = OUTPUT_DIR,
    stop_before_grab: bool = False,
    is_container: bool = False
) -> bool:
    """处理单个物品 (自适应姿态 -> 手臂拍照 -> bbox获取 -> 抓取 -> 放置)"""
    step_id = step_item.get("step_id")
    reason = step_item.get("reason", "")
    from_bbox = step_item.get("from_bbox")

    logger.info(f"=== 处理物品 step_id={step_id}, reason={reason} ===")

    from_pose = None
    to_pose = None
    from_bbox_id = bbox_id_map.get(f"{step_id}_from")
    to_bbox_id = bbox_id_map.get(f"{step_id}_to")

    for pose in pose_results:
        pose_id = pose.get("id")
        if pose_id == from_bbox_id:
            from_pose = pose
        elif pose_id == to_bbox_id:
            to_pose = pose

    if not from_pose:
        logger.error("%s 未获取到step_id=%s的from_pose (bbox_id=%s)", error_tag(ERR_FROM_POSE_MISSING), step_id, from_bbox_id)
        return False

    if not to_pose:
        logger.warning(f"未获取到step_id={step_id}的to_pose (bbox_id={to_bbox_id})，使用from_pose代替")
        to_pose = from_pose

    # 1. 位置自适应
    logger.info("--- 位置自适应 ---")
    success = run_pose_adaptive_from_pose(robot, from_pose, output_dir)
    if not success:
        logger.warning("位置自适应失败，继续执行后续任务")

    # 2. 手臂相机拍照
    logger.info("--- 手臂相机拍照 ---")
    success, arm_view = arm_image_query(robot, output_dir)
    if not success or not arm_view:
        logger.error("%s 手臂相机拍照失败", error_tag(ERR_IMAGE_QUERY_FAILED))
        return False

    # 3. 获取物品bbox
    logger.info("--- 获取物品bbox ---")
    success, item_bbox = run_item_bbox_match(robot, arm_view, head_view, from_bbox, output_dir)
    if not success or not item_bbox:
        logger.error("%s 获取物品bbox失败", error_tag(ERR_BBOX_MATCH_FAILED))
        return False

    if stop_before_grab:
        logger.info("已按配置在抓取前停止（仅生成到 bbox 输出）")
        return True

    # 4. 物品抓取
    logger.info("--- 物品抓取 ---")
    speak(robot, SPEECH_GRABBING)
    item_name = (reason or "").strip() or f"item_{step_id}"
    success = run_accurate_grab(robot, arm_view, item_bbox, item_name, output_dir)
    if not success:
        logger.error("%s 物品抓取失败", error_tag(ERR_GRAB_FAILED))
        return False

    # 5. 放置
    place_success = False
    speak(robot, SPEECH_PLACING)
    if is_container:
        logger.info("--- 容器放置 ---")
        try:
            to_bbox = step_item.get("to_bbox")
            if not to_bbox or len(to_bbox) < 4:
                raise ValueError("未获取到有效的to_bbox")
            def _pin():
                st = robot.eco_place_in(head_view, to_bbox[:4], timeout_sec=300.0)
                fail = mission_failed(st)
                if fail:
                    raise RuntimeError(fail)
                return st

            retry_run(_pin, name="eco_place_in")

            output_file = os.path.join(output_dir, f"step8_container_place_{step_id}.json")
            write_json(output_file, {
                "task_id": new_task_id(),
                "step_id": step_id,
                "to_bbox": to_bbox[:4],
                "status": "success",
            })
            logger.info(f"容器放置完成，输出已保存到: {output_file}")
            place_success = True
        except Exception as e:
            logger.warning("%s 容器放置失败: %s，尝试使用from_pose进行精准放置作为回退", error_tag(ERR_PLACE_IN_FAILED), e)
    else:
        logger.info("--- 精准放置 ---")
        success = run_accurate_place(robot, to_pose, output_dir)
        if success:
            place_success = True
        else:
            logger.warning("%s 精准放置失败，尝试使用from_pose进行精准放置作为回退", error_tag(ERR_PLACE_FAILED))

    if not place_success:
        success = run_accurate_place(robot, from_pose, output_dir)
        if success:
            logger.info(f"使用from_pose回退放置成功")
        else:
            logger.error("%s 回退放置也失败", error_tag(ERR_FALLBACK_PLACE_FAILED))
            return False

    logger.info(f"物品 {step_id} 处理完成")
    return True


def run_execution_workflow(
    robot: BajieRobot,
    head_view: RGBDViewWithPose,
    intent_steps: List[Dict[str, Any]],
    output_dir: str = OUTPUT_DIR,
    stop_before_grab: bool = False,
) -> bool:
    """运行执行工作流"""
    logger.info("=" * 50)
    logger.info("开始执行工作流 (Execution Workflow)")
    logger.info("=" * 50)

    # 计算6D姿态
    logger.info("--- 计算6D姿态 ---")
    success, pose_results, bbox_id_map = run_step5_with_intent(
        robot, head_view, intent_steps, output_dir,
    )
    if not success or not pose_results:
        logger.error("%s 6D姿态计算失败", error_tag(ERR_POSE_FAILED))
        return False

    logger.info(f"获取到 {len(pose_results)} 个姿态结果")

    # 循环处理每个物品
    logger.info("--- 循环处理每个物品 ---")
    for i, step_item in enumerate(intent_steps):
        is_container=step_item.get('is_container')
        logger.info(f"处理物品 {i+1}/{len(intent_steps)}")
        success = process_item(
            robot,
            step_item,
            head_view,
            pose_results,
            bbox_id_map,
            output_dir,
            stop_before_grab=stop_before_grab,
            is_container=is_container
        )
        if not success:
            logger.warning(f"物品 {i+1} 处理失败，继续下一个")
        if stop_before_grab:
            logger.info("已按配置 stop_before_grab=true，结束后续物品处理")
            return True

    logger.info("=" * 50)
    logger.info("执行工作流完成")
    logger.info("=" * 50)
    return True
