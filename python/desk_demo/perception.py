#!/usr/bin/env python3
"""
感知工作流 (Perception Workflow)

组合语义导航、机身高度控制、（可选）SDK ``eco_vlm_suggest_angle`` 头部选角、
头部相机拍照、``eco_vlm_perception`` 物体感知，完成桌面感知流程。

说明：model_service 的 HTTP ``pre_perception/v1`` 与 SDK 中的 ``eco_vlm_suggest_angle``
业务相近；本目录优先走 SDK（多角度采样 → VLM 推荐弧度 → ``eco_setRobotHead``）。

工作流程:
1. 语义导航到桌子
2. 升高机身到观察高度
3. （头部相机）多角度采样并 ``eco_vlm_suggest_angle``，再 ``eco_setRobotHead`` 到推荐角
4. 头部相机拍照（正式 step4）
5. 调用 ``eco_vlm_perception`` 感知物体
"""

import asyncio
import logging
import os
import time
from typing import Any, Dict, List, Optional, Sequence, Tuple

from bajie_sdk import BajieRobot, RGBDViewWithPose, SemanticNavigationRequest
from bajie_sdk.utils import image_payload_to_dict, pose_frame_to_goal_dict

from errors import RobotMissionError
from ref_utils import configure_logging, ensure_dir, new_task_id, rgb_image_dict_to_rgb_ndarray, write_json
from retry_utils import (
    ERR_HEAD_ANGLE_FAILED,
    ERR_HEAD_CTRL_FAILED,
    ERR_HEIGHT_CTRL_FAILED,
    ERR_IMAGE_QUERY_FAILED,
    ERR_MODEL_AUTH,
    ERR_PERCEPTION_FAILED,
    ERR_SEMANTIC_NAV_FAILED,
    classify_error_code,
    error_tag,
    mission_failed,
    retry_run,
)
from speech import (
    SPEECH_HEAD_ANGLE,
    SPEECH_NAVIGATING,
    SPEECH_PERCEIVING,
    SPEECH_RAISING,
    speak,
)

configure_logging()
logger = logging.getLogger(__name__)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "outputs")

PERCEPTION_SDK_TIMEOUT_SEC = 150.0

# 与 DeskConfig.head_sweep_angles_rad 默认一致；供函数签名默认值使用
DEFAULT_HEAD_SWEEP_ANGLES_RAD: Tuple[float, ...] = (0.15, 0.2, 0.25, 0.3, 0.35, 0.4)


def set_robot_head_angle(
    robot: BajieRobot,
    value_rad: float,
    *,
    timeout_sec: float = 120.0,
) -> bool:
    """设置头部俯仰（弧度）；SDK 内部会将超出 [0, 1.3] 的值截断。"""
    try:
        def _head():
            st = robot.eco_setRobotHead(float(value_rad), timeout_sec=float(timeout_sec))
            fail = mission_failed(st)
            if fail:
                raise RuntimeError(fail)

        retry_run(_head, name="eco_setRobotHead")
        return True
    except Exception as exc:
        logger.warning("%s eco_setRobotHead(%s) 失败: %s", error_tag(ERR_HEAD_CTRL_FAILED), value_rad, exc)
        return False


def collect_image_angle_pairs_for_suggest(
    robot: BajieRobot,
    head_angles_rad: Sequence[float],
    *,
    head_timeout_sec: float = 120.0,
    capture_timeout_sec: float = 120.0,
    sleep_sec: float = 0.0,
) -> Tuple[List[Tuple[Any, float]], Optional[float]]:
    """在每个采样角拍照，得到 ``eco_vlm_suggest_angle`` 所需的 ``[(rgb_ndarray, rad), ...]``，并返回最后一次成功设置的头部角度。"""
    pairs: List[Tuple[Any, float]] = []
    last_set_rad: Optional[float] = None
    for idx, rad in enumerate(head_angles_rad):
        rad_f = float(rad)
        logger.info(
            "头部选角采样 %s/%s: set_head=%.4f rad -> capture",
            idx + 1,
            len(head_angles_rad),
            rad_f,
        )
        if not set_robot_head_angle(robot, rad_f, timeout_sec=head_timeout_sec):
            logger.warning("采样角 %.4f：设置头部失败，跳过", rad_f)
            continue
        last_set_rad = rad_f
        if sleep_sec > 0:
            time.sleep(float(sleep_sec))
        try:
            def _cap():
                return robot.eco_captureImages(2, timeout_sec=float(capture_timeout_sec))

            resp = retry_run(_cap, name=f"eco_captureImages(head_sample rad={rad_f:.4f})")
            pairs.append((resp.rgb_image.img, rad_f))
        except Exception as exc:
            logger.warning("采样角 %.4f：拍照失败 %s", rad_f, exc)
    return pairs, last_set_rad


def run_head_angle_suggest_pipeline(
    robot: BajieRobot,
    head_angles_rad: Sequence[float],
    *,
    output_dir: str,
    suggest_timeout_sec: float = 150.0,
    head_timeout_sec: float = 120.0,
    capture_timeout_sec: float = 120.0,
    sleep_sec: float = 0.0,
) -> Tuple[bool, Optional[float]]:
    """多角度采样 → ``eco_vlm_suggest_angle`` → ``eco_setRobotHead`` 应用推荐角；失败时回退到确定角度。"""
    angles_list = [float(x) for x in head_angles_rad]
    pairs, last_set_rad = collect_image_angle_pairs_for_suggest(
        robot,
        angles_list,
        head_timeout_sec=head_timeout_sec,
        capture_timeout_sec=capture_timeout_sec,
        sleep_sec=sleep_sec,
    )
    rec_path = os.path.join(output_dir, "step_head_angle_vlm_suggest.json")
    fallback_rad = last_set_rad if last_set_rad is not None else (angles_list[-1] if angles_list else 0.4)

    def _apply_fallback(reason: str, error: Optional[str] = None) -> Tuple[bool, Optional[float]]:
        fallback_applied = set_robot_head_angle(robot, float(fallback_rad), timeout_sec=head_timeout_sec)
        if fallback_applied and sleep_sec > 0:
            time.sleep(float(sleep_sec))
        logger.warning("%s 头部选角未完成，回退到 %.4f rad（原因: %s）", error_tag(ERR_HEAD_ANGLE_FAILED), float(fallback_rad), reason)
        payload = {
            "status": "fallback",
            "reason": reason,
            "attempted_rad": angles_list,
            "sample_count": len(pairs),
            "fallback_rad": float(fallback_rad),
            "fallback_applied": bool(fallback_applied),
        }
        if error is not None:
            payload["error"] = error
        write_json(rec_path, payload)
        return False, float(fallback_rad)

    if len(pairs) < 2:
        logger.warning("有效采样不足 %s 组（需要≥2），跳过 eco_vlm_suggest_angle", len(pairs))
        return _apply_fallback("not_enough_samples")

    speak(robot, SPEECH_HEAD_ANGLE)
    try:
        suggested = float(
            retry_run(
                lambda: robot.eco_vlm_suggest_angle(pairs, timeout=float(suggest_timeout_sec)),
                name="eco_vlm_suggest_angle",
            )
        )
    except Exception as exc:
        logger.exception("%s eco_vlm_suggest_angle 失败: %s", error_tag(ERR_HEAD_ANGLE_FAILED), exc)
        return _apply_fallback("suggest_failed", str(exc))

    applied = max(0.0, min(1.3, suggested))
    if abs(applied - suggested) > 1e-6:
        logger.info("推荐角 %.6f 已钳制到 SDK 范围 → %.6f", suggested, applied)

    if not set_robot_head_angle(robot, applied, timeout_sec=head_timeout_sec):
        logger.warning("应用推荐头部角度 %.4f 失败", applied)
        return _apply_fallback("apply_suggested_failed")

    write_json(
        rec_path,
        {
            "status": "success",
            "attempted_rad": angles_list,
            "sample_count": len(pairs),
            "suggested_rad_raw": suggested,
            "applied_rad": applied,
        },
    )

    if sleep_sec > 0:
        time.sleep(float(sleep_sec))

    return True, applied


def _named_bbox_items_to_perception_dicts(items: List[Any]) -> List[Dict[str, Any]]:
    """将 ``eco_vlm_perception`` 返回的 ``NamedBBox`` 列表转为与旧 HTTP 一致的 ``{name,bbox}`` 列表（附带 ``id``）。"""
    out: List[Dict[str, Any]] = []
    for nb in items:
        bbox = getattr(nb, "bbox", None)
        if bbox is None or len(bbox) != 4:
            continue
        out.append(
            {
                "id": int(getattr(nb, "id", 0)),
                "name": str(getattr(nb, "name", "") or ""),
                "bbox": [int(bbox[0]), int(bbox[1]), int(bbox[2]), int(bbox[3])],
            }
        )
    return out


def semantic_navigation(
    robot: BajieRobot,
    area_name: str = "书桌",
    area_id: str = "",
    output_dir: str = OUTPUT_DIR,
) -> Tuple[bool, Optional[Dict[str, Any]]]:
    """语义导航到目标区域"""
    logger.info("=" * 50)
    logger.info("语义导航")
    logger.info(f"  目标区域: {area_name}")
    logger.info("=" * 50)

    task_id = new_task_id()
    try:
        speak(robot, SPEECH_NAVIGATING)
        def _nav():
            status = robot.eco_navigateToSemanticArea(
                SemanticNavigationRequest(
                    area_id=str(area_id or ""),
                    area_name=str(area_name or ""),
                ),
                timeout_sec=180.0,
            )
            fail = mission_failed(status)
            if fail:
                raise RuntimeError(fail)
            return status

        status = retry_run(_nav, name="eco_navigateToSemanticArea")

        output_file = f"{output_dir}/step2_navigate.json"
        result = {
            "task_id": task_id,
            "area_name": area_name,
            "area_id": area_id,
            "status": "success",
            "sdk_result": {
                "task_id": status.task_id,
                # "request_uuid": status.request_uuid,
                "error_code": getattr(status.error_info, "code", 0),
                "error_msg": getattr(status.error_info, "message", ""),
                "version_info": getattr(status, "version_info", ""),
            },
        }
        write_json(output_file, result)
        logger.info(f"语义导航成功，输出已保存到: {output_file}")
        return True, result

    except Exception as e:
        logger.exception("语义导航执行异常: %s", e)
        return False, None


def robot_height_ctrl(
    robot: BajieRobot,
    mode: int = 1,
    output_dir: str = OUTPUT_DIR,
) -> Tuple[bool, Optional[Dict[str, Any]]]:
    """机身高度控制"""
    logger.info("=" * 50)
    logger.info("机身高度控制")
    logger.info(f"  模式: {mode} ({'升高' if mode == 1 else '降低'})")
    logger.info("=" * 50)

    task_id = new_task_id()
    try:
        if int(mode) == 1:
            speak(robot, SPEECH_RAISING)
        value_m = 0.44 if int(mode) == 1 else 0.0

        def _h():
            st = robot.eco_setRobotHeight(float(value_m), timeout_sec=120.0)
            fail = mission_failed(st)
            if fail:
                raise RuntimeError(fail)
            return st

        status = retry_run(_h, name="eco_setRobotHeight")

        output_file = f"{output_dir}/step3_height_ctrl.json"
        result = {
            "task_id": task_id,
            "mode": mode,
            "value_m": value_m,
            "status": "success",
            "sdk_result": {
                "task_id": status.task_id,
                # "request_uuid": status.request_uuid,
                "error_code": getattr(status.error_info, "code", 0),
                "error_msg": getattr(status.error_info, "message", ""),
                "version_info": getattr(status, "version_info", ""),
            },
        }
        write_json(output_file, result)
        logger.info(f"机身高度控制成功 (目标高度: {value_m}m)")
        return True, result

    except Exception as e:
        logger.exception("机身高度控制执行异常: %s", e)
        return False, None


def image_query(
    robot: BajieRobot,
    camera_type: int = 2,
    output_dir: str = OUTPUT_DIR,
) -> Tuple[bool, Optional[Dict[str, Any]]]:
    """相机拍照

    Returns:
        (success, result_dict) 包含 rgb_image, depth_image, tf_goal, camera_info_k
    """
    camera_label = "头部相机" if camera_type == 2 else "手臂相机"
    logger.info("=" * 50)
    logger.info("相机拍照")
    logger.info(f"  相机类型: {camera_label}")
    logger.info("=" * 50)

    task_id = new_task_id()
    try:
        def _cap():
            resp = robot.eco_captureImages(int(camera_type), timeout_sec=120.0)
            fail = mission_failed(resp)
            if fail:
                raise RuntimeError(fail)
            return resp

        resp = retry_run(
            _cap,
            name=f"eco_captureImages(camera_type={int(camera_type)})",
        )
        # SDK 返回强类型对象，转换为 dict
        rgb_image = image_payload_to_dict(resp.rgb_image)
        depth_image = image_payload_to_dict(resp.depth_image)
        tf_goal = pose_frame_to_goal_dict(resp.tf_goal)
        # camera_info_k 多为 3×3 内参矩阵（numpy ndarray），需 tolist 后再写入 JSON
        _k = resp.camera_info_k
        if _k is None:
            camera_info_k: List[List[float]] = []
        elif hasattr(_k, "tolist"):
            camera_info_k = _k.tolist()
        else:
            camera_info_k = list(_k) if _k else []

        filename = "step4_image_query_head.json" if int(camera_type) == 2 else "step4_image_query_arm.json"
        output_file = f"{output_dir}/{filename}"

        result = {
            "task_id": task_id,
            "camera_type": int(camera_type),
            "status": "success",
            "sdk_result": {
                "task_id": resp.task_id,
                # "request_uuid": resp.request_uuid,
                "error_code": getattr(resp.error_info, "code", 0),
                "error_msg": getattr(resp.error_info, "message", ""),
                "version_info": getattr(resp, "version_info", ""),
            },
            "rgb_image_meta": {"width": rgb_image.get("width"), "height": rgb_image.get("height")},
            "depth_image_meta": {"width": depth_image.get("width"), "height": depth_image.get("height")},
            "rgb_image": rgb_image,
            "depth_image": depth_image,
            "tf_goal": tf_goal,
            "camera_info_k": camera_info_k,
        }

        write_json(output_file, result)
        logger.info(f"相机拍照成功，输出已保存到: {output_file}")
        return True, result

    except Exception as e:
        logger.exception("相机拍照执行异常: %s", e)
        return False, None


async def run_perception_workflow(
    robot: BajieRobot,
    area_name: str = "书桌",
    area_id: str = "",
    camera_type: int = 2,
    img_w: int = 640,
    img_h: int = 400,
    output_dir: str = OUTPUT_DIR,
    *,
    enable_vlm_head_angle_suggest: bool = True,
    head_sweep_angles_rad: Sequence[float] = DEFAULT_HEAD_SWEEP_ANGLES_RAD,
    head_sweep_sleep_sec: float = 0.3,
    vlm_suggest_angle_timeout_sec: float = 150.0,
    reuse_head_angle_rad: Optional[float] = None,
) -> Tuple[bool, Optional[Dict[str, Any]], Optional[RGBDViewWithPose]]:
    """运行感知工作流

    按顺序执行:
    1. 语义导航到目标区域
    2. 升高机身到观察高度
    3. （头部相机且启用）多角度采样 ``eco_vlm_suggest_angle`` 并 ``eco_setRobotHead``
    4. 头部相机拍照（正式 step4）
    5. 调用 ``eco_vlm_perception`` 感知物体

    Args:
        robot: 已连接的 BajieRobot 实例
        area_name: 目标区域名称
        area_id: 目标区域ID
        camera_type: 相机类型（默认2=头部相机）
        img_w: 图片宽度（元数据用途）
        img_h: 图片高度（元数据用途）
        output_dir: 输出目录
        enable_vlm_head_angle_suggest: 是否在头部拍照前做 SDK 选角（仅 camera_type==2）
        head_sweep_angles_rad: 选角采样俯仰角列表（弧度）
        head_sweep_sleep_sec: 每次转头/拍照后的短暂等待（秒）
        vlm_suggest_angle_timeout_sec: ``eco_vlm_suggest_angle`` 超时

    Returns:
        (success, image_dict, raw_view) 其中 image_dict 为 dict 格式图像数据
        raw_view 为 SDK 原始 RGBDViewWithPose 对象（用于后续执行阶段）
    """
    ensure_dir(output_dir)

    logger.info("=" * 60)
    logger.info("开始执行感知工作流 (Perception Workflow)")
    logger.info("=" * 60)
    logger.info(f"  目标区域: {area_name}")
    logger.info(f"  相机类型: {'头部相机' if camera_type == 2 else '手臂相机'}")
    logger.info(f"  图片尺寸: {img_w}x{img_h}")
    logger.info(f"  输出目录: {output_dir}")
    logger.info(f"  头部选角(eco_vlm_suggest_angle): {'开启' if int(camera_type) == 2 and enable_vlm_head_angle_suggest else '关闭'}")
    logger.info("=" * 60)

    # 语义导航
    success, nav_result = semantic_navigation(robot, area_name, area_id, output_dir)
    if not success:
        logger.error("%s 语义导航失败，工作流终止", error_tag(ERR_SEMANTIC_NAV_FAILED))
        return False, None, None

    # 升高机身
    success, height_result = robot_height_ctrl(robot, mode=1, output_dir=output_dir)
    if not success:
        logger.error("%s 机身高度控制失败，工作流终止", error_tag(ERR_HEIGHT_CTRL_FAILED))
        return False, None, None

    applied_head_angle_rad: Optional[float] = None

    # （头部）优先复用首次确定的角度；否则首次做 SDK 选角
    if int(camera_type) == 2 and reuse_head_angle_rad is not None:
        applied_head_angle_rad = float(reuse_head_angle_rad)
        logger.info("=" * 50)
        logger.info("复用已缓存头部角度: %.4f rad", applied_head_angle_rad)
        logger.info("=" * 50)
        reused_ok = set_robot_head_angle(robot, applied_head_angle_rad, timeout_sec=120.0)
        if reused_ok and head_sweep_sleep_sec > 0:
            time.sleep(float(head_sweep_sleep_sec))
        if not reused_ok:
            logger.warning("复用头部角度 %.4f 失败，使用当前头部姿态继续拍照", applied_head_angle_rad)
        write_json(
            os.path.join(output_dir, "step_head_angle_vlm_suggest.json"),
            {
                "status": "reused",
                "applied_rad": applied_head_angle_rad,
                "applied_ok": bool(reused_ok),
            },
        )
    elif int(camera_type) == 2 and enable_vlm_head_angle_suggest:
        logger.info("=" * 50)
        logger.info(
            "头部选角 (eco_vlm_suggest_angle)，采样角 rad=%s",
            [float(x) for x in head_sweep_angles_rad],
        )
        logger.info("=" * 50)
        ok_ang, chosen_rad = run_head_angle_suggest_pipeline(
            robot,
            head_sweep_angles_rad,
            output_dir=output_dir,
            suggest_timeout_sec=float(vlm_suggest_angle_timeout_sec),
            sleep_sec=float(head_sweep_sleep_sec),
        )
        if chosen_rad is not None:
            applied_head_angle_rad = float(chosen_rad)
        if ok_ang and chosen_rad is not None:
            logger.info("已应用推荐头部角度: %.4f rad", float(chosen_rad))
        elif chosen_rad is not None:
            logger.warning("头部选角未完成，已回退并应用头部角度: %.4f rad", float(chosen_rad))
        else:
            logger.warning("头部选角未完成，使用当前头部姿态继续拍照")

    # 头部相机拍照（正式）
    success, image_result = image_query(robot, camera_type=camera_type, output_dir=output_dir)
    if not success or not image_result:
        logger.error("%s 相机拍照失败，工作流终止", error_tag(ERR_IMAGE_QUERY_FAILED))
        return False, None, None

    # 保存原始 SDK 返回对象，用于后续执行阶段
    raw_view = None
    try:
        def _raw():
            return robot.eco_captureImages(int(camera_type), timeout_sec=120.0)

        raw_view = retry_run(
            _raw,
            name=f"eco_captureImages(raw_view cam={int(camera_type)})",
        )
    except Exception as e:
        logger.warning(f"保存原始 RGBDViewWithPose 失败: {e}")

    logger.info("=" * 50)
    logger.info("调用 eco_vlm_perception 进行物体感知")
    logger.info("=" * 50)
    speak(robot, SPEECH_PERCEIVING)
    try:
        rgb_arr = rgb_image_dict_to_rgb_ndarray(image_result.get("rgb_image", {}))
        if rgb_arr is None:
            logger.error("%s rgb_image 无法转为 ndarray，感知失败", error_tag(ERR_PERCEPTION_FAILED))
            return False, None, raw_view

        items = retry_run(
            lambda: robot.eco_vlm_perception(rgb_arr, timeout=PERCEPTION_SDK_TIMEOUT_SEC),
            name="eco_vlm_perception",
        )
        perception_data = _named_bbox_items_to_perception_dicts(items)
        image_result["perception"] = perception_data
        perception_file = f"{output_dir}/step4_perception.json"
        write_json(perception_file, {"perception": perception_data})
        logger.info("感知完成，共识别 %s 个物体", len(perception_data))
        logger.info("感知结果已保存到: %s", perception_file)
    except Exception as e:
        code = classify_error_code("eco_vlm_perception", e)
        if code == ERR_MODEL_AUTH:
            logger.exception("%s 物体感知执行异常: %s", error_tag(code), e)
            raise RobotMissionError(f"{error_tag(code)} 物体感知执行异常: {e}") from e
        logger.exception("%s 物体感知执行异常: %s", error_tag(ERR_PERCEPTION_FAILED), e)
        return False, None, raw_view

    image_result["head_angle_applied_rad"] = applied_head_angle_rad

    # 汇总结果
    summary = {
        "workflow": "perception_workflow",
        "status": "success",
        "area_name": area_name,
        "area_id": area_id,
        "camera_type": camera_type,
        "outputs": {
            "navigation": f"{output_dir}/step2_navigate.json",
            "height_ctrl": f"{output_dir}/step3_height_ctrl.json",
            **(
                {"head_angle_vlm_suggest": f"{output_dir}/step_head_angle_vlm_suggest.json"}
                if int(camera_type) == 2
                else {}
            ),
            "image_query": f"{output_dir}/step4_image_query_{'head' if camera_type == 2 else 'arm'}.json",
            "perception": f"{output_dir}/step4_perception.json",
        },
    }

    summary_file = f"{output_dir}/perception_workflow_summary.json"
    write_json(summary_file, summary)

    logger.info("=" * 60)
    logger.info("感知工作流执行完毕")
    logger.info(f"  汇总文件: {summary_file}")
    logger.info("=" * 60)

    return True, image_result, raw_view


async def main():
    """主函数（独立运行时使用）"""
    import argparse

    parser = argparse.ArgumentParser(description="感知工作流 - 导航到桌子并拍照")
    parser.add_argument("--area_name", default="书桌", help="目标区域名称")
    parser.add_argument("--area_id", default="", help="目标区域ID")
    parser.add_argument("--camera_type", type=int, default=2, choices=[1, 2])
    parser.add_argument("--output_dir", default=OUTPUT_DIR)
    parser.add_argument(
        "--no-head-angle-suggest",
        action="store_true",
        help="跳过 eco_vlm_suggest_angle",
    )
    parser.add_argument(
        "--head-sweep-rad",
        default="",
        help="逗号分隔采样弧度（≥2）；默认内置六档",
    )

    args = parser.parse_args()

    head_sweep: Optional[Tuple[float, ...]] = None
    raw_sw = (args.head_sweep_rad or "").strip()
    if raw_sw:
        parts = [p.strip() for p in raw_sw.split(",") if p.strip()]
        try:
            head_sweep = tuple(float(p) for p in parts)
        except ValueError as exc:
            raise SystemExit(f"invalid --head-sweep-rad: {exc}") from exc
        if len(head_sweep) < 2:
            raise SystemExit("--head-sweep-rad 至少需要 2 个弧度值")

    robot = BajieRobot()
    def _connect() -> None:
        if not robot.Connect(timeout_sec=60.0):
            raise RuntimeError(robot.LastWebSocketError() or "connect returned false")

    retry_run(_connect, name="websocket_connect")

    try:
        success, _, _ = await run_perception_workflow(
            robot,
            area_name=args.area_name,
            area_id=args.area_id,
            camera_type=args.camera_type,
            output_dir=args.output_dir,
            enable_vlm_head_angle_suggest=(not args.no_head_angle_suggest),
            head_sweep_angles_rad=head_sweep if head_sweep is not None else DEFAULT_HEAD_SWEEP_ANGLES_RAD,
        )
    finally:
        robot.Disconnect()

    if success:
        print("\n感知工作流执行成功！")
    else:
        print("\n感知工作流执行失败，请查看日志")


if __name__ == "__main__":
    asyncio.run(main())
