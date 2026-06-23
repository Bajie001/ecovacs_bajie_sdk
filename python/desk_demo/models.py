from __future__ import annotations

from dataclasses import dataclass, field

FIXED_IMG_W = 640
FIXED_IMG_H = 400


@dataclass
class DeskConfig:
    area_name: str = "书桌"
    area_id: str = ""
    user_input: str = ""
    stop_before_grab: bool = False
    img_w: int = field(default=FIXED_IMG_W, init=False)
    img_h: int = field(default=FIXED_IMG_H, init=False)
    connect_timeout_sec: float = 60.0
    max_loops: int = 3
    # 头部相机：升高后先用 eco_vlm_suggest_angle 选俯仰角，再正式拍照（SDK v0.2.0 / eco_vlm_suggest_angle）
    enable_vlm_head_angle_suggest: bool = True
    head_sweep_angles_rad: tuple[float, ...] = (0.15, 0.2, 0.25, 0.3, 0.35, 0.4)
    head_sweep_sleep_sec: float = 0.3
    vlm_suggest_angle_timeout_sec: float = 150.0
    # 用户侧 memory_input：仅记忆经验；发 plan 前由 demo 自动前缀【可抓清单】
    memory_input: str = ""
    grab_items_path: str = ""
