from __future__ import annotations

from dataclasses import dataclass


@dataclass
class ToyStorageConfig:
    user_id: str = "toy_demo_user"
    area_name: str = ""
    direction: str = "里"
    carrier: str = ""
    carrier_direct: str = ""
    pick_index: int = 0
    search_pose_index: int = -1
    connect_timeout_sec: float = 15.0
    loop_sleep_sec: float = 0.01
