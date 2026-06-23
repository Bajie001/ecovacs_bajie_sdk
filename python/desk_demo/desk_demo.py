#!/usr/bin/env python3
from __future__ import annotations

import argparse

from errors import DeskDemoError
from models import DeskConfig
from workflow import desk_workflow


def main() -> int:
    parser = argparse.ArgumentParser(description="Python 桌面整理示例（分层版）")
    parser.add_argument("--ws-url", default="", help="例如 ws://10.10.10.12:9900/")
    parser.add_argument("--area-name", default="书桌", help="语义地图中的桌子区域名")
    parser.add_argument("--area-id", default="", help="目标区域ID")
    parser.add_argument(
        "--user-input",
        default="",
        help="用户意图补充信息（如：'只整理文具，其他不动'）",
    )
    parser.add_argument(
        "--stop-before-grab",
        action="store_true",
        help="执行到 bbox 获取后停止，便于调试抓取",
    )
    parser.add_argument("--connect-timeout-sec", type=float, default=60.0)
    parser.add_argument(
        "--max-loops",
        type=int,
        default=3,
        help="最大循环次数（包含首次规划，默认: 3）",
    )
    parser.add_argument(
        "--no-head-angle-suggest",
        action="store_true",
        help="跳过 eco_vlm_suggest_angle（升高后直接按当前头部角度拍照）",
    )
    parser.add_argument(
        "--head-sweep-rad",
        default="",
        help="逗号分隔的头部采样弧度，供 eco_vlm_suggest_angle（至少 2 个数）；默认内置六档",
    )
    parser.add_argument(
        "--memory-input",
        default="",
        help="记忆经验（可抓清单由 grab_items.md 自动拼接，无需在此填写）",
    )
    parser.add_argument(
        "--grab-items-path",
        default="",
        help="可抓清单 md 路径，默认 desk_demo/grab_items.md",
    )
    args = parser.parse_args()

    head_sweep: tuple[float, ...] | None = None
    raw_sw = (args.head_sweep_rad or "").strip()
    if raw_sw:
        parts = [p.strip() for p in raw_sw.split(",") if p.strip()]
        try:
            head_sweep = tuple(float(p) for p in parts)
        except ValueError as exc:
            raise SystemExit(f"invalid --head-sweep-rad: {exc}") from exc
        if len(head_sweep) < 2:
            raise SystemExit("--head-sweep-rad 至少需要 2 个弧度值")

    cfg = DeskConfig(
        area_name=args.area_name,
        area_id=args.area_id,
        user_input=args.user_input,
        stop_before_grab=args.stop_before_grab,
        connect_timeout_sec=args.connect_timeout_sec,
        max_loops=args.max_loops,
        enable_vlm_head_angle_suggest=(not args.no_head_angle_suggest),
        head_sweep_angles_rad=head_sweep if head_sweep is not None else DeskConfig.head_sweep_angles_rad,
        memory_input=(args.memory_input or "").strip(),
        grab_items_path=(args.grab_items_path or "").strip(),
    )

    try:
        desk_workflow(ws_url=args.ws_url or None, cfg=cfg)
        return 0
    except KeyboardInterrupt:
        print("\n收到中断，退出")
        return 0
    except DeskDemoError as exc:
        print(f"流程失败: {exc}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
