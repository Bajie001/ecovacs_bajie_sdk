"""统一重试：整条 demo 对 SDK / WebSocket 抖动复用同一入口。"""

from __future__ import annotations

import logging
import time
from typing import Any, Callable, Optional, TypeVar

logger = logging.getLogger(__name__)

T = TypeVar("T")

ERR_CONNECT_FAILED = 300101
ERR_ROBOT_INFO_FAILED = 300102
ERR_RELOCATE_FAILED = 300103
ERR_PREPARE_POSE_FAILED = 300104
ERR_FINISH_POSE_FAILED = 300105
ERR_ARM_HOME_FAILED = 300106
ERR_SEMANTIC_NAV_FAILED = 300201
ERR_HEIGHT_CTRL_FAILED = 300202
ERR_HEAD_CTRL_FAILED = 300203
ERR_HEAD_ANGLE_FAILED = 300204
ERR_IMAGE_QUERY_FAILED = 300205
ERR_PERCEPTION_FAILED = 300206
ERR_MODEL_TIMEOUT = 300301
ERR_MODEL_GATEWAY = 300303
ERR_MODEL_AUTH = 300304
ERR_PLAN_FAILED = 300401
ERR_POSE_FAILED = 300403
ERR_FROM_POSE_MISSING = 300404
ERR_TO_POSE_MISSING = 300405
ERR_BBOX_MATCH_FAILED = 300406
ERR_EXECUTION_FAILED = 300501
ERR_GRAB_FAILED = 300502
ERR_PLACE_FAILED = 300503
ERR_PLACE_IN_FAILED = 300504
ERR_FALLBACK_PLACE_FAILED = 300505
ERR_JUDGE_FAILED = 300601


def error_hex(code: int) -> str:
    return f"0x{int(code):08X}"


def error_tag(code: int) -> str:
    return f"error_code={int(code)} error_hex={error_hex(int(code))}"


def classify_error_code(name: str, detail: object = "") -> int:
    s = f"{name} {detail}".lower()
    if any(
        x in s
        for x in (
            "401",
            "403",
            "api_key",
            "api key",
            "api-key",
            "apikey",
            "invalid token",
            "token invalid",
            "unauthorized",
            "forbidden",
            "鉴权",
            "认证",
            "令牌无效",
        )
    ):
        return ERR_MODEL_AUTH
    if any(x in s for x in ("502", "503", "504", "gateway", "service unavailable")):
        return ERR_MODEL_GATEWAY
    if any(x in s for x in ("timed out", "timeout", "超时")):
        return ERR_MODEL_TIMEOUT
    if "websocket" in s or "connect" in s:
        return ERR_CONNECT_FAILED
    if "relocate" in s or "map_relocation" in s:
        return ERR_RELOCATE_FAILED
    if "preparerobotpose" in s or "preparepose" in s:
        return ERR_PREPARE_POSE_FAILED
    if "finishrobotpose" in s or "ending_pose" in s:
        return ERR_FINISH_POSE_FAILED
    if "robotarmctrl" in s or "arm_home" in s:
        return ERR_ARM_HOME_FAILED
    if "navigatetosemanticarea" in s or "semanticnavigation" in s:
        return ERR_SEMANTIC_NAV_FAILED
    if "setrobotheight" in s or "height" in s:
        return ERR_HEIGHT_CTRL_FAILED
    if "setrobothead" in s or "head_ctrl" in s:
        return ERR_HEAD_CTRL_FAILED
    if "suggest_angle" in s or "preperception" in s:
        return ERR_HEAD_ANGLE_FAILED
    if "imagequery" in s or "captureimages" in s:
        return ERR_IMAGE_QUERY_FAILED
    if "vlm_perception" in s or "deskintentperception" in s:
        return ERR_PERCEPTION_FAILED
    if "desk_sort_plan" in s or "deskintentplan" in s or "plan" in s:
        return ERR_PLAN_FAILED
    if "computeobjectpose" in s or "getpose" in s:
        return ERR_POSE_FAILED
    if "vlm_match" in s or "deskintentmatch" in s or "match" in s:
        return ERR_BBOX_MATCH_FAILED
    if "pick" in s or "grab" in s:
        return ERR_GRAB_FAILED
    if "place_in" in s or "placein" in s:
        return ERR_PLACE_IN_FAILED
    if "place" in s:
        return ERR_PLACE_FAILED
    if "judge" in s:
        return ERR_JUDGE_FAILED
    return ERR_MODEL_TIMEOUT if "http" in s else ERR_PERCEPTION_FAILED


DEFAULT_MAX_ATTEMPTS = 3
DEFAULT_DELAY_SEC = 0.35


def mission_failed(status: Any) -> Optional[str]:
    """SDK 任务状态非 0 时返回错误描述，否则 None。"""
    ei = getattr(status, "error_info", None)
    code = int(getattr(ei, "code", 0) or 0)
    if code == 0:
        return None
    msg = getattr(ei, "message", "") or ""
    return f"code={code} msg={msg}"


def retry_run(
    op: Callable[[], T],
    *,
    name: str,
    max_attempts: int = DEFAULT_MAX_ATTEMPTS,
    delay_sec: float = DEFAULT_DELAY_SEC,
) -> T:
    """
    对 ``op()`` 最多执行 ``max_attempts`` 次；异常则退避后重试。

    日志：失败时 ``失败 [当前/最大]``，多次尝试后成功时 ``成功（第 k/最大 次尝试）``。
    """
    for attempt in range(1, max_attempts + 1):
        try:
            out = op()
            if attempt > 1:
                logger.info("%s 成功（第 %d/%d 次尝试）", name, attempt, max_attempts)
            return out
        except Exception as e:
            if attempt >= max_attempts:
                logger.error("%s %s 仍失败 [%d/%d]: %s", error_tag(classify_error_code(name, e)), name, attempt, max_attempts, e)
                raise
            logger.warning("%s %s 失败 [%d/%d] 将重试: %s", error_tag(classify_error_code(name, e)), name, attempt, max_attempts, e)
            time.sleep(delay_sec)
    raise RuntimeError(f"{name}: unreachable")  # pragma: no cover
