from __future__ import annotations

from dataclasses import dataclass


@dataclass
class DeliveryConfig:
    user_id: str = "delivery_demo_user"
    table_area_name: str = "桌子"
    place_direction: str = "上"
    pick_index: int = 0
    search_pose_index: int = -1
    connect_timeout_sec: float = 15.0
