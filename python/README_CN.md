# 八界机器人 SDK 开发文档（Python）

本文档面向最终使用者，介绍如何获取并安装 `bajie_sdk`、如何快速调用机器人能力，以及如何通过 CLI 查询接口与字段说明。

- **建议开发者选用 ubuntu 系统 x86_64 的 PC 进行开发及调试，windows 系统 x86_64 的 PC 安装 python 相关环境可自行配置，本文以 ubuntu 系统为例进行讲解**
- **在调试 API 函数和开发 demo 场景前需要确认机器是否有地图，若没有可以通过机身屏幕上的建图按钮和可视化开发平台中机器人控制中的建图按钮，以及调用 API 中的自动建图的 API 进行建图**

---

## 目录

- [1. 概述](#1-概述)
  - [1.1 SDK 是什么](#11-sdk-是什么)
  - [1.2 关键概念](#12-关键概念)
  - [1.3 建图前提](#13-建图前提)
- [2. 准备工作](#2-准备工作)
  - [2.1 获取 whl 包](#21-获取-whl-包)
  - [2.2 安装 SDK](#22-安装-sdk)
  - [2.3 验证安装](#23-验证安装)
  - [2.4 配置命令行补全（可选）](#24-配置命令行补全可选)
- [3. 快速开始](#3-快速开始)
  - [3.1 第一个程序：连接机器人并读取状态](#31-第一个程序连接机器人并读取状态)
  - [3.2 常用任务链路](#32-常用任务链路)
  - [3.3 抓取/放置流程](#33-抓取放置流程)
  - [3.4 异步回调模式](#34-异步回调模式)
  - [3.5 连接事件与错误处理](#35-连接事件与错误处理)
- [4. CLI 使用说明](#4-cli-使用说明)
  - [4.1 查询与帮助](#41-查询与帮助)
  - [4.2 执行单接口（run 子命令）](#42-执行单接口run-子命令)
  - [4.3 内联引用与图片占位符](#43-内联引用与图片占位符)
  - [4.4 返回值编码与图床](#44-返回值编码与图床)
- [5. `BajieRobot.eco_*` 接口一览（全量）](#5-bajieroboteco_-接口一览全量)
  - [5.1 感知](#51-感知)
    - [5.1.1 `eco_detect_objects`](#511-eco_detect_objects)
    - [5.1.2 `eco_build_embeddings`](#512-eco_build_embeddings)
    - [5.1.3 `eco_calc_similarity`](#513-eco_calc_similarity)
    - [5.1.4 `eco_pairwise`](#514-eco_pairwise)
    - [5.1.5 `eco_shoe_placement_plan`](#515-eco_shoe_placement_plan)
    - [5.1.6 `eco_vlm_desk_sort_plan`](#516-eco_vlm_desk_sort_plan)
    - [5.1.7 `eco_vlm_perception`](#517-eco_vlm_perception)
    - [5.1.8 `eco_vlm_match`](#518-eco_vlm_match)
    - [5.1.9 `eco_vlm_suggest_angle`](#519-eco_vlm_suggest_angle)
    - [5.1.10 `eco_vlm_judge`](#5110-eco_vlm_judge)
    - [5.1.11 `eco_put_where_summary`](#5111-eco_put_where_summary)
  - [5.2 建图与导航](#52-建图与导航)
    - [5.2.1 `eco_autoMapBuild`](#521-eco_automapbuild)
    - [5.2.2 `eco_semanticMapBuild`](#522-eco_semanticmapbuild)
    - [5.2.3 `eco_manageSemanticMap`](#523-eco_managesemanticmap)
    - [5.2.4 `eco_moveChassis`](#524-eco_movechassis)
    - [5.2.5 `eco_navigateToPoint`](#525-eco_navigatetopoint)
    - [5.2.6 `eco_navigateToSemanticArea`](#526-eco_navigatetosemanticarea)
    - [5.2.7 `eco_leaveDock`](#527-eco_leavedock)
    - [5.2.8 `eco_relocate`](#528-eco_relocate)
    - [5.2.9 `eco_startRecharge`](#529-eco_startrecharge)
  - [5.3 图像与位姿](#53-图像与位姿)
    - [5.3.1 `eco_captureImages`](#531-eco_captureimages)
    - [5.3.2 `eco_computeObjectPose`](#532-eco_computeobjectpose)
  - [5.4 高层任务](#54-高层任务)
    - [5.4.1 `eco_locatePerson`](#541-eco_locateperson)
    - [5.4.2 `eco_robotSpeech`](#542-eco_robotspeech)
    - [5.4.3 `eco_findObject`](#543-eco_findobject)
    - [5.4.4 `eco_prepareRobotPose`](#544-eco_preparerobotpose)
    - [5.4.5 `eco_finishRobotPose`](#545-eco_finishrobotpose)
    - [5.4.6 `eco_lookto`](#546-eco_lookto)
    - [5.4.7 `eco_pick`](#547-eco_pick)
    - [5.4.8 `eco_match_obj_views`](#548-eco_match_obj_views)
    - [5.4.9 `eco_pick_with_arm_review`](#549-eco_pick_with_arm_review)
    - [5.4.10 `eco_place_3D`](#5410-eco_place_3d)
    - [5.4.11 `eco_place_with_view`](#5411-eco_place_with_view)
    - [5.4.12 `eco_place_in`](#5412-eco_place_in)
  - [5.5 本体控制](#55-本体控制)
    - [5.5.1 `eco_setRobotHeight`](#551-eco_setrobotheight)
    - [5.5.2 `eco_setRobotHead`](#552-eco_setrobothead)
    - [5.5.3 `eco_setRobotArmMode`](#553-eco_setrobotarmmode)
    - [5.5.4 `eco_setRobotPose`](#554-eco_setrobotpose)
  - [5.6 任务暂停/恢复/取消](#56-任务暂停--恢复--取消)
    - [5.6.1 `eco_cancelAllMissions`](#561-eco_cancelallmissions)
    - [5.6.2 `eco_missionControl`](#562-eco_missioncontrol)
  - [5.7 机器状态](#57-机器状态)
    - [5.7.1 `eco_robotInfo`](#571-eco_robotinfo)
    - [5.7.2 `eco_refreshRobotInfo`](#572-eco_refreshrobotinfo)
    - [5.7.3 `eco_robotWorkState` ~ `eco_robotFurniture`](#573-eco_robotworkstate--eco_robotfurniture)
    - [5.7.4 `eco_bindRobotInfo` / `eco_unbindRobotInfo`](#574-eco_bindrobotinfo--eco_unbindrobotinfo)
- [6. 学习路径建议](#6-学习路径建议)
- [7. 故障排查](#7-故障排查)
  - [7.1 `Connect()` 失败](#71-connect-失败)
  - [7.2 CLI 查不到命令](#72-cli-查不到命令)
  - [7.3 JSON 参数不知道怎么写](#73-json-参数不知道怎么写)
  - [7.4 任务失败](#74-任务失败)
  - [7.5 任务超时](#75-任务超时)
  - [7.6 安装问题](#76-安装问题)
  - [7.7 常用错误码速查](#77-常用错误码速查)

---

## 1. 概述

### 1.1 SDK 是什么

`bajie_sdk` 是八界机器人 Python 开发套件，以 `.whl` 包形式随机器人出厂提供。通过 SDK 可以：

- **控制机器人**：移动、导航、机械臂操作、抓取放置
- **感知环境**：开放词检测（OVD）、图像特征匹配、VLM 桌面整理
- **查询状态**：获取电量、位置、告警、地图、家具信息
- **CLI 快速调试**：通过命令行直接调用接口，查询字段模板

### 1.2 关键概念

| 项目 | 说明 |
|:---|:---|
| 当前版本 | **0.4.2**（Python 3.10，`cp310`） |
| 命名 | 包名 `bajie-sdk`，导入名 `bajie_sdk`，类名 `BajieRobot` |
| 默认机器人 IP | **`10.10.10.11`**（以屏幕底部 `http://10.10.10.11:17890` 显示的 IP 为准） |
| 默认连接地址 | `ws://10.10.10.11:9900/` |
| 环境变量 | `bajie_sdk_WS_URL` 设置默认 WebSocket 地址 |
| 构造参数 | `BajieRobot(ws_url="ws://<IP>:9900/")` 显式指定地址 |
| 安装来源 | 无 PyPI 公开包 —— 请使用交付 `.whl` 文件安装 |

### 1.3 建图前提

- **导航相关接口**（如 `eco_navigateToPoint`）推荐先建图。
- **语义操作**（如 `eco_navigateToSemanticArea`）需要自动建图完成，自动建图会建立语义地图，也可以在机器网页中编辑语义地图。
- **VLM 相关接口**（如 `eco_vlm_perception`）需要在机器配置网页中配置 VLM API KEY 后方可使用。

---

## 2. 准备工作

### 2.1 获取 whl 包

`bajie_sdk` 以 whl 包形式随机器人出厂提供（机器随包）。**当前发行要求 Python 3.10**（`cp310`）；**x86_64** 与 **arm64（aarch64）** 为不同 wheel，须按目标机器人 CPU 架构选用对应文件。

### 2.2 安装 SDK

**x86_64** 示例：

```bash
python3 -m pip install ./bajie_sdk-*-cp310-cp310-linux_x86_64.whl
```

**arm64（aarch64）** 示例（文件名以交付物为准，常见为 `linux_aarch64`）：

```bash
python3 -m pip install ./bajie_sdk-*-cp310-cp310-linux_aarch64.whl
```

`pip install` 会自动安装 `pyproject.toml` 中声明的依赖（如 `websocket-client`、`numpy`、`opencv-python` 等）。如需跳过依赖安装，加 `--no-deps` 参数：

```bash
python3 -m pip install --no-deps ./bajie_sdk-*-cp310-cp310-linux_x86_64.whl
```

> **建议使用虚拟环境**（可避免依赖冲突）：
> ```bash
> python3 -m venv .venv
> source .venv/bin/activate
> pip install ./bajie_sdk-*-cp310-cp310-linux_x86_64.whl
> ```

### 2.3 验证安装

```bash
python3 -c "import bajie_sdk; print('ok', bajie_sdk.__name__)"
```

看到 `ok bajie_sdk` 表示安装成功。

### 2.4 配置命令行补全（可选）

安装后运行以下命令，自动设置 `bajie_sdk` 命令别名与 Tab 补全：

```bash
python3 -m bajie_sdk completion
```

该命令自动将以下两行写入 `~/.bashrc`（或 `~/.zshrc`）：

```bash
alias bajie_sdk='python3 -m bajie_sdk'
source /path/to/installed/bajie_sdk/cli/completion.sh
```

之后即可用 `bajie_sdk run eco_<TAB>` 补全方法名、`--json` 模板等。

> 未配置补全时，直接使用完整命令 `python -m bajie_sdk xxx`，效果完全相同。

---

## 3. 快速开始

### 3.1 第一个程序：连接机器人并读取状态

默认连 **`10.10.10.11:9900`**（根据屏幕显示 IP 替换）；换地址用 **`BajieRobot(ws_url="ws://…")`** 或环境变量 **`bajie_sdk_WS_URL`**，再 **`Connect()`**。

```python
from bajie_sdk import BajieRobot
import math

robot = BajieRobot()  # 或 ws_url="ws://<IP>:9900/"
if not robot.Connect(timeout_sec=8.0):
    raise RuntimeError("连接失败")

info = robot.eco_robotInfo()
print("电量:", info.battery)
print("工作状态:", info.workState)
print("异常告警:", info.alarm)
print("地图坐标:", info.pos)

# 原地旋转一圈（360° = 2π rad），无需地图
st = robot.eco_moveChassis(move_distance=0.0, move_angle=2 * math.pi, timeout_sec=30.0)
print("旋转完成:", st.task_id, st.error_info.code)

robot.Disconnect()
```

### 3.2 常用任务链路

根据任务需要选用合适的链路：

| 场景 | 接口顺序 |
|:---|:---|
| 观测与检测 | `eco_captureImages` + `eco_detect_objects` |
| 对准与抓取 | `eco_lookto` + `eco_pick` |
| 跨视角匹配后抓取 | `eco_match_obj_views` + `eco_pick_with_arm_review` |
| 放置 | `eco_place_with_view` / `eco_place_3D` / `eco_place_in` |
| VLM 桌面整理 | `eco_vlm_perception` → `eco_vlm_desk_sort_plan` → `eco_vlm_match` |
| 整理效果评估 | `eco_vlm_judge` |
| 鞋子收纳规划 | `eco_detect_objects`(SHOE) → `eco_pairwise` → `eco_shoe_placement_plan` |
| PutWhere 示教放置 | `eco_put_where_summary` → `eco_place_in` |
| 图像特征匹配 | `eco_build_embeddings` + `eco_calc_similarity` |
| 语义地图查询 | `eco_robotFurniture` → `eco_manageSemanticMap(QUERY, fid)` → 增/删/改 |

### 3.3 抓取/放置流程

抓取/放置编排能力已统一收敛到 `BajieRobot` 的 `eco_*` 方法。

```python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest, OvdEndpoint

robot = BajieRobot()
if not robot.Connect(timeout_sec=8.0):
    raise RuntimeError("连接失败")

head_view = robot.eco_captureImages(CameraType.HEAD)
items = robot.eco_detect_objects(
    DetectObjectsRequest(
        rgb_image=head_view.rgb_image.img,
        labels=["纸巾", "玩具汽车"],   # labels 支持的物品标签见 §5.1.1
        entry=OvdEndpoint.OVD,
    )
).items
target = items[0].to_named_bbox_for_grab()

robot.eco_lookto(target.bbox, view=head_view, camera_type=CameraType.ARM, timeout_sec=30.0)
arm_view = robot.eco_captureImages(CameraType.ARM)
arm_items = robot.eco_detect_objects(
    DetectObjectsRequest(
        rgb_image=arm_view.rgb_image.img,
        labels=[target.name],
        entry=OvdEndpoint.OVD,
    )
).items
matched = robot.eco_match_obj_views(head_view.rgb_image.img, target.bbox, arm_view.rgb_image.img, arm_items)
robot.eco_pick(arm_view, matched, timeout_sec=120.0)
robot.eco_place_in(head_view, [153.0, 230.0, 403.0, 384.0], timeout_sec=120.0)

robot.Disconnect()
```

详细参数、返回值与异常处理以 §5 接口说明为准。

### 3.4 异步回调模式

所有阻塞类的 `eco_*` 方法（即返回 `MissionStatus` 或其子类的方法）均支持可选的 **`on_task_done`** 回调参数。

**适用场景**：当你不希望当前线程阻塞等待任务 finish 时，传入回调让 SDK 在收包线程中自动触发，同时本方法立即返回 `task_id`（类型 `str`）。

**使用方式**：

```python
from bajie_sdk import BajieRobot, CameraType, MissionStatus, RGBDViewWithPose, SemanticNavigationRequest

robot = BajieRobot()
if not robot.Connect(timeout_sec=8.0):
    raise RuntimeError("连接失败")

def on_images_ready(result: "RGBDViewWithPose") -> None:
    print(f"收到图像: task_id={result.rgb_image.img.shape}")

def on_nav_done(status: "MissionStatus") -> None:
    print(f"导航完成: task_id={status.task_id}, error={status.error_info}")

# 不传 on_task_done → 阻塞等待 finish
img = robot.eco_captureImages(CameraType.HEAD, timeout_sec=30.0)
print("阻塞模式返回 RGBDViewWithPose")

# 传入 on_task_done → 不阻塞，立即返回 task_id
task_id = robot.eco_captureImages(CameraType.HEAD, timeout_sec=30.0, on_task_done=on_images_ready)
print(f"回调模式立即返回 task_id: {task_id}")

# 导航任务也支持回调
robot.eco_navigateToSemanticArea(
    SemanticNavigationRequest(area_id="", area_name="客厅"),
    timeout_sec=120.0,
    on_task_done=on_nav_done,
)

import time; time.sleep(120)
robot.Disconnect()
```

**规则说明**：

| 条件 | 行为 |
|:---|:---|
| 不传 `on_task_done`（默认） | 阻塞等待 finish，返回结果对象（`MissionStatus` / `RGBDViewWithPose` / …） |
| 传入 `on_task_done` | 不阻塞，立即返回 `str`（task_id）；结果在回调中异步接收 |
| `timeout_sec` 在回调模式下仍有效 | 用于等待 mission **start 确认**（response），而非 finish |
| `on_task_done` 在 SDK 收包线程中调用 | 回调内勿执行耗时操作，以免影响收包 |
| 方法签名使用 `@typing.overload` | IDE 可在不传 `on_task_done` 时正确推断返回类型 |

**哪些方法支持回调**：§5.2~§5.5 中除链式复合方法（`eco_lookto`、`eco_pick_with_arm_review`、`eco_place_with_view`）和 `eco_cancelAllMissions` 外的所有 mission 类方法。感知类方法（§5.1）和状态查询类方法（§5.7）不支持回调。

### 3.5 连接事件与错误处理

`BajieRobot` 提供了连接状态事件回调与错误格式化工具：

```python
from bajie_sdk import BajieRobot, format_error_info

robot = BajieRobot(ws_url="ws://10.10.10.11:9900/")
robot.SetOnConnected(lambda: print("已连接"))
robot.SetOnError(lambda e: print(format_error_info(e)))

if not robot.Connect(timeout_sec=8.0):
    raise RuntimeError("连接失败")

# ... 执行任务 ...

robot.Disconnect()
```

- **`SetOnConnected(callback)`** — 注册连接成功回调（无参数）。
- **`SetOnError(callback)`** — 注册错误回调，参数为 error 对象。
- **`format_error_info(error)`** — 将 SDK 内部错误对象格式化为可读字符串，适合日志和排查。

---

## 4. CLI 使用说明

SDK 安装后自带命令行工具 `bajie_sdk`（与 `python -m bajie_sdk` 等价，见 §2.4），可用来查询接口、生成 JSON 模板、直接调用机器人接口。

### 4.1 查询与帮助

| 命令 | 作用 |
|:---|:---|
| `bajie_sdk -h` | 主帮助 |
| `bajie_sdk run -h` | run 子命令帮助（含 `--background` 后台模式说明） |
| `bajie_sdk -l` | 列出全部分类（一行一条） |
| `bajie_sdk -l 关键字` | 按分类标题筛选并列出相关 `eco_*` 及签名首行 |
| `bajie_sdk -m eco_xxx` | 输出单个方法的一行签名 + 完整 docstring |
| `bajie_sdk -t TypeName` | 输出类型的字段、枚举成员或构造函数签名 |

> 建议：当不确定参数字段时，优先用 `-m` + `-t` 组合查询。

### 4.2 执行单接口（run 子命令）

> 建议先设置环境变量，避免每次指定 `--ws-url`：
>
> ```bash
> export bajie_sdk_WS_URL=ws://10.10.10.11:9900/
> ```
>
> 也可写入 `~/.bashrc` 或 `~/.zshrc` 以持久生效。`run` 子命令的 `--ws-url` 参数优先于该环境变量。

```bash
bajie_sdk run eco_prepareRobotPose --json '{}' -o prepare_pose.json
```

**基本参数**：

| 参数 | 说明 |
|:---|:---|
| `method` | 方法名，如 `eco_startRecharge`（可省略 `eco_` 前缀） |
| `-j` / `--json` | 内联 JSON 对象，键名对应 `eco_*` 形参（含 `timeout_sec` 等关键字参数）。若第一个参数为 dataclass，可写 `"req": { ... }` 嵌套，也可将其字段**平铺**在顶层 |
| `-t` / `--json-template` | 不连接机器人，仅根据签名生成 JSON 模板到 stdout。<br>**枚举字段**输出成员 name 并在行尾 `#` 注释标注所有可选值（`Name(值)`），方便查阅 |
| `-o PATH` | 将返回值编码为 JSON 写入指定文件 |

**`--json-template` 示例**：

```bash
# 短标志 -t 等效
bajie_sdk run eco_captureImages -t
# 输出示例（枚举字段行尾 # 标注可选值）：
# {
#   "camera_type": "ARM",  # ARM(1) | HEAD(2)
#   "timeout_sec": 30.0
# }
```

### 4.3 内联引用与图片占位符

`--json` 字符串中支持 `@(路径)` 语法：

| 写法 | 效果 |
|:---|:---|
| `@(config.json)` | 嵌入整个 JSON 根对象 |
| `@(filter.json).object` | 读取文件后按 `.` 键链下钻 |
| `@(embs.json)[0]` | 读取 JSON 数组的第 0 项，可与键链组合，如 `@(resp.json).items[0].bbox` |
| `"img": "@(images/rgb.png)"` | 图片占位符，运行时加载为 ndarray（png/jpg/jpeg/webp/bmp） |

> 注意：图片路径**不要**写成 `@(图.png).a.b`；栅格图占位符不支持键链。相对路径相对于当前工作目录，可用 `--base-dir` 改变基准。

**`--json` 输入支持 `#` 行注释**（字符串内 `#` 原样保留，不影响解析）：

```bash
bajie_sdk run eco_captureImages --json '
{
    "camera_type": 2,      # 2=HEAD 头部相机
    "timeout_sec": 30.0
}
'
```

### 4.4 返回值编码与图床

返回的 ndarray 图像若面积 > 4096 像素，CLI 自动保存为 PNG 到 `-o` 同目录，JSON 中用 `@(文件名)` 引用：

```json
{"rgb_image": "@(rgb_image_1745712345678_1.png)", "depth_image": "@(depth_image_1745712345678_2.png)"}
```

PNG 命名格式为 `{字段名}_{时间戳}_{序号}.png`。

---

## 5. `BajieRobot.eco_*` 接口一览（全量）

本节按功能模块组织 SDK 所有公开接口。每个接口说明包括：完整签名、字段说明、返回值说明、Python 示例和 CLI 示例。

> **使用提示**：
> - 感知类接口（§5.1）通常不需要 `Connect()`，直接通过 HTTP 调用。
> - 导航/控制/状态类接口（§5.2~§5.7）需要先调用 `robot.Connect()`。
> - `timeout_sec` 单位为秒；`on_task_done` 支持阻塞/异步双模式（见 §3.4）。
> - **两级路径**：① 本体 Mission：`BajieRobot` → `missions` → WebSocket。② 感知 HTTP：`BajieRobot.eco_*` → HTTP，不经 WebSocket。

### 5.1 感知

#### 5.1.1 `eco_detect_objects`

**签名**（`bajie_robot.py`）：

```python
def eco_detect_objects(req: 'DetectObjectsRequest', *, timeout: 'float' = 10.0) -> 'DetectObjectsResponse'
```

**概述**：

开放词汇检测（OVD）：对 RGB 图做通用目标检测与属性查询。

`labels` 支持的物品标签：

- **家具**：床、其他床、婴儿床、床头柜、书柜、梳妆台、飘窗、衣柜、桌子、落地镜、沙发、贵妃椅沙发、其他沙发、单人沙发、茶几、电视柜、置物架、边几、长凳、椅子、柜子、其他柜子、文件柜、岛台、换鞋凳、落地灯、鞋柜、鞋架、收纳箱、收纳筐、普通门、塑料凳
- **家电**：洗衣机、冰箱、扫地机器人、基站、电视、音响
- **厨房用品**：碗、筷子、勺子、玻璃杯、盘子、菜刀、水果刀、餐刀、餐叉、杯子、马克杯
- **电子产品**：键盘、鼠标、耳机、显示器、笔记本电脑、电脑主机、摄像头、计算器、手机、平板电脑、游戏手柄、电源线、数据线、插排、插头、充电宝、相机、收音机
- **日用品**：纸巾、湿巾、脏衣篓、垃圾桶、笔筒、桌面收纳盒、绿植、花艺摆件、花瓶、台灯、书本、笔、日历、眼镜、纸团、烟灰缸、钥匙串、手办、鼠标垫、地毯、钱包、闹钟、雨伞、手机支架、摆件、笔袋、胶带、橡皮擦、文具盒、固体胶、扇子、卫生巾、袜子、手绢、抹布、鞋子
- **衣物**：团成团的衣物、衣服、裤子
- **玩具**：玩具汽车、玩具球、玩具恐龙、玩具兔子、玩具小熊、玩具枪、玩具飞机、玩具滑板、玩具挖土车、玩具吊车、其他玩具、玩具积木、玩偶、气球、拨浪鼓、风车、玩具胡萝卜、玩具火车、魔方、棋盘、城堡模型、陀螺、抓娃娃机、玩具卡片
- **宠物用品**：猫砂盆、猫爬架
- **食品**：零食、瓶子
- **水果**：香蕉、苹果、西瓜、菠萝、橙子、梨、葡萄、草莓、樱桃、芒果、桃子、橘子、柚子、枇杷、火龙果、哈密瓜
- **其他**：蔬菜、易拉罐、洗手池、订书机、眼镜盒

`SHOE` 模型提供有限鞋子识别，扩展需要自定义训练上传权重。`entry` 参数选择 `OvdEndpoint.OVD`（默认）或 `OvdEndpoint.SHOE`。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | 请求体 `DetectObjectsRequest`，字段含义如下： |
| `timeout` | HTTP 请求超时（秒）。 |

**「`req`」子字段**：

| 字段 | 说明 |
|:---|:---|
| `rgb_image` | RGB 图像，一般为 `numpy.ndarray` (H×W×C)； |
| `labels` | 开放词标签字符串列表；非支持父类标签（如 `"玩具"`）自动展开为子类列表； |
| `position_region` | 可选，语义区域名称（如 `"客厅"`），检测范围限定到该区域； |
| `payload` | 可选，承载物名称（如 `"桌子"`），配合 `position_region` 进一步限定检测范围； |
| `ovd_property` | `OvdQueryProperty`（color / shape / person 等查询属性）； |
| `ovd_obj_thresh` | OVD 得分阈值； |
| `box_obj_thresh` | 框得分阈值； |
| `entry` | `OvdEndpoint`（`OVD` 或 `SHOE`）决定服务端的子路径。 |

**返回值**：

`DetectObjectsResponse`：`rgb_image`；`items`、`items_unknow` 为解析后的 `ObjectDetection` 列表。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest, OvdEndpoint

robot = BajieRobot("10.88.41.120", auto_connect=True)
img = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
req = DetectObjectsRequest(rgb_image=img, labels=["鞋子"], entry=OvdEndpoint.SHOE)
resp = robot.eco_detect_objects(req, timeout=15.0)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o img.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(img.json).rgb_image.img, "labels": ["鞋子"], "entry": "SHOE"}, "timeout": 15.0}' -o resp.json
```

#### 5.1.2 `eco_build_embeddings`

**签名**（`bajie_robot.py`）：

```python
def eco_build_embeddings(images_rgb: 'Sequence[Union[np.ndarray, str]]', *, timeout: 'float' = 10.0) -> 'List[List[float]]'
```

**概述**：

生成特征向量（embedding）：批量传入图片，返回对应的特征向量列表，每张图一个向量。得到的向量可传给 `eco_calc_similarity` 做相似度比较。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `images_rgb` | 多张 RGB 图片，每张可以是 `numpy.ndarray` 或 PNG base64 字符串。 |
| `timeout` | 超时（秒）。 |

**返回值**：

列表，每项是与 `images_rgb` 顺序对应的特征向量（`List[float]`）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
img = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
vecs = robot.eco_build_embeddings([img])
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o img.json
## 第2步：eco_build_embeddings
bajie_sdk run eco_build_embeddings --json '{"images_rgb": [@(img.json).rgb_image.img]}' -o vecs.json
```

#### 5.1.3 `eco_calc_similarity`

**签名**（`bajie_robot.py`）：

```python
def eco_calc_similarity(req: 'CalcSimilarityRequest') -> 'float'
```

**概述**：

计算相似度：比较两张图片或两组特征向量（embedding）的相似程度，返回 0~1 之间的分数。

> 注意：本方法的 timeout 设置在 `CalcSimilarityRequest` 内部而非关键字参数。每次调用时 A 路和 B 路必须各自至少填一个，不能全为空。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | `CalcSimilarityRequest` |

**「`req`」子字段**：

| 字段 | 说明 |
|:---|:---|
| `image_a_base64` | 可选，图片 A 的 PNG base64 字符串； |
| `image_b_base64` | 可选，图片 B 的 PNG base64 字符串； |
| `embedding_a` | 可选，图片 A 的特征向量（由 `eco_build_embeddings` 得到）； |
| `embedding_b` | 可选，图片 B 的特征向量； |
| `timeout` | 超时（秒）。 |

**返回值**：

`float` 相似度（值越接近 1 表示越相似）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, CalcSimilarityRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_view = robot.eco_captureImages(CameraType.HEAD)
arm_view = robot.eco_captureImages(CameraType.ARM)
embs = robot.eco_build_embeddings([head_view.rgb_image.img, arm_view.rgb_image.img])
s = robot.eco_calc_similarity(CalcSimilarityRequest(embedding_a=embs[0], embedding_b=embs[1]))
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o arm_view.json
## 第3步：eco_build_embeddings
bajie_sdk run eco_build_embeddings --json '{"images_rgb": [@(head_view.json).rgb_image.img, @(arm_view.json).rgb_image.img]}' -o embs.json
## 第4步：eco_calc_similarity
bajie_sdk run eco_calc_similarity --json '{"req": {"embedding_a": @(embs.json)[0], "embedding_b": @(embs.json)[1]}}' -o s.json
```

#### 5.1.4 `eco_pairwise`

**签名**（`bajie_robot.py`）：

```python
def eco_pairwise(frames: 'Sequence[tuple[Union[np.ndarray, str], Sequence[ObjectDetection]]]', *, timeout: 'float' = 15.0) -> 'List[tuple[str, str]]'
```

**概述**：

跨帧物体配对：将多帧中的检测结果按视觉相似性关联，同一物理物体在不同帧中会被分配相同的 ``pair_id``。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `frames` | 多帧序列，每帧为 ``(image, objs)`` 元组： |
| `timeout` | 超时（秒）。 |

**「`frames`」子字段**：

| 字段 | 说明 |
|:---|:---|
| `image` | RGB ``ndarray`` 或 PNG base64，**仅为匹配算法提供视觉特征，不直接参与配对**； |
| `objs` | 该帧中待配对的 ``ObjectDetection`` 列表（OVD 检测结果）。 |
| 备注 | **配对的核心是物体**：不同帧中视觉相似的物体会被归为同一对，允许多张图片的物体混合匹配。 |

**返回值**：

``(pair_id, item_id)`` 元组列表，每个元组将输入中的某个检测项与一个跨帧配对关联起来。
    ``pair_id`` 是服务端生成的跨帧唯一配对标识，同一物体在不同帧的检测项共享同一个 ``pair_id``；
    ``item_id`` 即对应 ``ObjectDetection.uuid``，可直接用其反查输入 ``objs`` 中的原始检测项。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest, OvdEndpoint

robot = BajieRobot("10.88.41.120", auto_connect=True)
view = robot.eco_captureImages(CameraType.HEAD)
objs = list(robot.eco_detect_objects(
    DetectObjectsRequest(rgb_image=view.rgb_image.img, labels=["鞋子"], entry=OvdEndpoint.SHOE),
).items)
links = robot.eco_pairwise([(view.rgb_image.img, objs)])
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o view.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(view.json).rgb_image.img, "labels": ["鞋子"], "entry": "SHOE"}}' -o objs.json
## 第3步：eco_pairwise
bajie_sdk run eco_pairwise --json '{"frames": [[@(view.json).rgb_image.img, @(objs.json).items]]}' -o links.json
```

#### 5.1.5 `eco_shoe_placement_plan`

**签名**（`bajie_robot.py`）：

```python
def eco_shoe_placement_plan(view: 'RGBDViewWithPose', place_region: 'Sequence[Sequence[float]]', front_edge: 'Sequence[Sequence[float]]', paired_bbox: 'Sequence[PairedBBox]', *, move_all: 'bool' = True, timeout: 'float' = 20.0) -> 'List[MovePlanStep]'
```

**概述**：

根据鞋子检测框与放置区域，生成鞋子摆放规划。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `view` | `RGBDViewWithPose` 序列。 |
| `place_region` | 放置区多边形顶点序列（地图坐标系，3D 点列表）。 |
| `front_edge` | 正向边折线，是place_region的一部分（导航点将在该边一侧前方生成）。 |
| `paired_bbox` | `PairedBBox` 列表（`bbox`、`pair_id`、可选 `is_tilted`）。 |
| `move_all` | `True` 所有鞋子都规划整理，`False` 符合条件的不整理。 |
| `timeout` | 超时（秒），同时作用于投影和摆放规划。 |

**返回值**：

`MovePlanStep` 列表（`step_id`、`from_bbox`、`to_bbox`）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest, OvdEndpoint, PairedBBox

robot = BajieRobot("10.88.41.120", auto_connect=True)
view = robot.eco_captureImages(CameraType.HEAD)
obs = robot.eco_detect_objects(
    DetectObjectsRequest(rgb_image=view.rgb_image.img, labels=["鞋子"], entry=OvdEndpoint.SHOE),
)
items = [PairedBBox(bbox=(0,0,0,0), pair_id=0)]

if obs.items:
    steps = robot.eco_shoe_placement_plan(
        view,
        [[1.2, 0.6, 0.0], [1.2, -0.6, 0.0], [0.4, -0.6, 0.0], [0.4, 0.6, 0.0]],
        [[0.4, -0.6, 0.0], [0.4, 0.6, 0.0]],
        items, timeout=15.0,
    )
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o view.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(view.json).rgb_image.img, "labels": ["鞋子"], "entry": "SHOE"}}' -o obs.json
## 第3步：eco_shoe_placement_plan
bajie_sdk run eco_shoe_placement_plan --json '{"view": @(view.json), "place_region": [[1.2, 0.6, 0.0], [1.2, -0.6, 0.0], [0.4, -0.6, 0.0], [0.4, 0.6, 0.0]], "front_edge": [[0.4, -0.6, 0.0], [0.4, 0.6, 0.0]], "paired_bbox": "items", "timeout": 15.0}' -o steps.json
```

#### 5.1.6 `eco_vlm_desk_sort_plan`

**签名**（`bajie_robot.py`）：

```python
def eco_vlm_desk_sort_plan(req: 'VlmPlanRequest', *, timeout: 'float' = 10.0) -> 'VlmPlanResponse'
```

**概述**：

VLM 桌面整理规划：基于图片、用户意图与感知结果生成逐步抓放计划。`judge_input` 与 `memory_input` 为可选输入，分别用于整理评估结果和用户历史偏好等记忆信息。

> VLM 类接口需在 Bajie Robot Web 配置页面中设置 VLM 模型 API 后方可使用。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | 请求体 `VlmPlanRequest` |
| `timeout` | 超时（秒）。 |

**「`req`」子字段**：

| 字段 | 说明 |
|:---|:---|
| `image` | RGB 图像（`numpy.ndarray`）； |
| `user_input` | 用户意图描述； |
| `perception` | 感知结果列表（`Sequence[NamedBBox]`）； |
| `judge_input` | 可选，整理效果评估输入； |
| `memory_input` | 可选，记忆输入。 |

**返回值**：

`VlmPlanResponse`：`user_input` 回显、`reason` 推理过程、`steps` 逐步抓放步骤列表。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, VlmPlanRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
img = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
items = robot.eco_vlm_perception(img)
result = robot.eco_vlm_desk_sort_plan(
    VlmPlanRequest(image=img, user_input="整理桌面", perception=items)
)
print(result.reason)
for step in result.steps:
    print(step.step_id, step.from_bbox, step.to_bbox)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o img.json
## 第2步：eco_vlm_perception
bajie_sdk run eco_vlm_perception --json '{"image": @(img.json).rgb_image.img}' -o items.json
## 第3步：eco_vlm_desk_sort_plan
bajie_sdk run eco_vlm_desk_sort_plan --json '{"req": {"image": @(img.json).rgb_image.img, "user_input": "整理桌面", "perception": @(items.json)}}' -o result.json
```

#### 5.1.7 `eco_vlm_perception`

**签名**（`bajie_robot.py`）：

```python
def eco_vlm_perception(image: 'np.ndarray', *, timeout: 'float' = 10.0) -> 'List[NamedBBox]'
```

**概述**：

VLM 桌面物品感知：输入桌面图片，输出物品 `name` 与 `bbox` 列表。

> VLM 类接口需在 Bajie Robot Web 配置页面中设置 VLM 模型 API 后方可使用。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `image` | RGB `ndarray`。 |
| `timeout` | 超时（秒）。 |

**返回值**：

`NamedBBox` 列表（可传入 `eco_vlm_desk_sort_plan` 作为 `perception` 参数）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
img = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
items = robot.eco_vlm_perception(img)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o img.json
## 第2步：eco_vlm_perception
bajie_sdk run eco_vlm_perception --json '{"image": @(img.json).rgb_image.img}' -o items.json
```

#### 5.1.8 `eco_vlm_match`

**签名**（`bajie_robot.py`）：

```python
def eco_vlm_match(req: 'DeskIntentMatchRequest', *, timeout: 'float' = 10.0) -> 'tuple[int, int, int, int]'
```

**概述**：

VLM 桌面匹配：头部相机中的物体框映射到手部相机图像中的框。

> VLM 类接口需在 Bajie Robot Web 配置页面中设置 VLM 模型 API 后方可使用。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | 结构体参数：`arm_image`（手臂 RGB）、`base_image`（头部 RGB）、`bbox`（头部图 `[x1,y1,x2,y2]`）。 |
| `timeout` | 超时（秒）。 |

**返回值**：

手部相机下的 `(x1, y1, x2, y2)` 像素框。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, DeskIntentMatchRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_view = robot.eco_captureImages(CameraType.HEAD)
arm_view = robot.eco_captureImages(CameraType.ARM)
bbox = robot.eco_vlm_match(
    DeskIntentMatchRequest(arm_image=arm_view.rgb_image.img, base_image=head_view.rgb_image.img, bbox=(100, 50, 300, 250)),
    timeout=15.0,
)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o arm_view.json
## 第3步：eco_vlm_match
bajie_sdk run eco_vlm_match --json '{"req": {"arm_image": @(arm_view.json).rgb_image.img, "base_image": @(head_view.json).rgb_image.img, "bbox": [100, 50, 300, 250]}, "timeout": 15.0}' -o bbox.json
```

#### 5.1.9 `eco_vlm_suggest_angle`

**签名**（`bajie_robot.py`）：

```python
def eco_vlm_suggest_angle(image_angles: 'Sequence[tuple[np.ndarray, float]]', *, timeout: 'float' = 10.0) -> 'float'
```

**概述**：

VLM 头部观测角度推荐：输入多角度桌面图片及对应摄像头角度，输出视野最佳的头部角度。

> VLM 类接口需在 Bajie Robot Web 配置页面中设置 VLM 模型 API 后方可使用。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `image_angles` | `[(image, angle), ...]`，每项为 `(RGB ndarray, 头部角度弧度)`。 |
| `timeout` | 超时（秒）。 |

**返回值**：

推荐头部角度（弧度）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
view1 = robot.eco_captureImages(CameraType.HEAD)
robot.eco_setRobotHead(0.8)
view2 = robot.eco_captureImages(CameraType.HEAD)
angle = robot.eco_vlm_suggest_angle([
    (view1.rgb_image.img, 0.3),
    (view2.rgb_image.img, 0.8),
])
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o view1.json
## 第2步：eco_setRobotHead
bajie_sdk run eco_setRobotHead --json '{"value": 0.8}'
## 第3步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o view2.json
## 第4步：eco_vlm_suggest_angle
bajie_sdk run eco_vlm_suggest_angle --json '{"image_angles": [[@(view1.json).rgb_image.img, 0.3], [@(view2.json).rgb_image.img, 0.8]]}' -o angle.json
```

#### 5.1.10 `eco_vlm_judge`

**签名**（`bajie_robot.py`）：

```python
def eco_vlm_judge(tidy_image: 'np.ndarray', user_input: 'str', *, timeout: 'float' = 10.0) -> 'VlmJudgeResponse'
```

**概述**：

VLM 整理效果评估：输入整理后的桌面图片与用户需求，判断是否符合需求。

> VLM 类接口需在 Bajie Robot Web 配置页面中设置 VLM 模型 API 后方可使用。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `tidy_image` | RGB `ndarray`（整理后重新拍摄的桌面图）。 |
| `user_input` | 用户需求语句（应与 `eco_vlm_desk_sort_plan` 中的一致）。 |
| `timeout` | 超时（秒）。 |

**返回值**：

`VlmJudgeResponse`：
- `is_tidy` — 是否符合整理需求（`bool`）；
- `reason` — 不符合原因或评价说明（`str`）；
- `score` — 整理后状态评分 0~1（`float`）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
tidy_img = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
result = robot.eco_vlm_judge(tidy_img, "整理桌面")
if not result.is_tidy:
    print(f"需进一步整理: {result.reason}")
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o tidy_img.json
## 第2步：eco_vlm_judge
bajie_sdk run eco_vlm_judge --json '{"tidy_image": @(tidy_img.json).rgb_image.img, "user_input": "整理桌面"}' -o result.json
```

#### 5.1.11 `eco_put_where_summary`

**签名**（`bajie_robot.py`）：

```python
def eco_put_where_summary(req: 'PutWhereSummaryRequest', *, timeout: 'float' = 30.0) -> 'List[float]'
```

**概述**：

示教 PutWhere 摘要。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | `PutWhereSummaryRequest` |
| `timeout` | 超时（秒）。 |

**「`req`」子字段**：

| 字段 | 说明 |
|:---|:---|
| `carrier` | 载体名称； |
| `carrier_direct` | 载体朝向描述； |
| `image` | 场景图 `numpy.ndarray`； |
| `obj_image` | 目标图 `numpy.ndarray`； |
| `placeholder` | 占位描述字符串； |
| `summary` | 任务描述。 |

**返回值**：

推荐放置像素坐标：长度 **4**（carrier 框）或 **2**（单点）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, PutWhereSummaryRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
scene = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
obj = robot.eco_captureImages(CameraType.ARM).rgb_image.img
r = robot.eco_put_where_summary(PutWhereSummaryRequest(
    carrier="桌子", carrier_direct="前", image=scene, obj_image=obj,
    placeholder="桌子", summary="桌面整理",
))
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o scene.json
## 第2步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o obj.json
## 第3步：eco_put_where_summary
bajie_sdk run eco_put_where_summary --json '{"req": {"carrier": "桌子", "carrier_direct": "前", "image": @(scene.json).rgb_image.img, "obj_image": @(obj.json).rgb_image.img, "placeholder": "桌子", "summary": "桌面整理"}}' -o r.json
```

### 5.2 建图与导航

本节接口需要先 `robot.Connect()`。

#### 5.2.1 `eco_autoMapBuild`

**签名**（`bajie_robot.py`）：

```python
def eco_autoMapBuild(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

自动建图（全自动 SLAM + 语义建图）。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `timeout_sec` | 等待任务结束（`finish`）的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `MissionStatus`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, Command
import time

robot = BajieRobot("10.88.41.120", auto_connect=True)
task_id = robot.eco_autoMapBuild(timeout_sec=120.0, on_task_done=lambda _: None)
time.sleep(10)
robot.eco_missionControl(Command.CANCEL, "eco_autoMapBuild", task_id)
```

**CLI 示例**：

```bash
## 第1步：eco_autoMapBuild
bajie_sdk run eco_autoMapBuild --json '{"timeout_sec": 120.0}' --background -o task_id.json
sleep 10
## 第2步：eco_missionControl
bajie_sdk run eco_missionControl --json '{"cmd": "CANCEL", "task_id": @(task_id.json), "arg2": @(task_id.json)}'
```

#### 5.2.2 `eco_semanticMapBuild`

**签名**（`bajie_robot.py`）：

```python
def eco_semanticMapBuild(area_info: 'Sequence[AreaInfo]', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

语义建图。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `area_info` | `AreaInfo` 序列，每项含 `area_id`、`area_name`。 |
| `timeout_sec` | 等待 `finish` 的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `MissionStatus`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, AreaInfo

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_semanticMapBuild([AreaInfo(area_id="r1", area_name="客厅")])
```

**CLI 示例**：

```bash
bajie_sdk run eco_semanticMapBuild --json '{"area_info": [{"area_id": "r1", "area_name": "客厅"}]}' -o st.json
```

#### 5.2.3 `eco_manageSemanticMap`

**签名**（`bajie_robot.py`）：

```python
def eco_manageSemanticMap(cmd: 'Union[SemanticMapManagerCmd, int]', object_info: 'SemanticMapManagerObjectInfo', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[SemanticMapManagerResponse], None]]' = None) -> 'Union[SemanticMapManagerResponse, str]'
```

**概述**：

语义地图增删改查。

> 注意：
> 1. ADD 时 id 为新 UUID（添加成功后该 id 会出现在 eco_robotFurniture 列表中）。
> 2. QUERY / MODIFY / DELETE 时 `SemanticMapManagerObjectInfo.id` 取值来源为 `eco_robotFurniture()` 返回的 `FurnitureItem.fid`。QUERY 时可空（查全部）或填入 fid（查单条）。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `cmd` | `SemanticMapManagerCmd`（如 `ADD` / `QUERY` / `MODIFY` / `DELETE` 等）或与枚举兼容的整型。 |
| `object_info` | `SemanticMapManagerObjectInfo`（`id`, `model_level`, `mssid`, `model_name`, `name`, `content`, `direction`, `app_is_custom`）。 |
| `timeout_sec` | 等待应答/结束的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `SemanticMapManagerResponse`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import (
    BajieRobot, SemanticMapManagerCmd,
    SemanticMapManagerObjectInfo, SemanticMapModelLevel,
)

robot = BajieRobot("10.88.41.120", auto_connect=True)

# 查询全部（id 留空）
resp = robot.eco_manageSemanticMap(SemanticMapManagerCmd.QUERY, SemanticMapManagerObjectInfo())
print(resp.objects_info)

# 按 fid 查询
fid = robot.eco_robotFurniture().info[0].fid
resp = robot.eco_manageSemanticMap(SemanticMapManagerCmd.QUERY, SemanticMapManagerObjectInfo(id=fid))

# 添加新家具
import uuid
robot.eco_manageSemanticMap(
    SemanticMapManagerCmd.ADD,
    SemanticMapManagerObjectInfo(
        id=str(uuid.uuid4()), model_level=SemanticMapModelLevel.CARRIER,
        model_name="桌子", direction=[[1.0, 0.0], [1.0, 1.0]],
        content=[[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]],
        name="书桌",
    ),
)
```

**CLI 示例**：

```bash
## 查询全部
bajie_sdk run eco_manageSemanticMap --json '{"cmd": "QUERY", "object_info": {}}' -o resp.json
## 按 fid 查询
bajie_sdk run eco_manageSemanticMap --json '{"cmd": "QUERY", "object_info": {"id": "fid"}}' -o resp.json
## 添加新家具
bajie_sdk run eco_manageSemanticMap --json '{"cmd": "ADD", "object_info": {"id": {}, "model_level": "CARRIER", "model_name": "桌子", "direction": [[1.0, 0.0], [1.0, 1.0]], "content": [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]], "name": "书桌"}}'
```

#### 5.2.4 `eco_moveChassis`

**签名**（`bajie_robot.py`）：

```python
def eco_moveChassis(move_distance: 'float', move_angle: 'float', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

底盘相对运动。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `move_distance` | 平移距离 (m)。 |
| `move_angle` | 旋转角 (rad)。 |
| `timeout_sec` | 等待 `finish` 的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `MissionStatus`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_moveChassis(0.1, 0.0)  # 约前进 10 cm
```

**CLI 示例**：

```bash
bajie_sdk run eco_moveChassis --json '{"move_distance": 0.1, "move_angle": 0.0}' -o st.json
```

#### 5.2.5 `eco_navigateToPoint`

**签名**（`bajie_robot.py`）：

```python
def eco_navigateToPoint(req: 'PointNavigationRequest', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

地图坐标系下的定点导航。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | `PointNavigationRequest`（`x`, `y`, `yaw`） |
| `timeout_sec` | 等待 `finish` 的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `MissionStatus`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, PointNavigationRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_navigateToPoint(PointNavigationRequest(x=1.0, y=2.0, yaw=0.0))
```

**CLI 示例**：

```bash
bajie_sdk run eco_navigateToPoint --json '{"req": {"x": 1.0, "y": 2.0, "yaw": 0.0}}' -o st.json
```

#### 5.2.6 `eco_navigateToSemanticArea`

**签名**（`bajie_robot.py`）：

```python
def eco_navigateToSemanticArea(req: 'SemanticNavigationRequest', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

语义区域导航。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | `SemanticNavigationRequest`（`area_id`, `area_name`） |
| `timeout_sec` | 等待 `finish` 的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `MissionStatus`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, SemanticNavigationRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_navigateToSemanticArea(
    SemanticNavigationRequest(area_id="", area_name="客厅")
)
```

**CLI 示例**：

```bash
bajie_sdk run eco_navigateToSemanticArea --json '{"req": {"area_id": "", "area_name": "客厅"}}' -o st.json
```

#### 5.2.7 `eco_leaveDock`

**签名**（`bajie_robot.py`）：

```python
def eco_leaveDock(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

下桩（离开充电桩）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_leaveDock()
```

**CLI 示例**：

```bash
bajie_sdk run eco_leaveDock --json '{"timeout_sec": 30.0}' -o st.json
```

#### 5.2.8 `eco_relocate`

**签名**（`bajie_robot.py`）：

```python
def eco_relocate(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

重定位。

**Python 示例**：

```python
from bajie_sdk import BajieRobot
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_relocate()
```

**CLI 示例**：

```bash
bajie_sdk run eco_relocate --json '{"timeout_sec": 30.0}' -o st.json
```

#### 5.2.9 `eco_startRecharge`

**签名**（`bajie_robot.py`）：

```python
def eco_startRecharge(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

回充。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, Command
import time

robot = BajieRobot("10.88.41.120", auto_connect=True)
task_id = robot.eco_startRecharge(timeout_sec=300.0, on_task_done=lambda _: None)
time.sleep(10)
robot.eco_missionControl(Command.CANCEL, "eco_startRecharge", task_id)
```

**CLI 示例**：

```bash
## 第1步：eco_startRecharge
bajie_sdk run eco_startRecharge --json '{"timeout_sec": 300.0}' --background -o task_id.json
sleep 10
## 第2步：eco_missionControl
bajie_sdk run eco_missionControl --json '{"cmd": "CANCEL", "task_id": @(task_id.json), "arg2": @(task_id.json)}'
```

### 5.3 图像与位姿

本节接口需要先 `robot.Connect()`。

#### 5.3.1 `eco_captureImages`

**签名**（`bajie_robot.py`）：

```python
def eco_captureImages(camera_type: 'Union[CameraType, int]', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[RGBDViewWithPose], None]]' = None) -> 'Union[RGBDViewWithPose, str]'
```

**概述**：

获取一帧相机图像。`camera_type`：`CameraType.ARM`(1) 为手臂相机、`CameraType.HEAD`(2) 为头部相机。
> ``RGBDViewWithPose.tf_goal`` 坐标系取决于相机类型：
> **头部相机 (HEAD)** 的 ``tf_goal`` 在 ``map`` 坐标系下；
> **手臂相机 (ARM)** 的 ``tf_goal`` 在 ``base_footprint`` 坐标系下。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `camera_type` | `CameraType` 或与枚举兼容的整型。 |
| `timeout_sec` | 等待图像 `finish` 的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `RGBDViewWithPose`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
img = robot.eco_captureImages(CameraType.ARM)
```

**CLI 示例**：

```bash
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o img.json
```

#### 5.3.2 `eco_computeObjectPose`

**签名**（`bajie_robot.py`）：

```python
def eco_computeObjectPose(named_bbox: 'Union[Sequence[NamedBBox], Sequence[tuple[int, int, int, int]]]', view: 'RGBDViewWithPose', *, timeout_sec: 'float' = 6.0, on_task_done: 'Optional[Callable[[List[ObjectPose3D]], None]]' = None) -> 'Union[List[ObjectPose3D], str]'
```

**概述**：

根据检测框与 RGBD 及地图系位姿，计算框内物体 6D pose。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `named_bbox` | `NamedBBox` 列表，或 `list[tuple[int,int,int,int]]`。 |
| `view` | `RGBDViewWithPose`（`rgb_image` / `depth_image` / `tf_goal`）。 |
| `timeout_sec` | 等待含 `pose_results` 的 `finish` 的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `list[ObjectPose3D]`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, NamedBBox

robot = BajieRobot("10.88.41.120", auto_connect=True)
view = robot.eco_captureImages(CameraType.ARM)
poses = robot.eco_computeObjectPose(
    named_bbox=[NamedBBox(id=0, name="obj", bbox=(10, 20, 100, 200))],
    view=view, timeout_sec=5.0,
)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o view.json
## 第2步：eco_computeObjectPose
bajie_sdk run eco_computeObjectPose --json '{"named_bbox": [{"id": 0, "name": "obj", "bbox": [10, 20, 100, 200]}], "view": @(view.json), "timeout_sec": 5.0}' -o poses.json
```

### 5.4 高层任务

本节接口需要先 `robot.Connect()`。

#### 5.4.1 `eco_locatePerson`

**签名**（`bajie_robot.py`）：

```python
def eco_locatePerson(req: 'FindPersonRequest', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

在指定区域找人。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | `FindPersonRequest`（`user_name`, `user_id`）。 |
| `timeout_sec` | 等待 `finish` 的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `MissionStatus`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, FindPersonRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_locatePerson(FindPersonRequest(user_name="张三"))
```

**CLI 示例**：

```bash
bajie_sdk run eco_locatePerson --json '{"req": {"user_name": "张三"}}' -o st.json
```

#### 5.4.2 `eco_robotSpeech`

**签名**（`bajie_robot.py`）：

```python
def eco_robotSpeech(text: 'str') -> 'str'
```

**概述**：

App 语音播报（TTS），fire-and-forget 模式。

发送播报请求后立即返回，不等待服务端回复。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `text` | 播报内容。 |

**返回值**：

task_id（格式 ``speech_<uuid_hex>``），用于追踪本次播报请求。
:raises RuntimeError: 发送失败时抛出。

**Python 示例**：

```python
from bajie_sdk import BajieRobot

robot = BajieRobot("10.88.41.120", auto_connect=True)
tid = robot.eco_robotSpeech("请让一让")
```

**CLI 示例**：

```bash
bajie_sdk run eco_robotSpeech --json '{"text": "请让一让"}' -o tid.json
```

#### 5.4.3 `eco_findObject`

**签名**（`bajie_robot.py`）：

```python
def eco_findObject(req: 'SearchRequest', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[SearchResponse], None]]' = None) -> 'Union[SearchResponse, str]'
```

**概述**：

在区域内搜索物体；可在 `filter_boxes` 中传入收纳筐 6D 框以排除筐内已收纳物体。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | `SearchRequest`：`object`(`ObjectInfo`)、`area`(`AreaInfo`)、`filter_boxes`(可选 `ObjectPose3D` 列表)。 |
| `timeout_sec` | 等待搜索结果 `finish` 的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `SearchResponse`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, SearchRequest, ObjectInfo, AreaInfo

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_findObject(
    SearchRequest(object=ObjectInfo(obj_name="玩具"), area=AreaInfo(area_name="玩具收纳区"))
)
```

**CLI 示例**：

```bash
bajie_sdk run eco_findObject --json '{"req": {"object": {"obj_name": "玩具"}, "area": {"area_name": "玩具收纳区"}}}' -o st.json
```

#### 5.4.4 `eco_prepareRobotPose`

**签名**（`bajie_robot.py`）：

```python
def eco_prepareRobotPose(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

机身进入任务准备姿态（开始任务时调用）。

**Python 示例**：

```python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_prepareRobotPose(timeout_sec=60.0)
```

**CLI 示例**：

```bash
bajie_sdk run eco_prepareRobotPose --json '{"timeout_sec": 60.0}' -o st.json
```

#### 5.4.5 `eco_finishRobotPose`

**签名**（`bajie_robot.py`）：

```python
def eco_finishRobotPose(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

机身恢复结束姿态（结束任务时调用）。

**Python 示例**：

```python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_finishRobotPose()
```

**CLI 示例**：

```bash
bajie_sdk run eco_finishRobotPose --json '{"timeout_sec": 30.0}' -o st.json
```

#### 5.4.6 `eco_lookto`

**签名**（`bajie_robot.py`）：

```python
def eco_lookto(req: 'Union[ObjectPose3D, tuple[int, int, int, int]]', *, view: 'Optional[RGBDViewWithPose]' = None, camera_type: 'Union[CameraType, int]' = <CameraType.ARM: 1>, timeout_sec: 'float' = 30.0) -> 'MissionStatus'
```

**概述**：

观测对准：统一手部 / 头部 `look to` 能力。传 bbox 时需配合 `view`，SDK 会先用 `eco_computeObjectPose` 求位姿（内部固定超时 6 秒）。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | `ObjectPose3D` 或单 bbox `(x1, y1, x2, y2)`。 |
| `view` | 当 `req` 为 bbox 时必填，对应求姿态所需 RGBD 视角。 |
| `camera_type` | 观测相机类型；`CameraType.ARM` 走手部观测，`CameraType.HEAD` 走头部观测。 |
| `timeout_sec` | 等待 `finish` 的最长秒数。 |

**返回值**：

`MissionStatus` 任务状态。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_view = robot.eco_captureImages(CameraType.HEAD)
st = robot.eco_lookto((120, 80, 260, 300), view=head_view, camera_type=CameraType.ARM, timeout_sec=30.0)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_lookto
bajie_sdk run eco_lookto --json '{"req": [120, 80, 260, 300], "view": @(head_view.json), "camera_type": "ARM", "timeout_sec": 30.0}' -o st.json
```

#### 5.4.7 `eco_pick`

**签名**（`bajie_robot.py`）：

```python
def eco_pick(view: 'RGBDViewWithPose', bbox: 'NamedBBox', *, timeout_sec: 'float' = 300.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

抓取：执行精准抓取。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `view` | 手臂相机 `RGBDViewWithPose`（rgb/depth/tf_goal）。 |
| `bbox` | `NamedBBox`（`name`、四角像素坐标等）。 |
| `timeout_sec` | 等待 `finish` 的最长秒数（抓取可能较慢）。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `MissionStatus`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
arm_view = robot.eco_captureImages(CameraType.ARM)
items = list(robot.eco_detect_objects(
    DetectObjectsRequest(rgb_image=arm_view.rgb_image.img, labels=["玩具"]),
).items)
if items:
    st = robot.eco_pick(arm_view, items[0].to_named_bbox_for_grab(), timeout_sec=300.0)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o arm_view.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(arm_view.json).rgb_image.img, "labels": ["玩具"]}}' -o items.json
## 第3步：eco_pick
bajie_sdk run eco_pick --json '{"view": @(arm_view.json), "bbox": @(items.json).items[0], "timeout_sec": 300.0}' -o st.json
```

#### 5.4.8 `eco_match_obj_views`

**签名**（`bajie_robot.py`）：

```python
def eco_match_obj_views(source_img: 'np.ndarray', ref_bbox: 'tuple[int, int, int, int]', target_img: 'np.ndarray', target_detections: 'Sequence[ObjectDetection]') -> 'NamedBBox'
```

**概述**：

跨视图匹配目标框：在目标检测列表中找到与参考框最相似的一项。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `source_img` | 参考视角 RGB 图像（通常为头部视图）。 |
| `ref_bbox` | 参考框 `(x1, y1, x2, y2)`。 |
| `target_img` | 目标视角 RGB 图像（通常为手臂视图）。 |
| `target_detections` | 目标视角检测结果列表（`ObjectDetection`）；至少 1 项。 |

**返回值**：

与参考框语义最相近的一项 `NamedBBox`（用于后续精准抓取）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_view = robot.eco_captureImages(CameraType.HEAD)
arm_view = robot.eco_captureImages(CameraType.ARM)
arm_items = list(robot.eco_detect_objects(
    DetectObjectsRequest(rgb_image=arm_view.rgb_image.img, labels=["玩具"]),
).items)
best = robot.eco_match_obj_views(
    head_view.rgb_image.img, (120, 80, 260, 300),
    arm_view.rgb_image.img, arm_items,
)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages (HEAD)
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_captureImages (ARM)
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o arm_view.json
## 第3步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(arm_view.json).rgb_image.img, "labels": ["玩具"]}}' -o arm_items.json
## 第4步：eco_match_obj_views
bajie_sdk run eco_match_obj_views --json '{"source_img": @(head_view.json).rgb_image.img, "ref_bbox": [120, 80, 260, 300], "target_img": @(arm_view.json).rgb_image.img, "target_detections": @(arm_items.json).items}' -o best.json
```

#### 5.4.9 `eco_pick_with_arm_review`

**签名**（`bajie_robot.py`）：

```python
def eco_pick_with_arm_review(view: 'RGBDViewWithPose', object_bbox: 'NamedBBox', *, timeout_sec: 'float' = 100.0) -> 'MissionStatus'
```

**概述**：

抓取（臂视角）：手部观测后在臂图重检并匹配，再执行精准抓取。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `view` | 参考视角 `RGBDViewWithPose`（通常为头部拍照结果）。 |
| `object_bbox` | `NamedBBox`：`id`、`name`（用于臂图检测标签；空串时回退 `object`）、`bbox`。 |
| `timeout_sec` | 单次 mission 超时（秒）。 |

**返回值**：

`MissionStatus` 任务状态。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_view = robot.eco_captureImages(CameraType.HEAD)
items = list(robot.eco_detect_objects(
    DetectObjectsRequest(rgb_image=head_view.rgb_image.img, labels=["玩具"]),
).items)
if items:
    st = robot.eco_pick_with_arm_review(head_view, items[0].to_named_bbox_for_grab(), timeout_sec=120.0)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(head_view.json).rgb_image.img, "labels": ["玩具"]}}' -o items.json
## 第3步：eco_pick_with_arm_review
bajie_sdk run eco_pick_with_arm_review --json '{"view": @(head_view.json), "object_bbox": @(items.json).items[0], "timeout_sec": 120.0}' -o st.json
```

#### 5.4.10 `eco_place_3D`

**签名**（`bajie_robot.py`）：

```python
def eco_place_3D(req: 'ObjectPose3D', *, timeout_sec: 'float' = 300.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

精准放置：按 6D 位姿自动导航并完成放置。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `req` | `ObjectPose3D`：`position`(`Vec3f`)、`orientation`(`Quatf`)、`box_length`(`Vec3f`)、`frame_id`。 |
| `timeout_sec` | 等待 `finish` 的最长秒数。 |
| `on_task_done` | 若给定，在任务结束时触发回调（不阻塞至 finish），本方法立即返回 `task_id`。 |

**返回值**：

未传 `on_task_done` 时返回 `MissionStatus`；传入时返回 `task_id`。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, ObjectPose3D, Vec3f, Quatf

robot = BajieRobot("10.88.41.120", auto_connect=True)
req = ObjectPose3D(position=Vec3f(x=1.0, y=2.0, z=0.0), orientation=Quatf(),
                   box_length=Vec3f(0.12, 0.1, 0.05), frame_id="map")
st = robot.eco_place_3D(req)
```

**CLI 示例**：

```bash
bajie_sdk run eco_place_3D --json '{"req": {"position": {"x": 1.0, "y": 2.0, "z": 0.0}, "orientation": {}, "box_length": {}, "frame_id": "map"}}' -o st.json
```

#### 5.4.11 `eco_place_with_view`

**签名**（`bajie_robot.py`）：

```python
def eco_place_with_view(view: 'RGBDViewWithPose', place_ref: 'Sequence[float]', recapture_place: 'bool' = False, *, timeout_sec: 'float' = 300.0) -> 'MissionStatus'
```

**概述**：

放置：`recapture_place=False` 执行语义放置；`True` 执行基于重投影的精准放置。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `view` | 头部 `RGBDViewWithPose`。 |
| `place_ref` | 长度 **2**（单点）或长度 **4**（框，取中心点）的像素坐标。 |
| `recapture_place` | `False` 语义放置；`True` 精准放置。 |
| `timeout_sec` | 等待 `finish` 的最长秒数。 |

**Python 示例**：

```python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_rgbd = robot.eco_captureImages(CameraType.HEAD)
st = robot.eco_place_with_view(head_rgbd, [153.0, 230.0, 403.0, 384.0], recapture_place=False)
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_rgbd.json
## 第2步：eco_place_with_view
bajie_sdk run eco_place_with_view --json '{"view": @(head_rgbd.json), "place_ref": [153.0, 230.0, 403.0, 384.0], "recapture_place": false}' -o st.json
```

#### 5.4.12 `eco_place_in`

**签名**（`bajie_robot.py`）：

```python
def eco_place_in(view: 'RGBDViewWithPose', carrier_bbox: 'Sequence[float]', *, timeout_sec: 'float' = 300.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

容器内推荐放：`carrier_bbox` 取前 4 项作为容器框，按 `[x1, y1, x2, y2]` 使用。

**Python 示例**：

```python
robot = BajieRobot("10.88.41.120", auto_connect=True)
head_rgbd = robot.eco_captureImages(CameraType.HEAD)
st = robot.eco_place_in(head_rgbd, [153.0, 230.0, 403.0, 384.0])
```

**CLI 示例**：

```bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_rgbd.json
## 第2步：eco_place_in
bajie_sdk run eco_place_in --json '{"view": @(head_rgbd.json), "carrier_bbox": [153.0, 230.0, 403.0, 384.0]}' -o st.json
```

### 5.5 本体控制

本节接口需要先 `robot.Connect()`。

#### 5.5.1 `eco_setRobotHeight`

**签名**（`bajie_robot.py`）：

```python
def eco_setRobotHeight(value: 'float', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

升降高度控制。`value` 范围 [0, 0.44] 米；0 = 最低，0.44 = 最高；超出范围自动截断并告警。

**Python 示例**：

```python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_setRobotHeight(0.22)
```

**CLI 示例**：

```bash
bajie_sdk run eco_setRobotHeight --json '{"value": 0.22}' -o st.json
```

#### 5.5.2 `eco_setRobotHead`

**签名**（`bajie_robot.py`）：

```python
def eco_setRobotHead(value: 'float', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

头部俯仰控制。`value` 范围 [0, 1.3] 弧度；0 = 最低，0.83 = 水平，1.3 = 最高；超出范围自动截断并告警。

**Python 示例**：

```python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_setRobotHead(0.83)
```

**CLI 示例**：

```bash
bajie_sdk run eco_setRobotHead --json '{"value": 0.83}' -o st.json
```

#### 5.5.3 `eco_setRobotArmMode`

**签名**（`bajie_robot.py`）：

```python
def eco_setRobotArmMode(mode: 'Union[RobotArmCtrlMode, int]', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

机械臂/夹爪模式控制。

`mode` 取值：`ARM_HOME(2)` 归位 / `GRIPPER_OPEN(6)` 打开 / `GRIPPER_CLOSE(7)` 关闭 / `UNBOX_POSE(9)` 开箱姿态 / `GRIPPER_MAINTENANCE(40)` 维护姿态。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, RobotArmCtrlMode

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_setRobotArmMode(RobotArmCtrlMode.ARM_HOME)
```

**CLI 示例**：

```bash
bajie_sdk run eco_setRobotArmMode --json '{"mode": "ARM_HOME"}' -o st.json
```

#### 5.5.4 `eco_setRobotPose`

**签名**（`bajie_robot.py`）：

```python
def eco_setRobotPose(object_pose: 'Sequence[PoseFrame]', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**概述**：

机械臂 pose 控制。每项使用 `header.frame_id`、`position`、`orientation`。`header.frame_id` 建议使用相机坐标系（如 `arm_camera_depth_optical_frame`）。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, PoseFrame, FrameHeader, Vec3f, Quatf

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_setRobotPose([
    PoseFrame(header=FrameHeader(frame_id="arm_camera_color_optical_frame"),
              position=Vec3f(0.3, 0.0, 0.5), orientation=Quatf()),
])
```

**CLI 示例**：

```bash
bajie_sdk run eco_setRobotPose --json '{"object_pose": [{"header": {"frame_id": "arm_camera_color_optical_frame"}, "position": {}, "orientation": {}}]}' -o st.json
```

### 5.6 任务暂停 / 恢复 / 取消

本节接口需要先 `robot.Connect()`。

#### 5.6.1 `eco_cancelAllMissions`

**签名**（`bajie_robot.py`）：

```python
def eco_cancelAllMissions(*, timeout_sec: 'float' = 30.0) -> 'MissionStatus'
```

**概述**：

取消所有任务。

**Python 示例**：

```python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_cancelAllMissions()
```

**CLI 示例**：

```bash
bajie_sdk run eco_cancelAllMissions --json '{"timeout_sec": 30.0}' -o st.json
```

#### 5.6.2 `eco_missionControl`

**签名**（`bajie_robot.py`）：

```python
def eco_missionControl(cmd: 'Command', task_id: 'str', *, timeout_sec: 'float' = 30.0) -> 'MissionStatus'
```

**概述**：

统一 mission 控制：暂停、恢复或取消。

> SDK 0.3.0 起返回类型从 `bool` 升级为 `MissionStatus`，包含错误码和版本信息。

**字段**（请求 / 入参）：

| 名称 | 说明 |
|:---|:---|
| `cmd` | 仅接受 `Command.PAUSE`、`Command.RESUME`、`Command.CANCEL`。 |
| `task_id` | 任务实例 id，须与目标任务一致。 |
| `timeout_sec` | 等待 `response` 的最长秒数。 |

**Python 示例**：

```python
from bajie_sdk import BajieRobot, Command

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_missionControl(Command.PAUSE, "semantic_navigation_0")
st = robot.eco_missionControl(Command.RESUME, "semantic_navigation_0")
st = robot.eco_missionControl(Command.CANCEL, "semantic_navigation_0")
```

**CLI 示例**：

```bash
bajie_sdk run eco_missionControl --json '{"cmd": "PAUSE", "task_id": "semantic_navigation_0"}' -o st.json
bajie_sdk run eco_missionControl --json '{"cmd": "RESUME", "task_id": "semantic_navigation_0"}' -o st.json
bajie_sdk run eco_missionControl --json '{"cmd": "CANCEL", "task_id": "semantic_navigation_0"}' -o st.json
```

### 5.7 机器状态

本节接口需要先 `robot.Connect()`。SDK 连接成功后自动获取 RobotInfo 缓存，后续随服务端推送更新。

#### 5.7.1 `eco_robotInfo`

**签名**（`bajie_robot.py`）：

```python
def eco_robotInfo() -> 'RobotInfo'
```

**概述**：

当前合并后的 `robot_info` 缓存快照。

**返回值**：

`RobotInfo`，字段：
- `workState` — `WorkState`（`task_id`、`name`、`cmd`）；
- `battery` — `BatteryInfo`（`value`、`isCharge`、`mode`）；
- `alarm` — `List[int]` 异常编码列表；
- `pos` — `Position`（`room`、`x`、`y`、`yaw`）；
- `mapinfo` — `MapInfo`（`mid`、`mname`、像素尺寸、分辨率、地图数据等）；
- `furniture` — `Furniture`（`info: List[SemanticMapManagerObjectInfo]`）全部语义地图家具/区域信息。

**Python 示例**：

```python
from bajie_sdk import BajieRobot

robot = BajieRobot("10.88.41.120", auto_connect=True)
info = robot.eco_robotInfo()
if info.workState:
    print(info.workState.cmd)
```

**CLI 示例**：

```bash
bajie_sdk run eco_robotInfo --json '{}' -o info.json
```

#### 5.7.2 `eco_refreshRobotInfo`

**签名**（`bajie_robot.py`）：

```python
def eco_refreshRobotInfo(topics: 'Optional[List[str]]' = None, *, timeout_sec: 'float' = 5.0) -> 'None'
```

**概述**：

手动刷新（可选 `topics` 子集）；合法值见 `RobotInfoTopic`。`topics` 为空时刷新全量。

**Python 示例**：

```python
from bajie_sdk import BajieRobot, RobotInfoTopic

robot = BajieRobot("10.88.41.120", auto_connect=True)
robot.eco_refreshRobotInfo(timeout_sec=5.0)
robot.eco_refreshRobotInfo(
    topics=[RobotInfoTopic.WORK_STATE.value, RobotInfoTopic.BATTERY.value],
    timeout_sec=3.0,
)
```

**CLI 示例**：

```bash
bajie_sdk run eco_refreshRobotInfo --json '{"timeout_sec": 5.0}'
bajie_sdk run eco_refreshRobotInfo --json '{"topics": ["workState", "battery"], "timeout_sec": 3.0}'
```

#### 5.7.3 `eco_robotWorkState` ~ `eco_robotFurniture`

以下接口均从缓存中读取对应的子状态：

| 接口 | 返回类型 | 关键字段 |
|:---|:---|:---|
| `eco_robotWorkState()` | `Optional[WorkState]` | `task_id`, `name`, `cmd` |
| `eco_robotBattery()` | `Optional[BatteryInfo]` | `value`, `isCharge`, `mode` |
| `eco_robotAlarm()` | `Optional[List[int]]` | 异常编码列表 |
| `eco_robotPosition()` | `Optional[Position]` | `room`, `x`, `y`, `yaw` |
| `eco_robotMapInfo()` | `Optional[MapInfo]` | `mid`, `mname`, `totalWidth`, `totalHeight`, 栅格数据 |
| `eco_robotFurniture()` | `Optional[Furniture]` | `info: List[SemanticMapManagerObjectInfo]` |

未获取时返回 `None`。

#### 5.7.4 `eco_bindRobotInfo` / `eco_unbindRobotInfo`

**签名**（`bajie_robot.py`）：

```python
def eco_bindRobotInfo(callback: 'Callable[[str, Any], None]', fields: 'Optional[List[str]]' = None) -> 'None'
def eco_unbindRobotInfo(callback: 'Callable[[str, Any], None]') -> 'None'
```

**概述**：

绑定/解绑 `robot_info` 字段变化回调。`fields` 为 `None` 时监听所有字段，此时 `field_name` 为 `"robot_info"`、`new_value` 为全量快照。

**Python 示例**：

```python
from typing import Any
from bajie_sdk import BajieRobot

def on_change(field: str, val: Any) -> None:
    print(f"{field} -> {val}")

robot = BajieRobot("10.88.41.120", auto_connect=True)
robot.eco_bindRobotInfo(on_change)                        # 监听全部
robot.eco_bindRobotInfo(on_change, fields=["workState"])  # 仅 workState
robot.eco_unbindRobotInfo(on_change)
```

**CLI 示例**：

```bash
bajie_sdk run eco_bindRobotInfo --json '{"callback": "on_change"}'
bajie_sdk run eco_bindRobotInfo --json '{"callback": "on_change", "fields": ["workState"]}'
bajie_sdk run eco_unbindRobotInfo --json '{"callback": "on_change"}'
```

---

## 6. 学习路径建议

如果你是第一次使用 Python SDK，建议按以下顺序逐步掌握：

| 阶段 | 目标 | 学习内容 |
|:---|:---|:---|
| **1. 安装验证** | SDK 装好 | 阅读 [§2 准备工作](#2-准备工作)，完成安装与验证 |
| **2. 首次连接** | 连接上机器人 | 运行 [§3.1 第一个程序](#31-第一个程序连接机器人并读取状态) |
| **3. 基础控制** | 移动 + 拍照 | 阅读 [§5.2.4 `eco_moveChassis`](#524-eco_movechassis) 和 [§5.3.1 `eco_captureImages`](#531-eco_captureimages) |
| **4. CLI 熟练** | 不写代码也能调接口 | 阅读 [§4 CLI 使用说明](#4-cli-使用说明) |
| **5. 感知与检测** | 识别物体 | 阅读 [§5.1.1 `eco_detect_objects`](#511-eco_detect_objects) |
| **6. 抓取放置** | 完成拾放闭环 | 阅读 [§3.3 抓取/放置流程](#33-抓取放置流程) 和 [§5.4](#54-高层任务) |
| **7. 建图导航** | 机器人自主移动 | 阅读 [§5.2.1](#521-eco_automapbuild)、[§5.2.5](#525-eco_navigatetopoint)、[§5.2.6](#526-eco_navigatetosemanticarea) |
| **8. 桌面整理（VLM）** | AI 驱动整理 | 阅读 [§5.1.6 `eco_vlm_desk_sort_plan`](#516-eco_vlm_desk_sort_plan) 及其上下游接口 |
| **9. 高级用法** | 异步回调、状态订阅 | 阅读 [§3.4 异步回调模式](#34-异步回调模式)、[§3.5 连接事件与错误处理](#35-连接事件与错误处理)、[§5.7 机器状态](#57-机器状态) |

> **排障参考**：任何时候遇到问题，先看 [§7 故障排查](#7-故障排查)。

---

## 7. 故障排查

### 7.1 `Connect()` 失败

- 检查机器人 IP 是否正确。
- 确认电脑和机器人是否在同一网络。
- 确认 WebSocket 地址是否写成 `ws://机器人IP:9900/`。
- 检查机器人端机身控制服务（WebSocket/9900）是否正常运行。

### 7.2 CLI 查不到命令

先验证 SDK 是否安装：

```bash
python3 -m bajie_sdk -h
```

如果可以运行，但 `bajie_sdk` 这个短命令不可用，说明别名/补全没配置。可以继续使用：

```bash
python3 -m bajie_sdk run eco_robotInfo --json '{}'
```

### 7.3 JSON 参数不知道怎么写

优先用：

```bash
bajie_sdk run eco_xxx --json-template
bajie_sdk -m eco_xxx
bajie_sdk -t TypeName
```

### 7.4 任务失败

优先打印：

```python
print(status.task_id)
print(status.error_info)
```

如果 `error_info` 中包含错误码，先看错误码等级，再决定处理方案：

| 等级 | 含义 | 建议处理 |
|:---|:---|:---|
| `0` | 成功 | 无需处理 |
| `1` | 警告/可恢复错误 | 可重试 1-3 次，或检查参数、网络、服务响应 |
| `2` | 错误 | 结合场景排查，必要时停止当前流程 |
| `3` | 严重错误 | 先做环境分析，清除障碍或调整目标，再重试 |

### 7.5 任务超时

**超时不一定会取消机器人正在执行的任务**。如果要主动取消：

```python
# 取消所有任务
robot.eco_cancelAllMissions()

# 取消指定任务
from bajie_sdk import Command
robot.eco_missionControl(Command.CANCEL, task_id)
```

### 7.6 安装问题

| 问题 | 处理 |
|:---|:---|
| `pip install bajie-sdk` 找不到包 | PyPI 无公开包，请用 `.whl` 文件安装（见 §2.2） |
| 安装后 `import bajie_sdk` 失败 | 确认使用的 `python3` 与安装时一致；wheel 与平台/Python 版本需匹配 |
| pip 安装依赖下载困难 | 使用 `pip install --no-deps ./bajie_sdk-*.whl` 跳过依赖安装 |
| `PYTHONPATH` 问题（可编辑安装） | 在 `python_ws_sdk` 目录执行 `export PYTHONPATH=.` 或 `pip install -e .` |

### 7.7 常用错误码速查

#### 成功与通用状态

| 错误码 | 十进制 | 等级 | 含义 | 处理 |
|:---|:---|:---|:---|:---|
| `0x00000000` | `0` | `0` | 成功 | 无需处理 |
| `0x00000208` | `520` | `-` | 通用超时 | 检查网络、机器人服务和当前任务状态 |
| `0x00000242` | `578` | `-` | 获取机器状态超时 | 重试状态查询，检查连接 |
| `0x00000243` | `579` | `-` | 机器在充电且电量低 | 等待充电或避免执行高耗电任务 |
| `0x00000244` | `580` | `-` | 机器未充电且电量低 | 优先回充 |
| `0x00000245` | `581` | `-` | 机器状态非空闲 | 查询 `eco_robotWorkState()`，必要时取消当前任务 |
| `0x00000253` | `595` | `-` | 机器忙碌中或电量不足 | 等待空闲、取消任务或先回充 |

#### 任务调度

| 错误码 | 十进制 | 等级 | 含义 | 处理 |
|:---|:---|:---|:---|:---|
| `0x00002000` | `8192` | `1` | 任务恢复失败 | 确认任务是否仍存在 |
| `0x00002001` | `8193` | `1` | 任务暂停失败 | 确认任务是否正在运行 |
| `0x00002003` | `8195` | `1` | 任务未在运行 | 不要继续 pause/resume/cancel 这个任务 |
| `0x00002004` | `8196` | `1` | 任务参数无效 | 检查请求结构和字段类型 |
| `0x00002005` | `8197` | `1` | 任务已在运行 | 等待当前任务结束，或先取消 |
| `0x0000201c` | `8220` | `1` | 机器正在充电 | 需要移动/抓取前先下桩 |

#### 移动、建图、导航

| 错误码 | 十进制 | 等级 | 含义 | 处理 |
|:---|:---|:---|:---|:---|
| `0x00002710` | `10000` | `1` | 底盘移动参数无效 | 检查 `move_distance`、`move_angle` |
| `0x00002711` | `10001` | `1` | 底盘移动超时 | 检查地面和障碍物 |
| `0x00002720` | `10016` | `1` | 导航参数无效 | 检查目标点或区域参数 |
| `0x00002721` | `10017` | `1` | 导航任务超时 | 检查目标是否过远、路径是否被挡 |
| `0x00002730` | `10032` | `1` | SLAM 探索服务无响应 | 检查建图服务状态，重试 |
| `0x00002740` | `10048` | `1` | 语义探索参数无效 | 检查区域信息 |
| `0x00002770` | `10096` | `1` | 语义地图管理参数无效 | 检查 `id`、`model_level`、轮廓和朝向 |
| `0x00007002` | `28674` | `3` | 无法搜索出全局路线 | 检查起点、关键路径、传感器噪声和障碍物 |
| `0x00007004` | `28676` | `3` | 路线被物体阻挡 | 清除可移动障碍物，再重试 |
| `0x00007006` | `28678` | `3` | 机器陷入致命障碍物层 | 检查机器周围障碍，必要时人工处理 |

#### 图像、视觉、VLM

| 错误码 | 十进制 | 等级 | 含义 | 处理 |
|:---|:---|:---|:---|:---|
| `0x00002790` | `10128` | `1` | 视觉模型参数无效 | 检查图片、bbox、请求结构 |
| `0x00002791` | `10129` | `1` | 视觉模型任务超时 | 检查 VLM/视觉服务和网络，适当加大超时 |
| `0x00002792` | `10131` | `1` | 视觉模型服务未响应 | 检查服务状态，重试 |
| `0x00002793` | `10132` | `1` | 未识别出有效物体 | 换角度拍照、调整标签、改善光照 |
| `0x000027A0` | `10144` | `1` | 图片请求参数无效 | 检查 `camera_type` |
| `0x000027A1` | `10145` | `1` | 图片请求超时 | 检查相机服务，重试 |

#### 机械臂、抓取、放置

| 错误码 | 十进制 | 等级 | 含义 | 处理 |
|:---|:---|:---|:---|:---|
| `0x000027E0` | `10208` | `1` | 机械臂观测自适应服务无响应 | 重试；检查机械臂服务 |
| `0x000027E1` | `10209` | `1` | 机械臂观测自适应参数无效 | 检查 bbox、view、camera_type |
| `0x000027E2` | `10210` | `1` | 机械臂观测自适应超时 | 换角度或重试 |
| `0x00002840` | `10304` | `1` | 高度控制任务超时 | 检查升降机构是否被阻挡 |
| `0x00002842` | `10306` | `1` | 高度控制参数无效 | `value` 应在 `[0, 0.44]` |
| `0x00009000` | `36864` | `1` | 机械臂执行失败 | 重试；多次失败后进入通用异常流程 |
| `0x00009001` | `36865` | `1` | 机械臂任务超时 | 检查是否被阻挡 |
| `0x00009002` | `36866` | `1` | 机械臂轨迹规划失败 | 清除障碍 |
| `0x00009004` | `36868` | `3` | 目标点超出机械臂工作范围 | 换目标位置或让机器人重新对准 |
| `0x00009006` | `36870` | `1` | 机械臂逆解失败 | 重新观测一次，生成新的抓取姿态 |
| `0x00009015` | `36885` | `1` | 托盘已满，无法放置 | 清理托盘或换放置位置 |
| `0x00009021` | `36897` | `1` | 机械臂未抓到物体 | 重新观测抓取，或调用寻物/附近找物逻辑 |

#### 回充、下桩、电量

| 错误码 | 十进制 | 等级 | 含义 | 处理 |
|:---|:---|:---|:---|:---|
| `0x00002880` | `10368` | `1` | 回充任务超时 | 检查充电桩附近环境，重试 |
| `0x00002881` | `10369` | `1` | 回充服务无响应 | 检查回充服务状态 |
| `0x000028b0` | `10416` | `1` | 电量过低 | 优先回充 |
| `0x000028b1` | `10417` | `1` | 电量读取超时 | 重试读取电量 |
| `0x000028c0` | `10432` | `1` | 下桩超时 | 检查充电桩周围是否有障碍 |
| `0x000028c3` | `10435` | `1` | 下桩电流异常 | 停止任务并检查充电接触/底盘状态 |
| `0x00033001` | `208897` | `1` | 找不到反光片和红外信号 | 检查充电桩、反光片、红外信号和遮挡 |
