#!/usr/bin/env python3
"""固定文案语音播报（eco_robotSpeech 非阻塞），失败不中断主流程。"""

from __future__ import annotations

import logging
from typing import Any

from retry_utils import mission_failed

logger = logging.getLogger(__name__)

# 各步骤开始播报（固定文案）
SPEECH_RELOCATING = "正在重定位"
SPEECH_PREPARE_POSE = "正在准备姿态"
SPEECH_NAVIGATING = "正在导航"
SPEECH_RAISING = "正在升高"
SPEECH_HEAD_ANGLE = "正在调整头部角度"
SPEECH_PERCEIVING = "正在识别物品"
SPEECH_PLANNING = "正在规划"
SPEECH_EXECUTION = "开始整理"
SPEECH_GRABBING = "正在抓取"
SPEECH_PLACING = "正在放置"
SPEECH_JUDGING = "正在评判"
SPEECH_DONE = "整理完成"

# 回调模式：timeout 仅等待任务下发确认，不等播完
SPEECH_START_TIMEOUT_SEC = 30.0
SPEECH_ENABLED = False


def speak(robot: Any, text: str, *, timeout_sec: float = SPEECH_START_TIMEOUT_SEC) -> None:
    """非阻塞播报；无接口、下发失败或播完失败均只打日志。"""
    if not SPEECH_ENABLED:
        return
    t = (text or "").strip()
    if not t:
        return
    if not hasattr(robot, "eco_robotSpeech"):
        logger.warning("当前 SDK 无 eco_robotSpeech，跳过播报: %s", t)
        return
    try:
        logger.info("语音播报: %s", t)
        ret = robot.eco_robotSpeech(t)
        if isinstance(ret, str):
            logger.debug("语音任务已下发: task_id=%s", ret)
        else:
            fail = mission_failed(ret)
            if fail:
                logger.warning("语音播报下发失败 (%s): %s", t, fail)
    except Exception as exc:
        logger.warning("语音播报异常 (%s): %s", t, exc)
