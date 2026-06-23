from __future__ import annotations

import json
import logging
import os
import uuid
from pathlib import Path
from typing import Any, Mapping, TypeVar

T = TypeVar("T")


def configure_logging(level: int = logging.INFO) -> None:
    """
    配置脚本日志。

    说明：
    - 这些 step 脚本通常以"单文件脚本"形式运行，为避免重复配置导致 handler 叠加，
      这里使用 `basicConfig` 的幂等行为（若 root logger 已有 handler 则不重复配置）。
    """
    logging.basicConfig(
        level=level,
        format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
    )


def new_task_id() -> str:
    return str(uuid.uuid4())


def ensure_dir(path: str | os.PathLike[str]) -> None:
    Path(path).mkdir(parents=True, exist_ok=True)


def read_json(path: str | os.PathLike[str]) -> Any:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def write_json(path: str | os.PathLike[str], data: Any) -> None:
    ensure_dir(Path(path).parent)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)


def rgb_image_dict_to_rgb_ndarray(rgb_image: Mapping[str, Any] | None) -> Any | None:
    """
    将感知/拍照落盘的 ``rgb_image`` dict（含 ``img`` 或 base64 ``data``）转为 RGB ndarray(H,W,3)，
    供 ``eco_vlm_judge`` 等需要 ``numpy.ndarray`` 的 SDK 接口使用。

    依赖 opencv-python（随 bajie_sdk 一并安装）。
    """
    import base64

    import cv2
    import numpy as np

    if not isinstance(rgb_image, Mapping):
        return None
    raw_img = rgb_image.get("img")
    if raw_img is not None:
        arr = np.asarray(raw_img, dtype=np.uint8)
        if arr.ndim == 3 and arr.shape[2] >= 3:
            return np.ascontiguousarray(arr[..., :3])
        return None
    data = rgb_image.get("data")
    if not isinstance(data, str):
        return None
    s = data.strip()
    if not s:
        return None
    if "," in s:
        s = s.split(",", 1)[1]
    try:
        buf = base64.b64decode(s)
        arr = np.frombuffer(buf, dtype=np.uint8)
        bgr = cv2.imdecode(arr, cv2.IMREAD_COLOR)
        if bgr is None:
            return None
        return cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)
    except Exception:
        return None
