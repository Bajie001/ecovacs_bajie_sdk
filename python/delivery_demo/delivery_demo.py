#!/usr/bin/env python3
from __future__ import annotations

import argparse

from errors import DeliveryDemoError
from models import DeliveryConfig
from workflow import delivery_demo


def main() -> int:
    parser = argparse.ArgumentParser(description="Python 玩具配送示例（分层版）")
    parser.add_argument("--ws-url", default="", help="例如 ws://10.10.10.12:9900/")
    parser.add_argument("--user-id", default="delivery_demo_user")
    parser.add_argument("--table-area-name", default="桌子", help="语义地图中的桌子区域名")
    parser.add_argument("--place-direction", default="上", help="semantic_place 的 direction")
    parser.add_argument("--pick-index", type=int, default=0, help="OVD 检测结果中用于抓取的目标索引")
    parser.add_argument(
        "--search-pose-index",
        type=int,
        default=-1,
        help="search 返回多位姿时的选择索引；默认 -1（最后一个）",
    )
    parser.add_argument("--connect-timeout-sec", type=float, default=15.0)
    args = parser.parse_args()

    cfg = DeliveryConfig(
        user_id=args.user_id,
        table_area_name=args.table_area_name,
        place_direction=args.place_direction,
        pick_index=args.pick_index,
        search_pose_index=args.search_pose_index,
        connect_timeout_sec=args.connect_timeout_sec,
    )

    try:
        delivery_demo(ws_url=args.ws_url or None, cfg=cfg)
        return 0
    except KeyboardInterrupt:
        print("\n收到中断，退出")
        return 0
    except DeliveryDemoError as exc:
        print(f"流程失败: {exc}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
