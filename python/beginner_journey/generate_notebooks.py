"""Generate the Bajie SDK beginner journey notebooks.

Run from this directory:
    python3 generate_notebooks.py
"""

from __future__ import annotations

import json
from pathlib import Path
from textwrap import dedent


ROOT = Path(__file__).resolve().parent


def md(source: str) -> dict:
    return {"cell_type": "markdown", "metadata": {}, "source": dedent(source).strip().splitlines(True)}


def code(source: str) -> dict:
    return {
        "cell_type": "code",
        "execution_count": None,
        "metadata": {},
        "outputs": [],
        "source": dedent(source).strip().splitlines(True),
    }


def notebook(cells: list[dict]) -> dict:
    return {
        "cells": cells,
        "metadata": {
            "kernelspec": {
                "display_name": "Python 3",
                "language": "python",
                "name": "python3",
            },
            "language_info": {
                "codemirror_mode": {"name": "ipython", "version": 3},
                "file_extension": ".py",
                "mimetype": "text/x-python",
                "name": "python",
                "pygments_lexer": "ipython3",
                "version": "3.10",
            },
        },
        "nbformat": 4,
        "nbformat_minor": 5,
    }


COMMON_SETUP = code(
    """
    from pathlib import Path
    import sys

    NOTEBOOK_DIR = Path.cwd()
    if str(NOTEBOOK_DIR) not in sys.path:
        sys.path.insert(0, str(NOTEBOOK_DIR))

    from helpers import DEFAULT_WS_URL

    WS_URL = DEFAULT_WS_URL
    print("WS_URL =", WS_URL)
    """
)


NOTEBOOKS: list[tuple[str, list[dict]]] = [
    (
        "README_课程导览.ipynb",
        [
            md(
                """
                # 八界机器人 SDK 小白入门旅程

                这套 notebook 的目标不是把每个 API 都讲完，而是帮你建立一条稳定的学习路径：

                **认识机器人 → 装好 SDK → 连上机器人 → 查接口 → 看状态 → 拍照 → 检测 → 导航/控制 → 异步任务 → 抓放闭环 → 读懂 demo**

                参考文献：《八界机器人 SDK 开发文档（Python）v0.3.0》。
                """
            ),
            md(
                """
                ## 推荐学习顺序

                | 阶段 | Notebook | 你会获得什么 |
                |:---|:---|:---|
                | 认知 | `00_认识八界机器人与SDK.ipynb` | SDK 和机器人任务的基本心智模型 |
                | 环境 | `01_准备环境与安装SDK.ipynb` | Python、wheel、导入验证 |
                | 连接 | `02_第一次连接机器人.ipynb` | 连接、断开、基础状态 |
                | 查询 | `03_学会查接口和字段.ipynb` | 自己查方法、查类型、生成 JSON 模板 |
                | 安全 | `04_机器人状态与安全检查.ipynb` | 任务前 checklist |
                | 感知 | `05_拍照与图像理解.ipynb`、`06_开放词检测入门.ipynb` | 拍照、看图、检测框 |
                | 行动 | `07_导航与基础本体控制.ipynb`、`08_同步任务_异步回调与任务控制.ipynb` | mission、任务控制、回调 |
                | 闭环 | `09_抓取放置最小闭环.ipynb` | 从检测到抓放的最小链路 |
                | 迁移 | `10_进阶任务与现有Demo导读.ipynb` | 读懂现有 demo 并迁移到脚本 |
                """
            ),
            md(
                """
                ## 学习节奏建议

                - **没有机器人时**：完成 00、01、03、10；理解命令和代码结构。
                - **第一次上机器人时**：只完成 02、04、05；先别抓取和移动。
                - **场地安全且有人看护时**：再做 06、07、08、09。
                - 每次真实动作前，先确认：连接地址、电量、告警、工作状态、地图/家具、周围环境。
                """
            ),
            md(
                """
                ## 完成标准

                学完后你应该能做到：

                - 解释 HEAD 相机和 ARM 相机分别适合什么场景。
                - 看懂 `MissionStatus.task_id` 和 `error_info`。
                - 遇到失败时，按“连接 → 状态 → 参数 → 感知 → 执行”排查。
                - 用 `python3 -m bajie_sdk -m eco_xxx` 自己查一个新接口。
                - 把 notebook 中的分步骤代码整理成一个 `.py` 小脚本。
                """
            ),
        ],
    ),
    (
        "00_认识八界机器人与SDK.ipynb",
        [
            md(
                """
                # 00 认识八界机器人与 SDK

                ## 学习目标

                - 知道 `bajie_sdk` 是什么。
                - 知道 `BajieRobot.eco_*` 是主要入口。
                - 分清 WebSocket mission 类能力和 HTTP 感知类能力。
                - 建立第一组机器人使用习惯。

                参考文献：《八界机器人 SDK 开发文档（Python）v0.3.0》第 3、4、6、7 章。
                """
            ),
            md(
                """
                ## 本章完成标准

                你不需要连接机器人。读完后，你应该能回答：

                - SDK 连接的是哪个 IP？
                - 哪些接口属于会让机器人执行任务的 mission？
                - 为什么不能把一串动作命令连续快速发出去？
                """
            ),
            md(
                """
                ## 背景知识

                八界机器人 SDK 把机器人能力封装成 Python 方法。多数业务代码只需要：

                1. `from bajie_sdk import BajieRobot`
                2. 创建 `robot = BajieRobot(ws_url=...)`
                3. `robot.Connect()`
                4. 调用 `robot.eco_*`
                5. `robot.Disconnect()`

                机器人能力大致分两类：

                - **Mission 类能力**：导航、拍照、抓取、放置、本体控制等。它们通常会产生任务，有 `task_id`，需要等待 `finish`。
                - **感知类能力**：开放词检测、VLM 感知、embedding、相似度等。它们通常基于图像走 HTTP 服务。

                一个常见闭环是：

                `拍照 eco_captureImages` → `检测 eco_detect_objects` → `对准 eco_lookto` → `抓取 eco_pick` → `放置 eco_place_in / eco_place_3D`
                """
            ),
            md(
                """
                ## 机器人使用习惯

                - 不要连续狂发动作命令。导航、抓取、放置都需要时间完成。
                - 做任务前先看状态：电量、告警、是否正在工作、地图/家具是否存在。
                - 拍照和检测依赖视角。看不到、被遮挡、光照差，检测就可能失败。
                - Linux 板 IP 才是 SDK 连接地址，常见默认值是 `ws://10.10.10.12:9900/`。这个 IP 需要到机器人网页配置页面查询或设置，不要直接使用屏幕显示的安卓 IP。
                - 真实机器人旁边有人时再执行移动、抓取、放置；notebook 默认把这些开关关掉。
                """
            ),
            COMMON_SETUP,
            code(
                """
                # 先不用连接机器人，只确认本教程的默认连接地址。
                # 后续每个 notebook 顶部都可以改 WS_URL。
                print("SDK 默认机器人地址:", WS_URL)
                print("如果不是这个地址，请到机器人网页配置页面查询 Linux 板 IP，然后改 WS_URL。")
                """
            ),
            md(
                """
                ## 故障排查卡片

                - 不知道接口怎么用：先查 `python -m bajie_sdk -m eco_xxx`。
                - 不知道请求字段：再查 `python -m bajie_sdk -t TypeName`。
                - 任务失败：先看 `MissionStatus.error_info`。
                - 连接失败：先到机器人网页配置页面确认 Linux 板 IP，再检查端口 9900、网络连通性和机器人服务。

                ## 小练习

                用自己的话解释：为什么抓取前通常要先拍照和检测？
                """
            ),
        ],
    ),
    (
        "01_准备环境与安装SDK.ipynb",
        [
            md(
                """
                # 01 准备环境与安装 SDK

                ## 学习目标

                - 检查 Python 版本和 CPU 架构。
                - 选择正确 wheel。
                - 验证 `bajie_sdk` 可以导入。
                - 知道“装到了哪个 Python”。

                参考文献：文档第 1、2、7 章。
                """
            ),
            code(
                """
                import platform
                import sys

                print("Python:", sys.version)
                print("Platform:", platform.platform())
                print("Machine:", platform.machine())
                print("Executable:", sys.executable)

                if sys.version_info[:2] != (3, 10):
                    print("注意：当前发行要求 Python 3.10。请确认 notebook kernel 使用的是 Python 3.10。")
                else:
                    print("Python minor version looks good for cp310 wheels.")
                """
            ),
            code(
                """
                from pathlib import Path

                DEMO_PYTHON_DIR = Path.cwd().parent
                wheels = sorted(DEMO_PYTHON_DIR.glob("bajie_sdk-0.3.0-*.whl"))
                print("wheel search dir:", DEMO_PYTHON_DIR)
                for wheel in wheels:
                    print(wheel.name)
                if not wheels:
                    print("没有找到 wheel。请确认当前 notebook 从 beginner_journey 目录启动。")
                """
            ),
            md(
                """
                ## 安装说明

                根据机器架构选择 wheel：

                - x86_64：`bajie_sdk-0.3.0-cp310-cp310-linux_x86_64.whl`
                - arm64/aarch64：`bajie_sdk-0.3.0-cp310-cp310-linux_aarch64.whl`

                如果这台机器已经安装过 SDK，可以跳过安装 cell，只运行导入验证。
                """
            ),
            code(
                """
                # 默认不自动安装，避免误装到错误的 kernel。
                # 确认 wheel 与 Python 3.10、CPU 架构匹配后，把 INSTALL_SDK 改为 True。
                INSTALL_SDK = False

                if INSTALL_SDK:
                    import platform
                    import subprocess

                    machine = platform.machine()
                    if machine in ("x86_64", "amd64"):
                        wheel = DEMO_PYTHON_DIR / "bajie_sdk-0.3.0-cp310-cp310-linux_x86_64.whl"
                    elif machine in ("aarch64", "arm64"):
                        wheel = DEMO_PYTHON_DIR / "bajie_sdk-0.3.0-cp310-cp310-linux_aarch64.whl"
                    else:
                        raise RuntimeError(f"Unknown architecture: {machine}")

                    subprocess.check_call([sys.executable, "-m", "pip", "install", str(wheel)])
                else:
                    print("INSTALL_SDK is False. Skipping install.")
                """
            ),
            code(
                """
                try:
                    import bajie_sdk
                    print("ok", bajie_sdk.__name__)
                    print("导入成功：后续 notebook 可以继续。")
                except Exception as exc:
                    print("import bajie_sdk failed:", exc)
                    print("请确认 wheel 已安装到当前 notebook kernel 对应的 Python。")
                """
            ),
            md(
                """
                ## 故障排查卡片

                - `cp310` wheel 需要 Python 3.10。
                - x86_64 和 aarch64 wheel 不能混用。
                - pip 下载依赖困难时，可按文档尝试 `pip install ./bajie_sdk-*.whl --no-deps`，但要确认依赖已存在。
                - `python`、`python3`、Jupyter kernel 可能不是同一个解释器，优先看本页打印的 `sys.executable`。

                ## 小练习

                找到你当前机器应该使用哪个 wheel，并说明原因。
                """
            ),
        ],
    ),
    (
        "02_第一次连接机器人.ipynb",
        [
            md(
                """
                # 02 第一次连接机器人

                ## 学习目标

                - 配置 `WS_URL`。
                - 连接和断开机器人。
                - 查询基础状态。
                - 记录第一次连接的结果。

                参考文献：文档第 4、6.7、7 章。
                """
            ),
            COMMON_SETUP,
            code(
                """
                # 如果你的机器人不是默认地址，在这里改：
                # WS_URL = "ws://192.168.x.x:9900/"
                # Linux 板 IP 请到机器人网页配置页面查询或设置。
                CONNECT_ROBOT = False
                """
            ),
            code(
                """
                from helpers import connect_robot, disconnect_safely, print_robot_status

                robot = None
                if CONNECT_ROBOT:
                    robot = connect_robot(WS_URL, timeout_sec=8.0)
                    print("connected")
                    print_robot_status(robot)
                    print("\\n预期输出：能看到 work_state、battery、alarm、position、map、furniture。")
                else:
                    print("CONNECT_ROBOT is False. Read the notes, set it to True, then run this cell.")
                """
            ),
            code(
                """
                disconnect_safely(robot)
                print("disconnected")
                """
            ),
            md(
                """
                ## 机器人习惯

                - 连接成功后，SDK 会维护一份 `robot_info` 缓存。
                - 做任务前先看电量、告警、工作状态。
                - 如果状态看起来是空的，先尝试 `eco_refreshRobotInfo()`。

                ## 故障排查卡片

                - `Connect()` 失败：先到机器人网页配置页面查询或设置 Linux 板 IP，再检查端口 9900、网络连通、机器人服务状态。
                - 连接地址必须包含协议和端口，例如 `ws://10.10.10.12:9900/`。
                - 屏幕上的安卓 IP 不一定是 SDK 要连的 Linux 板 IP；以网页配置页面中的 Linux 板 IP 为准。

                ## 小练习

                连接成功后，记录当前电量、工作状态和是否有告警。

                记录模板：

                - WS_URL：
                - 电量：
                - 工作状态：
                - 告警：
                - 地图名称：
                """
            ),
        ],
    ),
    (
        "03_学会查接口和字段.ipynb",
        [
            md(
                """
                # 03 学会查接口和字段

                ## 学习目标

                - 使用 SDK 自带 CLI 查帮助、接口、类型。
                - 形成“先查方法，再查类型，再写代码”的习惯。
                - 理解 notebook 中为什么用 `python3 -m bajie_sdk`，终端中为什么可以用 `bajie_sdk`。

                参考文献：文档第 3、5 章。
                """
            ),
            md(
                """
                ## 先配置终端补全

                安装 SDK 后，在终端运行一次：

                ```bash
                python3 -m bajie_sdk completion
                ```

                它会在 `~/.bashrc` 或 `~/.zshrc` 中加入：

                ```bash
                alias bajie_sdk='python3 -m bajie_sdk'
                source /path/to/installed/bajie_sdk/cli/completion.sh
                ```

                重新打开终端或执行 `source ~/.bashrc` 后，就可以直接使用
                `bajie_sdk run eco_<TAB>` 补全接口名。

                注意：Jupyter 的 `!命令` 通常不会加载 `.bashrc` 里的 alias。
                所以下面的 notebook cell 使用更稳定的 `python3 -m bajie_sdk`。
                在真实终端里，你可以直接使用 `bajie_sdk -h`、`bajie_sdk run ...`。
                """
            ),
            code("!python3 -m bajie_sdk -h"),
            md(
                """
                你应该看到主帮助，里面包含 `-l`、`-m`、`-t` 和 `run` 子命令说明。
                """
            ),
            code("!python3 -m bajie_sdk -l"),
            code("!python3 -m bajie_sdk -m eco_captureImages"),
            code("!python3 -m bajie_sdk -t SemanticNavigationRequest"),
            code(
                """
                # 生成 JSON 模板，不连接机器人。
                # 真实终端里可写：bajie_sdk run eco_navigateToSemanticArea --json-template
                !python3 -m bajie_sdk run eco_navigateToSemanticArea --json-template
                """
            ),
            md(
                """
                ## 故障排查卡片

                - `No module named bajie_sdk`：SDK 没装到当前 notebook kernel。
                - 方法名不确定：先 `-l` 看分类，再 `-m eco_xxx` 看签名。
                - JSON 不知道怎么写：优先用 `bajie_sdk run eco_xxx --json-template`。

                ## 小练习

                查出 `eco_setRobotHead` 的参数和默认超时时间。
                """
            ),
        ],
    ),
    (
        "04_机器人状态与安全检查.ipynb",
        [
            md(
                """
                # 04 机器人状态与安全检查

                ## 学习目标

                - 查询工作状态、电量、告警、位置、地图、家具。
                - 建立任务前安全 checklist。
                - 判断现在是否适合执行真实动作。

                参考文献：文档第 6.7 章。
                """
            ),
            COMMON_SETUP,
            code("CONNECT_ROBOT = False"),
            code(
                """
                from helpers import connect_robot, disconnect_safely, print_robot_status

                robot = connect_robot(WS_URL) if CONNECT_ROBOT else None
                if robot:
                    print_robot_status(robot)
                else:
                    print("Set CONNECT_ROBOT = True after checking the robot.")
                """
            ),
            code(
                """
                if robot:
                    ws = robot.eco_robotWorkState()
                    cmd = getattr(ws, "cmd", None) if ws else None
                    if cmd == "working":
                        print("机器人正在工作。等待完成，或确认 task_id 后再取消。")
                    elif cmd == "pause":
                        print("机器人处于暂停状态。确认任务后再 resume/cancel。")
                    elif cmd == "idle":
                        print("机器人空闲。可以开始安全检查后的任务。")
                    else:
                        print("工作状态未知:", ws)
                """
            ),
            code("disconnect_safely(robot)"),
            md(
                """
                ## 任务前 checklist

                - 已连接机器人。
                - 电量足够。
                - 无告警或告警已确认不影响当前任务。
                - 机器人不是 `working` 状态。
                - 需要导航时，地图和语义家具列表存在。
                - 场地没有人、线缆、障碍物处于危险位置。

                ## 故障排查卡片

                - 无地图/无家具：语义导航、找物、语义地图相关任务不可用。
                - 机器人正在工作：不要直接发新任务，先等待完成或用正确 `task_id` 控制任务。
                - 有告警：先处理机器人端异常，再继续 SDK 调用。

                ## 小练习

                写下你准备运行导航前要确认的 3 个状态字段。
                """
            ),
        ],
    ),
    (
        "05_拍照与图像理解.ipynb",
        [
            md(
                """
                # 05 拍照与图像理解

                ## 学习目标

                - 用 HEAD/ARM 相机拍照。
                - 显示 RGB 图，查看深度图信息。
                - 理解两个相机的使用场景。
                - 通过 shape 判断图像是否正常返回。

                参考文献：文档第 6.3.1 章。
                """
            ),
            COMMON_SETUP,
            code(
                """
                CONNECT_ROBOT = False
                CAPTURE_HEAD = False
                CAPTURE_ARM = False
                """
            ),
            code(
                """
                from helpers import connect_robot, disconnect_safely, show_rgb, summarize_view
                from bajie_sdk import CameraType

                robot = connect_robot(WS_URL) if CONNECT_ROBOT else None
                """
            ),
            code(
                """
                head_view = None
                if robot and CAPTURE_HEAD:
                    head_view = robot.eco_captureImages(CameraType.HEAD, timeout_sec=30.0)
                    show_rgb(head_view, "HEAD camera")
                    summarize_view(head_view)
                else:
                    print("Set CONNECT_ROBOT and CAPTURE_HEAD to True to capture a head image.")
                """
            ),
            code(
                """
                arm_view = None
                if robot and CAPTURE_ARM:
                    arm_view = robot.eco_captureImages(CameraType.ARM, timeout_sec=30.0)
                    show_rgb(arm_view, "ARM camera")
                    summarize_view(arm_view)
                else:
                    print("Set CONNECT_ROBOT and CAPTURE_ARM to True to capture an arm image.")
                """
            ),
            code("disconnect_safely(robot)"),
            md(
                """
                ## 机器人背景知识

                - HEAD 适合看场景、桌面、家具、放置区域。
                - ARM 适合抓取前近距离确认目标。
                - 图像质量会受光照、姿态、距离、遮挡影响。

                ## 故障排查卡片

                - 拍照超时：检查机器人是否忙、相机服务是否正常、网络是否稳定。
                - 图像为空或视角不对：先调整头部角度/机身高度/位置。
                - 任务没完成时重复拍照可能让状态混乱，先等上一个 mission finish。

                ## 小练习

                说明一个适合 HEAD 相机的任务和一个适合 ARM 相机的任务。
                """
            ),
        ],
    ),
    (
        "06_开放词检测入门.ipynb",
        [
            md(
                """
                # 06 开放词检测入门

                ## 学习目标

                - 使用 `eco_detect_objects` 检测物品。
                - 可视化检测框。
                - 理解 OVD 与 SHOE endpoint。
                - 根据检测数量和 bbox 判断是否适合进入抓取流程。

                参考文献：文档第 6.1.1 章。
                """
            ),
            COMMON_SETUP,
            code(
                """
                CONNECT_ROBOT = False
                CAPTURE_IMAGE = False
                DETECT_OBJECTS = False
                LABELS = ["鞋子"]
                USE_SHOE_ENDPOINT = True
                """
            ),
            code(
                """
                from helpers import connect_robot, disconnect_safely, show_rgb, draw_bboxes, summarize_detections
                from bajie_sdk import CameraType, DetectObjectsRequest, OvdEndpoint

                robot = connect_robot(WS_URL) if CONNECT_ROBOT else None
                view = None
                if robot and CAPTURE_IMAGE:
                    view = robot.eco_captureImages(CameraType.HEAD, timeout_sec=30.0)
                    show_rgb(view, "Image for detection")
                else:
                    print("Set CONNECT_ROBOT and CAPTURE_IMAGE to True first.")
                """
            ),
            code(
                """
                detections = []
                if robot and view is not None and DETECT_OBJECTS:
                    entry = OvdEndpoint.SHOE if USE_SHOE_ENDPOINT else OvdEndpoint.OVD
                    resp = robot.eco_detect_objects(
                        DetectObjectsRequest(
                            rgb_image=view.rgb_image.img,
                            labels=LABELS,
                            entry=entry,
                        ),
                        timeout=15.0,
                    )
                    detections = list(resp.items)
                    summarize_detections(detections)
                    draw_bboxes(view, detections, "Detection results")
                else:
                    print("Set DETECT_OBJECTS = True after capturing an image.")
                """
            ),
            code("disconnect_safely(robot)"),
            md(
                """
                ## 机器人习惯

                - 标签词尽量具体，比如“鞋子”“杯子”“玩具汽车”通常比“东西”更好。
                - 检测依赖图片质量；先让目标出现在画面中，再检测。
                - SHOE endpoint 是鞋子专用检测，其他开放词通常走 OVD。

                ## 故障排查卡片

                - 没检测到：换标签、换角度、降低阈值、确认目标在画面里。
                - 父类标签可能会自动展开为子类，但具体任务中仍建议用明确标签。
                - 服务未配置或不可达时，感知接口可能超时或报错。

                ## 小练习

                换一个标签，观察检测框数量和位置是否变化。
                """
            ),
        ],
    ),
    (
        "07_导航与基础本体控制.ipynb",
        [
            md(
                """
                # 07 导航与基础本体控制

                ## 学习目标

                - 语义导航到区域。
                - 控制头部和升降高度。
                - 谨慎理解底盘相对运动。
                - 每个动作后检查 `MissionStatus`。

                参考文献：文档第 6.2、6.5 章。
                """
            ),
            COMMON_SETUP,
            code(
                """
                CONNECT_ROBOT = False
                ENABLE_NAVIGATION = False
                ENABLE_HEAD = False
                ENABLE_HEIGHT = False
                ENABLE_MOVE = False

                AREA_NAME = "客厅"
                HEAD_RAD = 0.83
                HEIGHT_M = 0.22
                MOVE_DISTANCE_M = 0.05
                MOVE_ANGLE_RAD = 0.0
                """
            ),
            code(
                """
                from helpers import connect_robot, disconnect_safely, print_mission_status, print_safety_gate, require_enabled
                from bajie_sdk import SemanticNavigationRequest

                robot = connect_robot(WS_URL) if CONNECT_ROBOT else None
                print_safety_gate(connected=robot is not None, enabled=ENABLE_NAVIGATION, action="semantic navigation")
                """
            ),
            code(
                """
                if robot and ENABLE_HEAD:
                    st = robot.eco_setRobotHead(HEAD_RAD, timeout_sec=30.0)
                    print_mission_status(st)
                else:
                    print("Head control disabled.")
                """
            ),
            code(
                """
                if robot and ENABLE_HEIGHT:
                    st = robot.eco_setRobotHeight(HEIGHT_M, timeout_sec=30.0)
                    print_mission_status(st)
                else:
                    print("Height control disabled.")
                """
            ),
            code(
                """
                if robot and ENABLE_NAVIGATION:
                    req = SemanticNavigationRequest(area_id="", area_name=AREA_NAME)
                    st = robot.eco_navigateToSemanticArea(req, timeout_sec=120.0)
                    print_mission_status(st)
                else:
                    print("Navigation disabled.")
                """
            ),
            code(
                """
                if robot and ENABLE_MOVE:
                    require_enabled(ENABLE_MOVE, "eco_moveChassis")
                    st = robot.eco_moveChassis(MOVE_DISTANCE_M, MOVE_ANGLE_RAD, timeout_sec=30.0)
                    print_mission_status(st)
                else:
                    print("Chassis move disabled by default.")
                """
            ),
            code("disconnect_safely(robot)"),
            md(
                """
                ## 机器人背景知识

                - 导航依赖地图和语义区域。
                - 头部角度单位是弧度。
                - 升降高度有范围限制，SDK 会对越界值截断并告警。
                - 底盘移动是真实运动，运行前要确认周围安全。

                ## 故障排查卡片

                - 区域名错误：先用 `eco_robotFurniture()` 看语义地图信息。
                - 被障碍物挡住：清理路径或换目标点。
                - 任务失败：先看 `error_info.code` 和 `error_info.message`。

                ## 小练习

                查询家具列表，找出一个真实区域名替换 `AREA_NAME`。
                """
            ),
        ],
    ),
    (
        "08_同步任务_异步回调与任务控制.ipynb",
        [
            md(
                """
                # 08 同步任务、异步回调与任务控制

                ## 学习目标

                - 对比阻塞模式和 `on_task_done` 回调模式。
                - 理解 `task_id`。
                - 了解暂停、恢复、取消。
                - 知道回调模式下 `timeout_sec` 等的是 start 确认。

                参考文献：文档第 4.2、6.6 章。
                """
            ),
            COMMON_SETUP,
            code(
                """
                CONNECT_ROBOT = False
                RUN_SYNC_CAPTURE = False
                RUN_ASYNC_CAPTURE = False
                ENABLE_CANCEL = False
                """
            ),
            code(
                """
                import time
                from helpers import connect_robot, disconnect_safely
                from bajie_sdk import CameraType, Command

                robot = connect_robot(WS_URL) if CONNECT_ROBOT else None
                async_task_id = None
                """
            ),
            code(
                """
                if robot and RUN_SYNC_CAPTURE:
                    view = robot.eco_captureImages(CameraType.HEAD, timeout_sec=30.0)
                    print("同步拍照完成:", view.rgb_image.img.shape)
                else:
                    print("Sync capture disabled.")
                """
            ),
            code(
                """
                done_results = []

                def on_images_ready(result):
                    print("回调收到图像:", result.rgb_image.img.shape)
                    done_results.append(result)

                if robot and RUN_ASYNC_CAPTURE:
                    async_task_id = robot.eco_captureImages(
                        CameraType.HEAD,
                        timeout_sec=30.0,
                        on_task_done=on_images_ready,
                    )
                    print("立即返回 task_id:", async_task_id)
                    print("等待回调几秒...")
                    time.sleep(5)
                    print("回调结果数量:", len(done_results))
                    print("如果数量仍为 0，任务可能还没 finish，或 notebook 主线程等待时间太短。")
                else:
                    print("Async capture disabled.")
                """
            ),
            code(
                """
                # 任务控制需要真实 task_id。默认关闭。
                if robot and ENABLE_CANCEL and async_task_id:
                    st = robot.eco_missionControl(Command.CANCEL, async_task_id, timeout_sec=30.0)
                    print(st)
                else:
                    print("Cancel disabled or no task_id.")
                """
            ),
            code("disconnect_safely(robot)"),
            md(
                """
                ## 机器人习惯

                - 长任务适合异步，例如导航、回充、抓取。
                - 回调在 SDK 收包线程执行，回调里不要做耗时操作。
                - 取消/暂停/恢复需要正确的 `task_id`。
                - 回调模式下，`timeout_sec` 用于等待 mission start 确认，不是等待 finish。

                ## 故障排查卡片

                - 回调没有触发：确认任务是否真的完成，主线程是否过早退出。
                - 取消失败：确认任务仍在运行、`task_id` 正确。
                - 不确定当前任务：先看 `eco_robotWorkState()`。

                ## 小练习

                用一句话说明同步调用和异步回调模式的返回值差异。
                """
            ),
        ],
    ),
    (
        "09_抓取放置最小闭环.ipynb",
        [
            md(
                """
                # 09 抓取放置最小闭环

                ## 学习目标

                - 按可观察步骤完成抓取/放置链路。
                - 理解 HEAD 视角、ARM 视角、bbox 匹配的关系。
                - 在真实执行前检查每一步中间结果。
                - 知道每个开关应该按什么顺序打开。

                参考文献：文档第 4.1、6.4.5 至 6.4.11 章。
                """
            ),
            COMMON_SETUP,
            code(
                """
                CONNECT_ROBOT = False
                CAPTURE_HEAD = False
                DETECT_HEAD = False
                ENABLE_LOOKTO = False
                CAPTURE_ARM = False
                DETECT_ARM = False
                ENABLE_PICK = False
                ENABLE_PLACE = False

                LABELS = ["玩具"]
                PLACE_BBOX = [153.0, 230.0, 403.0, 384.0]

                # 推荐开关顺序：
                # 1. CONNECT_ROBOT + CAPTURE_HEAD
                # 2. DETECT_HEAD
                # 3. ENABLE_LOOKTO + CAPTURE_ARM
                # 4. DETECT_ARM
                # 5. 确认两个 bbox 都合理后 ENABLE_PICK
                # 6. 确认放置框合理后 ENABLE_PLACE
                """
            ),
            code(
                """
                from helpers import (
                    connect_robot, disconnect_safely, draw_bboxes, first_or_none,
                    print_mission_status, require_enabled, show_rgb,
                    summarize_detections, summarize_view,
                )
                from bajie_sdk import CameraType, DetectObjectsRequest, OvdEndpoint

                robot = connect_robot(WS_URL) if CONNECT_ROBOT else None
                head_view = None
                head_items = []
                target = None
                arm_view = None
                arm_items = []
                matched = None
                """
            ),
            code(
                """
                if robot and CAPTURE_HEAD:
                    head_view = robot.eco_captureImages(CameraType.HEAD, timeout_sec=30.0)
                    show_rgb(head_view, "HEAD view")
                    summarize_view(head_view)
                else:
                    print("Head capture disabled.")
                """
            ),
            code(
                """
                if robot and head_view is not None and DETECT_HEAD:
                    resp = robot.eco_detect_objects(
                        DetectObjectsRequest(
                            rgb_image=head_view.rgb_image.img,
                            labels=LABELS,
                            entry=OvdEndpoint.OVD,
                        ),
                        timeout=15.0,
                    )
                    head_items = list(resp.items)
                    summarize_detections(head_items)
                    draw_bboxes(head_view, head_items, "HEAD detections")
                    first = first_or_none(head_items)
                    target = first.to_named_bbox_for_grab() if first else None
                    print("target:", target)
                else:
                    print("Head detection disabled or no head_view.")
                """
            ),
            code(
                """
                if robot and target is not None and ENABLE_LOOKTO:
                    st = robot.eco_lookto(
                        target.bbox,
                        view=head_view,
                        camera_type=CameraType.ARM,
                        timeout_sec=30.0,
                    )
                    print_mission_status(st)
                else:
                    print("lookto disabled or no target.")
                """
            ),
            code(
                """
                if robot and CAPTURE_ARM:
                    arm_view = robot.eco_captureImages(CameraType.ARM, timeout_sec=30.0)
                    show_rgb(arm_view, "ARM view")
                    summarize_view(arm_view)
                else:
                    print("Arm capture disabled.")
                """
            ),
            code(
                """
                if robot and arm_view is not None and DETECT_ARM:
                    label = (getattr(target, "name", "") or "object") if target else "object"
                    resp = robot.eco_detect_objects(
                        DetectObjectsRequest(
                            rgb_image=arm_view.rgb_image.img,
                            labels=[label],
                            entry=OvdEndpoint.OVD,
                        ),
                        timeout=15.0,
                    )
                    arm_items = list(resp.items)
                    summarize_detections(arm_items)
                    draw_bboxes(arm_view, arm_items, "ARM detections")
                    if target and arm_items:
                        matched = robot.eco_match_obj_views(head_view.rgb_image.img, target.bbox, arm_view.rgb_image.img, arm_items)
                        print("matched:", matched)
                else:
                    print("Arm detection disabled or no arm_view.")
                """
            ),
            code(
                """
                if robot and matched is not None and ENABLE_PICK:
                    require_enabled(ENABLE_PICK, "eco_pick")
                    st = robot.eco_pick(arm_view, matched, timeout_sec=120.0)
                    print_mission_status(st)
                else:
                    print("Pick disabled or no matched bbox.")
                """
            ),
            code(
                """
                if robot and head_view is not None and ENABLE_PLACE:
                    require_enabled(ENABLE_PLACE, "eco_place_in")
                    st = robot.eco_place_in(head_view, PLACE_BBOX, timeout_sec=120.0)
                    print_mission_status(st)
                else:
                    print("Place disabled or no head_view.")
                """
            ),
            code("disconnect_safely(robot)"),
            md(
                """
                ## 机器人背景知识

                - 抓取前要确认目标框准不准。
                - HEAD 视角和 ARM 视角不是同一张图，所以需要 `eco_match_obj_views`。
                - 放置可以使用像素框、像素点或 3D pose，最小入门先使用容器框。

                ## 故障排查卡片

                - bbox 不准：重新拍照、换标签、换角度。
                - ARM 视角没看到物体：重新 `lookto` 或调整检测标签。
                - 抓取失败：先恢复姿态，确认环境安全，再继续。

                ## 小练习

                在真正打开 `ENABLE_PICK` 前，截图或记录 HEAD/ARM 两个检测框是否都合理。
                """
            ),
        ],
    ),
    (
        "10_进阶任务与现有Demo导读.ipynb",
        [
            md(
                """
                # 10 进阶任务与现有 Demo 导读

                ## 学习目标

                - 看懂现有 demo 的工作流。
                - 知道 notebook 如何迁移成 `.py` 脚本。
                - 学会按阶段定位完整任务失败。
                - 选择一个 demo 作为下一步练习。

                参考文献：文档第 4 章常用链路，以及 `bajie_demo/python` 下各 demo README。
                """
            ),
            code(
                """
                from pathlib import Path

                DEMO_ROOT = Path.cwd().parent
                for name in ["desk_demo", "toy_storage", "shoe_sorting", "delivery_demo"]:
                    readme = DEMO_ROOT / name / "README.md"
                    print(f"=== {name} ===")
                    print(readme)
                    print(readme.exists())
                """
            ),
            md(
                """
                ## Demo 地图

                - **桌面整理 `desk_demo`**：重定位、准备姿态、语义导航、头部选角、VLM 感知、VLM 规划、6D 姿态、抓取、精准放置。
                - **玩具收纳 `toy_storage`**：搜索收纳筐、搜索玩具、过滤筐内物体、手部观测、OVD 检测、抓取、PutWhere 放置。
                - **鞋子整理 `shoe_sorting`**：SHOE 检测、跨帧配对、鞋子摆放规划、执行整理。
                - **送物 `delivery_demo`**：导航、找人、任务编排。
                """
            ),
            code(
                """
                # 从 notebook 迁移到脚本时，通常保留这些结构：
                # 1. 配置 dataclass / argparse
                # 2. connect_robot
                # 3. 分阶段函数：perception -> planning -> execution
                # 4. 每个 mission 后检查 error_info
                # 5. finally 中 disconnect
                print("Read the demo README files, then open each workflow.py from simple to complex.")
                """
            ),
            md(
                """
                ## 故障排查卡片

                完整 demo 失败时，不要一次性猜原因。按阶段拆：

                1. **连接**：IP、端口、服务、SDK 安装。
                2. **状态**：电量、告警、是否 busy、地图/家具。
                3. **感知**：图像是否清楚、检测框是否合理、VLM/OVD/PutWhere 是否配置。
                4. **规划**：输入字段、区域名、目标描述是否正确。
                5. **执行**：导航是否可达、抓取框是否准确、放置区域是否合理。

                VLM 相关接口使用前，需要在 Bajie Robot Web 配置页面中设置 VLM 模型 API。

                ## 小练习

                选择一个 demo，画出它的“感知 → 规划 → 执行”三段流程。
                """
            ),
        ],
    ),
]


def main() -> None:
    for filename, cells in NOTEBOOKS:
        path = ROOT / filename
        path.write_text(json.dumps(notebook(cells), ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(path.name)


if __name__ == "__main__":
    main()
