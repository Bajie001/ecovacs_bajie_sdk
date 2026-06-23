#!/usr/bin/env python3
from __future__ import annotations

import argparse

from errors import ToyStorageError
from models import ToyStorageConfig
from workflow import toy_storage


def main() -> int:
    parser = argparse.ArgumentParser(description="Python 玩具收纳示例（分层版）")
    parser.add_argument("--ws-url", default="", help="例如 ws://10.10.10.12:9900/")
    parser.add_argument("--user-id", default="toy_demo_user")
    parser.add_argument("--area-name", default="", help="可选搜索区域")
    parser.add_argument("--direction", default="里", help="semantic_place direction")
    parser.add_argument("--carrier", default="", help="PutWhere carrier（默认空串）")
    parser.add_argument("--carrier-direct", default="", help="PutWhere carrier_direct（默认空串）")
    parser.add_argument("--pick-index", type=int, default=0, help="OVD 检测结果中用于抓取的目标索引")
    parser.add_argument(
        "--search-pose-index",
        type=int,
        default=-1,
        help="search 返回多位姿时的选择索引；默认 -1（最后一个）",
    )
    parser.add_argument("--connect-timeout-sec", type=float, default=15.0)
    parser.add_argument("--max-rounds", type=int, default=0, help="0 为无限循环")
    args = parser.parse_args()

    cfg = ToyStorageConfig(
        user_id=args.user_id,
        area_name=args.area_name,
        direction=args.direction,
        carrier=args.carrier,
        carrier_direct=args.carrier_direct,
        pick_index=args.pick_index,
        search_pose_index=args.search_pose_index,
        connect_timeout_sec=args.connect_timeout_sec,
    )

    try:
        toy_storage(ws_url=args.ws_url or None, cfg=cfg, max_rounds=args.max_rounds)
        return 0
    except KeyboardInterrupt:
        print("\n收到中断，退出")
        return 0
    except ToyStorageError as exc:
        print(f"流程失败: {exc}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
