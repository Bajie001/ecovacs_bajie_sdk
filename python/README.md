# README

# BaJie Robot SDK Development Document \(Python\) 

This document is intended for end users, introducing how to obtain and install ` bajie_sdk `, how to quickly call the robot's capabilities, and how to query interface and field descriptions via CLI\. 

- **It is recommended that developers choose an x86\_64 PC running the Ubuntu system for development and debugging\. For an x86\_64 PC running the Windows system, the installation of Python\-related environments can be configured independently\. This article uses the Ubuntu system as an example for explanation\. **

- **Before debugging API functions and developing demo scenarios, it is necessary to confirm whether the machine has a map\. If not, mapping can be performed through the mapping button on the body screen, the mapping button in robot control on the visual development platform, and by calling the automatic mapping API in the API\.**

---

## Table of Contents

- 1\. Overview

    - 1\.1 What is the SDK?

    - 1\.2 Key Concepts

    - 1\.3 Mapping Prerequisites

- 2\. Preparation

    - 2\.1 Obtain the whl Package

    - 2\.2 Install the SDK

    - 2\.3 Verify Installation

    - 2\.4 Configure Command Line Completion \(Optional\)

- 3\. Quick Start

    - 3\.1 First Program: Connect to the Robot and Read Status

    - 3\.2 Common Task Links

    - 3\.3 Pick/Place Process

    - 3\.4 Asynchronous Callback Mode

    - 3\.5 Connection Events and Error Handling

- 4\. CLI Usage Instructions

    - 4\.1 Query and Help

    - 4\.2 Execute Single Interface \(run Subcommand\)

    - 4\.3 Inline References and Image Placeholders

    - 4\.4 Return Value Encoding and Image Hosting

- 5\. `BajieRobot.eco_*` Interface Overview \(Full\)

    - 5\.1 Perception

        - 5\.1\.1 `eco_detect_objects`

        - 5\.1\.2 `eco_build_embeddings`

        - 5\.1\.3 `eco_calc_similarity`

        - 5\.1\.4 `eco_pairwise`

        - 5\.1\.5 `eco_shoe_placement_plan`

        - 5\.1\.6 `eco_vlm_desk_sort_plan`

        - 5\.1\.7 `eco_vlm_perception`

        - 5\.1\.8 `eco_vlm_match`

        - 5\.1\.9 `eco_vlm_suggest_angle`

        - 5\.1\.10 `eco_vlm_judge`

        - 5\.1\.11 `eco_put_where_summary`

    - 5\.2 Mapping and Navigation

        - 5\.2\.1 `eco_autoMapBuild`

        - 5\.2\.2 `eco_semanticMapBuild`

        - 5\.2\.3 `eco_manageSemanticMap`

        - 5\.2\.4 `eco_moveChassis`

        - 5\.2\.5 `eco_navigateToPoint`

        - 5\.2\.6 `eco_navigateToSemanticArea`

        - 5\.2\.7 `eco_leaveDock`

        - 5\.2\.8 `eco_relocate`

        - 5\.2\.9 `eco_startRecharge`

    - 5\.3 Image and Pose

        - 5\.3\.1 `eco_captureImages`

        - 5\.3\.2 `eco_computeObjectPose`

    - 5\.4 High\-level Tasks

        - 5\.4\.1 `eco_locatePerson`

        - 5\.4\.2 `eco_robotSpeech`

        - 5\.4\.3 `eco_findObject`

        - 5\.4\.4 `eco_prepareRobotPose`

        - 5\.4\.5 `eco_finishRobotPose`

        - 5\.4\.6 `eco_lookto`

        - 5\.4\.7 `eco_pick`

        - 5\.4\.8 `eco_match_obj_views`

        - 5\.4\.9 `eco_pick_with_arm_review`

        - 5\.4\.10 `eco_place_3D`

        - 5\.4\.11 `eco_place_with_view`

        - 5\.4\.12 `eco_place_in`

    - 5\.5 Ontology Control

        - 5\.5\.1 `eco_setRobotHeight`

        - 5\.5\.2 `eco_setRobotHead`

        - 5\.5\.3 `eco_setRobotArmMode`

        - 5\.5\.4 `eco_setRobotPose`

    - 5\.6 Task Pause/Resume/Cancel

        - 5\.6\.1 `eco_cancelAllMissions`

        - 5\.6\.2 `eco_missionControl`

    - 5\.7 Machine Status

        - 5\.7\.1 `eco_robotInfo`

        - 5\.7\.2 `eco_refreshRobotInfo`

        - 5\.7\.3 `eco_robotWorkState` \~ `eco_robotFurniture`

        - 5\.7\.4 `eco_bindRobotInfo` / `eco_unbindRobotInfo`

- 6\. Learning Path Recommendations

- 7\. Troubleshooting

    - 7\.1 `Connect()` Failed

    - 7\.2 CLI cannot find the command

    - 7\.3 Don't know how to write JSON parameters

    - 7\.4 Task Failed

    - 7\.5 Task Timeout

    - 7\.6 Installation Issues

    - 7\.7 Quick Reference for Common Error Codes

---

## 1\. Overview

### 1\.1 What is the SDK?

`bajie_sdk` is the Bajie Robot Python Development Kit, provided in the form of a `.whl` package with the robot upon factory shipment\. Through the SDK, you can:

- **Control Robot**: Movement, Navigation, Manipulator Operation, Grasping and Placement

- **Perceptual Environment**: Open Vocabulary Detection \(OVD\), Image Feature Matching, VLM Desktop Organization

- **Query Status**: Obtain power, location, alarm, map, and furniture information

- **CLI Quick Debugging**: Directly call the interface via the command line to query field templates

### 1\.2 Key Concepts

|Project|Description|
|---|---|
|Current Version|**0\.4\.2**（Python 3\.10，`cp310`）|
|Name|Package name `bajie-sdk`, import name `bajie_sdk`, class name `BajieRobot`|
|Default Robot IP|**`10.10.10.11`** \(subject to the IP displayed at the bottom of the screen `http://10.10.10.11:17890`\) |
|Default connection address|`ws://10.10.10.11:9900/`|
|Environment Variable|`bajie_sdk_WS_URL` sets the default WebSocket address|
|Constructor Parameters|`BajieRobot(ws_url="ws://<IP>:9900/")` explicitly specifies the address |
|Installation Source|No public PyPI package —— please use the delivered `.whl ` file for installation |

### 1\.3 Mapping Prerequisites

- **Navigation\-related interfaces** \(such as `eco_navigateToPoint`\) are recommended to build a map first\.

- **Semantic operations** \(such as `eco_navigateToSemanticArea`\) require the completion of automatic mapping, which will create a semantic map and can also edit the semantic map on the machine's web page\.

- **VLM\-related interfaces ** \(such as ` eco_vlm_perception `\) can only be used after configuring the VLM API KEY in the machine configuration web page\. 

---

## 2\. Preparation

### 2\.1 Obtain the whl Package

`bajie_sdk` is provided in the form of a whl package with the robot upon factory shipment \(the package comes with the robot\)\. ** The current publish requires Python 3\.10 ** \( ` cp310 ` \); ** x86\_64 ** and ** arm64 \(aarch64\) ** are different wheels, and the corresponding file must be selected according to the target robot's CPU architecture\. 

### 2\.2 Install the SDK

**x86\_64** Example:

```Bash
python3 -m pip install ./bajie_sdk-*-cp310-cp310-linux_x86_64.whl
```

**arm64 \(aarch64\)** Example \(filename subject to deliverables, commonly `linux_aarch64`\) 

```Bash
python3 -m pip install ./bajie_sdk-*-cp310-cp310-linux_aarch64.whl
```

`pip install` will automatically install the dependencies declared in `pyproject.toml` \(such as `websocket-client`, `numpy`, `opencv-python`, etc\.\)\. If you need to skip dependency installation, add the `--no-deps` parameter:

```Bash
python3 -m pip install --no-deps ./bajie_sdk-*-cp310-cp310-linux_x86_64.whl
```

> **It is recommended to use a virtual environment ** \(which can avoid dependency conflicts\): 

```Bash
python3 -m venv .venv
source .venv/bin/activate
pip install ./bajie_sdk-*-cp310-cp310-linux_x86_64.whl
```

### 2\.3 Verify Installation

```Bash
python3 -c "import bajie_sdk; print('ok', bajie_sdk.__name__)"
```

Seeing `ok bajie_sdk` indicates successful installation\.

### 2\.4 Configure Command Line Completion \(Optional\)

After installation, run the following command to automatically set up ` bajie_sdk ` command aliases and Tab completion: 

```Bash
python3 -m bajie_sdk completion
```

This command automatically writes the following two lines to `~/.bashrc` \(or `~/.zshrc`\): 

```Bash
alias bajie_sdk='python3 -m bajie_sdk'
source /path/to/installed/bajie_sdk/cli/completion.sh
```

After that, you can use ` bajie_sdk run eco_<TAB> ` to complete method names, ` --json ` templates, etc\. 

> When completion is not configured, directly use the full command `python -m bajie_sdk xxx`, and the effect is exactly the same\. 

---

## 3\. Quick Start

### 3\.1 First Program: Connect to the Robot and Read Status

Default connection **`10.10.10.11:9900`** \(replace according to the IP displayed on the screen\); to change the address, use **`BajieRobot(ws_url="ws://…")`** or the environment variable **`bajie_sdk_WS_URL`**, then **`Connect()`**\.

```Python
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

### 3\.2 Common Task Links

Select appropriate links according to task requirements: 

|Scene|Interface Order|
|---|---|
|Observation and Detection|`eco_captureImages` \+ `eco_detect_objects`|
|Alignment and Grasping|`eco_lookto` \+ `eco_pick`|
|Grasping after cross\-view matching |`eco_match_obj_views` \+ `eco_pick_with_arm_review`|
|Place |`eco_place_with_view` / `eco_place_3D` / `eco_place_in`|
|VLM Desktop Organizer |`eco_vlm_perception` → `eco_vlm_desk_sort_plan` → `eco_vlm_match`|
|Sorting Effect Evaluation|`eco_vlm_judge`|
|Shoe Storage Plan |`eco_detect_objects`\(SHOE\) → `eco_pairwise` → `eco_shoe_placement_plan`|
|PutWhere Teach Placement|`eco_put_where_summary` → `eco_place_in`|
|Image Feature Matching |`eco_build_embeddings` \+ `eco_calc_similarity`|
|Semantic Map Query|`eco_robotFurniture` → `eco_manageSemanticMap(QUERY, fid)` → 增/删/改|

### 3\.3 Pick/Place Process

The grasping/placement choreography capabilities have been unified and converged into the ` BajieRobot ` 's ` eco_* ` method\. 

```Python
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

Detailed parameters, return values, and exception handling shall be subject to the interface description in §5\.

### 3\.4 Asynchronous Callback Mode

All blocking `eco_*` methods \(i\.e\., methods that return `MissionStatus` or its subclasses\) support an optional **`on_task_done`** callback parameter\.

**Applicable Scenario**: When you do not want the current thread to block and wait for the task to finish, pass in a callback to let the SDK automatically trigger it in the packet receiving thread, and at the same time, this method immediately returns `task_id` \(type `str`\)\. 

**Usage Method**: 

```Python
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

**Rule Explanation**: 

|Condition|Behavior|
|---|---|
|Do not pass`on_task_done`\(default\)|Block and wait for finish, returning the result object \( ` MissionStatus ` / ` RGBDViewWithPose ` /\.\.\. \) |
|Passed ` on_task_done `|Non\-blocking, returns immediately`str`\(task\_id\); the result is received asynchronously in the callback|
|`timeout_sec` remains effective in callback mode|is used to wait for mission ** start confirmation ** \(response\), not finish |
|`on_task_done` is called in the SDK packet receiving thread|Do not perform time\-consuming operations within the callback to avoid affecting packet reception |
|Method signature uses `@typing.overload`|The IDE can correctly infer the return type without passing ` on_task_done ` |

**Which methods support callbacks**: All mission class methods in §5\.2\~§5\.5 except for chained composite methods \(`eco_lookto`, `eco_pick_with_arm_review`, `eco_place_with_view`\) and `eco_cancelAllMissions`\. Perception class methods \(§5\.1\) and status query class methods \(§5\.7\) do not support callbacks\.

### 3\.5 Connection Events and Error Handling

`BajieRobot` provides connection status event callbacks and error formatting tools:

```Python
from bajie_sdk import BajieRobot, format_error_info

robot = BajieRobot(ws_url="ws://10.10.10.11:9900/")
robot.SetOnConnected(lambda: print("已连接"))
robot.SetOnError(lambda e: print(format_error_info(e)))

if not robot.Connect(timeout_sec=8.0):
    raise RuntimeError("连接失败")

# ... 执行任务 ...

robot.Disconnect()
```

- **`SetOnConnected(callback)`** — Register a callback for successful connection \(no parameters\)\.

- **`SetOnError(callback)`** — Register an error callback, with the parameter being an error object\.

- **`format_error_info(error)`** — Formats the SDK internal error object into a readable string, suitable for logging and troubleshooting\.

---

## 4\. CLI Usage Instructions

After the SDK is installed, it comes with a command\-line tool ` bajie_sdk ` \(equivalent to ` python -m bajie_sdk `, see §2\.4\), which can be used to query interfaces, generate JSON templates, and directly call robot interfaces\. 

### 4\.1 Query and Help

|Command|Function|
|---|---|
|`bajie_sdk -h`|Main Help|
|`bajie_sdk run -h`|Help for run subcommand \(including `--background` background mode description\)|
|`bajie_sdk -l`|List all categories \(one per line\)|
|`bajie_sdk -l keyword`|Filter by category title and list related `eco_*` and signature first line|
|`bajie_sdk -m eco_xxx`|Output a single line signature \+ full docstring of a single method|
|`bajie_sdk -t TypeName`|Field, enum member, or constructor signature of the output type|

> Recommendation: When unsure about parameter fields, prioritize using the `-m` \+ `-t` combination for queries\.

### 4\.2 Execute Single Interface \(run Subcommand\)

> It is recommended to set environment variables first to avoid specifying ` --ws-url ` every time: 

```Bash
export bajie_sdk_WS_URL=ws://10.10.10.11:9900/
```

> can also be written into `~/.bashrc` or `~/.zshrc` for persistent effect\. `run` subcommand's `--ws-url` parameter takes precedence over this environment variable\. 

```Bash
bajie_sdk run eco_prepareRobotPose --json '{}' -o prepare_pose.json
```

**Basic Parameters**: 

|Parameter|Description|
|---|---|
|`method`|Method name, e\.g\., `eco_startRecharge` \(the `eco_` prefix can be omitted\)|
|`-j` / `--json`|Inline JSON object, where the key names correspond to `eco_*` formal parameters \(including keyword parameters such as `timeout_sec`\)\. If the first parameter is a dataclass, you can write `"req": {... }` nested, or you can **flatten** its fields at the top level|
|`-t` / `--json-template`|Do not connect to the robot, only generate a JSON template to stdout based on the signature\.**Enumerate fields**Output member name and at the end of the line `#` Comment to mark all optional values \(`Name(Value)`\) for easy reference|
|`-o PATH`|Encode the return value as JSON and write it to the specified file|

**`--json-template`**** Example **: 

```Bash
# 短标志 -t 等效
bajie_sdk run eco_captureImages -t
# 输出示例（枚举字段行尾 # 标注可选值）：
# {
#   "camera_type": "ARM",  # ARM(1) | HEAD(2)
#   "timeout_sec": 30.0
# }
```

### 4\.3 Inline References and Image Placeholders

`--json ` string supports `@(path) ` syntax: 

|Writing Style |Effect|
|---|---|
|`@(config.json)`|Embed the entire JSON root object|
|`@(filter.json).object`|After reading the file, press `. ` key chain to drill down |
|`@(embs.json)[0]`|Read the 0th item of the JSON array, which can be combined with key chains, such as `@(resp.json).items[0].bbox`|
|`"img": "@(images/rgb.png)"`|Image placeholder, loaded as ndarray \(png/jpg/jpeg/webp/bmp\) at runtime|

> Note: The image path** should not **be written as `@(fig.png).a.b`; raster image placeholders do not support key chains\. Relative paths are relative to the current working directory, and the base can be changed using `--base-dir`\.

**`--json `**** input supports ****`#`**** line comments ** \(within strings, `#` is preserved as is and does not affect parsing\): 

```Bash
bajie_sdk run eco_captureImages --json '
{
    "camera_type": 2,      # 2=HEAD 头部相机
    "timeout_sec": 30.0
}
'
```

### 4\.4 Return Value Encoding and Image Hosting

If the area of the returned ndarray image is \> 4096 pixels, the CLI automatically saves it as a PNG to the same directory as `-o`, and references it in JSON using `@(filename)`: 

```JSON
{"rgb_image": "@(rgb_image_1745712345678_1.png)", "depth_image": "@(depth_image_1745712345678_2.png)"}
```

PNG naming format is `{Field Name}_{Timestamp}_{Sequence Number}.png`\. 

---

## 5\. `BajieRobot.eco_*` Interface Overview \(Full\)

This section organizes all public interfaces of the SDK by functional module\. Each interface description includes: full signature, field description, return value description, Python example, and CLI example\. 

> **Usage Instructions**: 

- Perception interfaces \(§5\.1\) typically do not require `Connect()` and are directly invoked via HTTP\.

- Navigation/Control/Status Class Interfaces \(§5\.2\~§5\.7\) need to be called after `robot.Connect()`\.

- `timeout_sec` is in seconds;`on_task_done` supports both blocking and asynchronous modes \(see §3\.4\)\.

- **Two\-level Path**: ① Ontology Mission: `BajieRobot` → `missions` → WebSocket\. ② Perception HTTP: `BajieRobot.eco_*` → HTTP, without WebSocket\.

### 5\.1 Perception

#### 5\.1\.1 `eco_detect_objects`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_detect_objects(req: 'DetectObjectsRequest', *, timeout: 'float' = 10.0) -> 'DetectObjectsResponse'
```

**Overview**: 

Open Vocabulary Detection \(OVD\): Perform general object detection and attribute query on RGB images\.

`labels` Supported item labels:

- **Furniture **: Bed, Other Beds, Crib, Nightstand, Bookcase, Dresser, Bay Window, Wardrobe, Table, Floor Mirror, Sofa, Chaise Lounge Sofa, Other Sofas, Armchair, Coffee Table, TV Stand, Storage Rack, Side Table, Bench, Chair, Cabinet, Other Cabinets, Filing Cabinet, Island Counter, Shoe Changing Stool, Floor Lamp, Shoe Cabinet, Shoe Rack, Storage Box, Storage Basket, Ordinary Door, Plastic Stool 

- **Home Appliances**: Washing Machine, Refrigerator, Floor Cleaning Robot, Base Station, TV, Audio System

- **Kitchenware**: bowls, chopsticks, spoons, glasses, plates, kitchen knives, fruit knives, dinner knives, forks, cups, mugs

- **Electronic Products **: Keyboard, Mouse, Headphone, Monitor, Laptop, Computer Host, Camera, Calculator, Mobile Phone, Tablet, Game Controller, Power Cord, Data Cable, Power Strip, Plug, Power Bank, Camera, Radio 

- **Daily Necessities**: Tissue, Wet Wipe, Laundry Basket, Trash Can, Pen Holder, Desk Organizer, Green Plant, Floral Decoration, Vase, Table Lamp, Book, Pen, Calendar, Glasses, Paper Ball, Ashtray, Keychain, Figurine, Mouse Pad, Carpet, Wallet, Alarm Clock, Umbrella, Phone Stand, Ornament, Pencil Case, Tape, Eraser, Stationery Box, Glue Stick, Fan, Sanitary Napkin, Socks, Handkerchief, Rag, Shoes

- **Clothing **: Balled\-up clothing, shirts, pants 

- **Toys **: toy cars, toy balls, toy dinosaurs, toy rabbits, toy bears, toy guns, toy airplanes, toy skateboards, toy excavators, toy cranes, other toys, toy building blocks, dolls, balloons, rattles, windmills, toy carrots, toy trains, Rubik's cubes, game boards, castle models, spinning tops, claw machines, toy cards 

- **Pet Supplies**: Litter Box, Cat Tree

- **Food**: Snacks, Bottles

- **Fruits **: banana, apple, watermelon, pineapple, orange, pear, grape, strawberry, cherry, mango, peach, tangerine, pomelo, loquat, dragon fruit, Hami melon 

- **Others **: Vegetables, Cans, Sinks, Staplers, Eyeglass Cases 

`SHOE` model provides limited shoe recognition, and expansion requires custom training and uploading weights\.`entry` parameter selection `OvdEndpoint.OVD` \(default\) or `OvdEndpoint.SHOE`\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|Request Body ` DetectObjectsRequest `, the field meanings are as follows: |
|`timeout`|HTTP request timeout \(seconds\)\.|

**The ****` req `**** subfield ** : 

|Field|Description|
|---|---|
|`rgb_image`|RGB images are generally `numpy.ndarray` \(H×W×C\); |
|`labels`|List of open word tag strings; non\-supported parent tags \(e\.g\., `"toys"`\) are automatically expanded into a list of child tags;|
|`position_region`|Optional, semantic area name \(e\.g\., `"Living Room"`\), limits the detection range to this area; |
|`payload`|Optional, name of the carrier \(e\.g\., `"table"`\), used in conjunction with `position_region` to further limit the detection range;|
|`ovd_property`|`OvdQueryProperty` \(query properties such as color/shape/person, etc\.\); |
|`ovd_obj_thresh`|OVD Score Threshold;|
|`box_obj_thresh`|Box score threshold;|
|`entry`|`OvdEndpoint` \( `OVD` or `SHOE` \) determines the subpath of the server\.|

**Return Value **: 

`DetectObjectsResponse`: `rgb_image`; `items`, `items_unknow` are parsed `ObjectDetection` lists\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest, OvdEndpoint

robot = BajieRobot("10.88.41.120", auto_connect=True)
img = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
req = DetectObjectsRequest(rgb_image=img, labels=["鞋子"], entry=OvdEndpoint.SHOE)
resp = robot.eco_detect_objects(req, timeout=15.0)
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o img.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(img.json).rgb_image.img, "labels": ["鞋子"], "entry": "SHOE"}, "timeout": 15.0}' -o resp.json
```

#### 5\.1\.2 `eco_build_embeddings`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_build_embeddings(images_rgb: 'Sequence[Union[np.ndarray, str]]', *, timeout: 'float' = 10.0) -> 'List[List[float]]'
```

**Overview**: 

Generate feature vectors \(embeddings\): Pass in images in batches and return a list of corresponding feature vectors, with one vector per image\. The obtained vectors can be passed to `eco_calc_similarity` for similarity comparison\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`images_rgb`|Multiple RGB images, each of which can be `numpy.ndarray` or a PNG base64 string\.|
|`timeout`|Timeout \(seconds\)\.|

**Return value **: 

List, where each item is a feature vector corresponding to the order of `images_rgb` \(`List[float]`\)\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
img = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
vecs = robot.eco_build_embeddings([img])
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o img.json
## 第2步：eco_build_embeddings
bajie_sdk run eco_build_embeddings --json '{"images_rgb": [@(img.json).rgb_image.img]}' -o vecs.json
```

#### 5\.1\.3 `eco_calc_similarity`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_calc_similarity(req: 'CalcSimilarityRequest') -> 'float'
```

**Overview**: 

Calculate Similarity: Compare the similarity between two images or two sets of feature vectors \(embeddings\), and return a score between 0 and 1\. 

> Note: The timeout setting for this method is inside `CalcSimilarityRequest` rather than a keyword argument\. Each time it is called, at least one value must be filled in for both Route A and Route B, and they cannot all be empty\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|`CalcSimilarityRequest`|

**The ****` req `**** subfield ** : 

|Field|Description|
|---|---|
|`image_a_base64`|Optional, PNG base64 string of Image A;|
|`image_b_base64`|Optional, PNG base64 string of Image B;|
|`embedding_a`|Optional, the feature vector of image A \(obtained by `eco_build_embeddings`\);|
|`embedding_b`|Optional, feature vector of image B;|
|`timeout`|Timeout \(seconds\)\.|

**Return Value **: 

`float` Similarity \(the closer the value is to 1, the more similar it is\)\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType, CalcSimilarityRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_view = robot.eco_captureImages(CameraType.HEAD)
arm_view = robot.eco_captureImages(CameraType.ARM)
embs = robot.eco_build_embeddings([head_view.rgb_image.img, arm_view.rgb_image.img])
s = robot.eco_calc_similarity(CalcSimilarityRequest(embedding_a=embs[0], embedding_b=embs[1]))
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o arm_view.json
## 第3步：eco_build_embeddings
bajie_sdk run eco_build_embeddings --json '{"images_rgb": [@(head_view.json).rgb_image.img, @(arm_view.json).rgb_image.img]}' -o embs.json
## 第4步：eco_calc_similarity
bajie_sdk run eco_calc_similarity --json '{"req": {"embedding_a": @(embs.json)[0], "embedding_b": @(embs.json)[1]}}' -o s.json
```

#### 5\.1\.4 `eco_pairwise`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_pairwise(frames: 'Sequence[tuple[Union[np.ndarray, str], Sequence[ObjectDetection]]]', *, timeout: 'float' = 15.0) -> 'List[tuple[str, str]]'
```

**Overview**: 

Cross\-frame object pairing: Associate the detection results across multiple frames based on visual similarity, and the same physical object will be assigned the same `pair_id` in different frames\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`frames`|Multi\-frame sequence, where each frame is a `(image, objs)` tuple:|
|`timeout`|Timeout \(seconds\)\.|

**The ****` frames `**** subfield **: 

|Field|Description|
|---|---|
|`image`|RGB `ndarray` or PNG base64, **only provides visual features for the matching algorithm and does not directly participate in the pairing **; |
|`objs`|List of `ObjectDetection` to be paired in this frame \(OVD detection results\)\. |
|Remarks|**The core of pairing is objects **: visually similar objects in different frames will be grouped into the same pair, allowing mixed matching of objects from multiple images\. |

**Return Value **: 

`(pair_id, item_id)` tuple list, where each tuple associates a detection item in the input with a cross\-frame pairing\.
`pair_id` is a cross\-frame unique pairing identifier generated by the server, and detection items of the same object in different frames share the same `pair_id`;
`item_id` corresponds to `ObjectDetection.uuid`, which can be directly used to look up the original detection item in the input `objs`\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest, OvdEndpoint

robot = BajieRobot("10.88.41.120", auto_connect=True)
view = robot.eco_captureImages(CameraType.HEAD)
objs = list(robot.eco_detect_objects(
    DetectObjectsRequest(rgb_image=view.rgb_image.img, labels=["鞋子"], entry=OvdEndpoint.SHOE),
).items)
links = robot.eco_pairwise([(view.rgb_image.img, objs)])
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o view.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(view.json).rgb_image.img, "labels": ["鞋子"], "entry": "SHOE"}}' -o objs.json
## 第3步：eco_pairwise
bajie_sdk run eco_pairwise --json '{"frames": [[@(view.json).rgb_image.img, @(objs.json).items]]}' -o links.json
```

#### 5\.1\.5 `eco_shoe_placement_plan`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_shoe_placement_plan(view: 'RGBDViewWithPose', place_region: 'Sequence[Sequence[float]]', front_edge: 'Sequence[Sequence[float]]', paired_bbox: 'Sequence[PairedBBox]', *, move_all: 'bool' = True, timeout: 'float' = 20.0) -> 'List[MovePlanStep]'
```

**Overview**: 

Generate shoe placement plans based on shoe detection boxes and placement areas\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`view`|`RGBDViewWithPose` sequence\.|
|`place_region`|Placement area polygon vertex sequence \(map coordinate system, list of 3D points\)\.|
|`front_edge`|The forward edge polyline is part of the place\_region \(navigation points will be generated in front of one side of this edge\)\. |
|`paired_bbox`|`PairedBBox` list \(`bbox`, `pair_id`, optional `is_tilted`\)\. |
|`move_all`|`True` All shoes are planned and organized, `False` Those that meet the criteria are not organized\. |
|`timeout`|Timeout \(seconds\), which applies to both projection and placement planning\.|

**Return value **: 

`MovePlanStep` 列表（`step_id`、`from_bbox`、`to_bbox`）。

**Python Example**: 

```Python
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

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o view.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(view.json).rgb_image.img, "labels": ["鞋子"], "entry": "SHOE"}}' -o obs.json
## 第3步：eco_shoe_placement_plan
bajie_sdk run eco_shoe_placement_plan --json '{"view": @(view.json), "place_region": [[1.2, 0.6, 0.0], [1.2, -0.6, 0.0], [0.4, -0.6, 0.0], [0.4, 0.6, 0.0]], "front_edge": [[0.4, -0.6, 0.0], [0.4, 0.6, 0.0]], "paired_bbox": "items", "timeout": 15.0}' -o steps.json
```

#### 5\.1\.6 `eco_vlm_desk_sort_plan`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_vlm_desk_sort_plan(req: 'VlmPlanRequest', *, timeout: 'float' = 10.0) -> 'VlmPlanResponse'
```

**Overview**: 

VLM Desktop Organization Planning: Generate a step\-by\-step pick\-and\-place plan based on images, user intent, and perception results\.`judge_input` and `memory_input` are optional inputs, used for organizing evaluation results and memory information such as user historical preferences, respectively\.

> VLM \- type interfaces can only be used after setting the VLM model API in the Bajie Robot Web configuration page\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|Request Body ` VlmPlanRequest `|
|`timeout`|Timeout \(seconds\)\.|

**" ****`Req `****" subfield **:

|Field|Description|
|---|---|
|`image`|RGB image \(`numpy.ndarray`\);|
|`user_input`|User intent description;|
|`perception`|Perception result list \(`Sequence[NamedBBox]`\);|
|`judge_input`|Optional, input for evaluation of sorting effectiveness;|
|`memory_input`|Optional, memory input\.|

**Return Value **: 

`VlmPlanResponse`: `user_input` echo, `reason` inference process, `steps` step\-by\-step pick\-and\-place step list\.

**Python Example**: 

```Python
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

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o img.json
## 第2步：eco_vlm_perception
bajie_sdk run eco_vlm_perception --json '{"image": @(img.json).rgb_image.img}' -o items.json
## 第3步：eco_vlm_desk_sort_plan
bajie_sdk run eco_vlm_desk_sort_plan --json '{"req": {"image": @(img.json).rgb_image.img, "user_input": "整理桌面", "perception": @(items.json)}}' -o result.json
```

#### 5\.1\.7 `eco_vlm_perception`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_vlm_perception(image: 'np.ndarray', *, timeout: 'float' = 10.0) -> 'List[NamedBBox]'
```

**Overview**: 

VLM Desktop Object Perception: Input a desktop image, output a list of object ` name ` and ` bbox `\. 

> The VLM class interface can only be used after setting the VLM model API in the Bajie Robot Web configuration page\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`image`|RGB `ndarray`。|
|`timeout`|Timeout \(seconds\)\.|

**Return Value **: 

`NamedBBox` list \(can pass `eco_vlm_desk_sort_plan` as `perception` parameter\)\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
img = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
items = robot.eco_vlm_perception(img)
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o img.json
## 第2步：eco_vlm_perception
bajie_sdk run eco_vlm_perception --json '{"image": @(img.json).rgb_image.img}' -o items.json
```

#### 5\.1\.8 `eco_vlm_match`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_vlm_match(req: 'DeskIntentMatchRequest', *, timeout: 'float' = 10.0) -> 'tuple[int, int, int, int]'
```

**Overview**: 

VLM Desktop Matching: The object bounding box in the head camera is mapped to the bounding box in the hand camera image\. 

> VLM \- type interfaces can only be used after setting the VLM model API in the Bajie Robot Web configuration page\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|Structural parameters:`arm_image` \(arm RGB\), `base_image` \(head RGB\), `bbox` \(head image `[x1,y1,x2,y2]`\)\. |
|`timeout`|Timeout \(seconds\)\.|

**Return Value **: 

Pixel box under the hand camera`(x1, y1, x2, y2)`\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType, DeskIntentMatchRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_view = robot.eco_captureImages(CameraType.HEAD)
arm_view = robot.eco_captureImages(CameraType.ARM)
bbox = robot.eco_vlm_match(
    DeskIntentMatchRequest(arm_image=arm_view.rgb_image.img, base_image=head_view.rgb_image.img, bbox=(100, 50, 300, 250)),
    timeout=15.0,
)
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o arm_view.json
## 第3步：eco_vlm_match
bajie_sdk run eco_vlm_match --json '{"req": {"arm_image": @(arm_view.json).rgb_image.img, "base_image": @(head_view.json).rgb_image.img, "bbox": [100, 50, 300, 250]}, "timeout": 15.0}' -o bbox.json
```

#### 5\.1\.9 `eco_vlm_suggest_angle`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_vlm_suggest_angle(image_angles: 'Sequence[tuple[np.ndarray, float]]', *, timeout: 'float' = 10.0) -> 'float'
```

**Overview**: 

Recommended VLM Head Observation Angle: Input multi\-angle desktop images and corresponding camera angles, output the head angle with the best field of view\.

> VLM \- type interfaces can only be used after setting the VLM model API in the Bajie Robot Web configuration page\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`image_angles`|`[(image, angle),...]`, where each item is `(RGB ndarray, head angle in radians)`\.|
|`timeout`|Timeout \(seconds\)\.|

**Return Value **: 

Recommended head angle \(radians\)\.

**Python Example**: 

```Python
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

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o view1.json
## 第2步：eco_setRobotHead
bajie_sdk run eco_setRobotHead --json '{"value": 0.8}'
## 第3步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o view2.json
## 第4步：eco_vlm_suggest_angle
bajie_sdk run eco_vlm_suggest_angle --json '{"image_angles": [[@(view1.json).rgb_image.img, 0.3], [@(view2.json).rgb_image.img, 0.8]]}' -o angle.json
```

#### 5\.1\.10 `eco_vlm_judge`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_vlm_judge(tidy_image: 'np.ndarray', user_input: 'str', *, timeout: 'float' = 10.0) -> 'VlmJudgeResponse'
```

**Overview**: 

VLM Sorting Effect Evaluation: Input the sorted desktop image and user requirements, and determine whether they meet the requirements\. 

> The VLM class interface can only be used after setting the VLM model API in the Bajie Robot Web configuration page\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`tidy_image`|RGB `ndarray` \(desktop image retaken after organization\)\. |
|`user_input`|User requirement statement \(should be consistent with that in `eco_vlm_desk_sort_plan`\)\.|
|`timeout`|Timeout \(seconds\)\.|

**Return Value **: 

`VlmJudgeResponse`：

- `is_tidy` — Whether it meets the tidying requirement \(`bool`\);

- `reason` \- Reason for non\-compliance or evaluation description \(`str`\);

- `score` — Score of the state after organization, ranging from 0 to 1 \(`float`\)\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
tidy_img = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
result = robot.eco_vlm_judge(tidy_img, "整理桌面")
if not result.is_tidy:
    print(f"需进一步整理: {result.reason}")
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o tidy_img.json
## 第2步：eco_vlm_judge
bajie_sdk run eco_vlm_judge --json '{"tidy_image": @(tidy_img.json).rgb_image.img, "user_input": "整理桌面"}' -o result.json
```

#### 5\.1\.11 `eco_put_where_summary`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_put_where_summary(req: 'PutWhereSummaryRequest', *, timeout: 'float' = 30.0) -> 'List[float]'
```

**Overview**: 

Teach PutWhere Summary\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|`PutWhereSummaryRequest`|
|`timeout`|Timeout \(seconds\)\.|

**The ****` req `**** subfield ** : 

|Field|Description|
|---|---|
|`carrier`|Carrier Name;|
|`carrier_direct`|Carrier orientation description;|
|`image`|Scene Graph`numpy.ndarray`;|
|`obj_image`|Target image`numpy.ndarray`;|
|`placeholder`|Placeholder description string; |
|`summary`|Task description\. |

**Return Value **: 

Recommended placement pixel coordinates: length **4** \(carrier box\) or **2** \(single point\)\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType, PutWhereSummaryRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
scene = robot.eco_captureImages(CameraType.HEAD).rgb_image.img
obj = robot.eco_captureImages(CameraType.ARM).rgb_image.img
r = robot.eco_put_where_summary(PutWhereSummaryRequest(
    carrier="桌子", carrier_direct="前", image=scene, obj_image=obj,
    placeholder="桌子", summary="桌面整理",
))
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o scene.json
## 第2步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o obj.json
## 第3步：eco_put_where_summary
bajie_sdk run eco_put_where_summary --json '{"req": {"carrier": "桌子", "carrier_direct": "前", "image": @(scene.json).rgb_image.img, "obj_image": @(obj.json).rgb_image.img, "placeholder": "桌子", "summary": "桌面整理"}}' -o r.json
```

### 5\.2 Mapping and Navigation

This section of the interface requires ` robot.Connect() ` first\. 

#### 5\.2\.1 `eco_autoMapBuild`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_autoMapBuild(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Automatic Mapping \(Fully Automatic SLAM \+ Semantic Mapping\)\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`timeout_sec`|The maximum number of seconds to wait for the task to end \( ` finish ` \)\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

Returns` MissionStatus ` when ` on_task_done ` is not passed; returns ` task_id ` when passed\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, Command
import time

robot = BajieRobot("10.88.41.120", auto_connect=True)
task_id = robot.eco_autoMapBuild(timeout_sec=120.0, on_task_done=lambda _: None)
time.sleep(10)
robot.eco_missionControl(Command.CANCEL, "eco_autoMapBuild", task_id)
```

**CLI Example**: 

```Bash
## 第1步：eco_autoMapBuild
bajie_sdk run eco_autoMapBuild --json '{"timeout_sec": 120.0}' --background -o task_id.json
sleep 10
## 第2步：eco_missionControl
bajie_sdk run eco_missionControl --json '{"cmd": "CANCEL", "task_id": @(task_id.json), "arg2": @(task_id.json)}'
```

#### 5\.2\.2 `eco_semanticMapBuild`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_semanticMapBuild(area_info: 'Sequence[AreaInfo]', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Semantic Mapping\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`area_info`|`AreaInfo` sequence, each item contains `area_id`, `area_name`\. |
|`timeout_sec`|The maximum number of seconds to wait for ` finish `\. |
|`on_task_done`|If given, a callback is triggered at the end of the task \(without blocking until finish\), and this method immediately returns `task_id`\. |

**Return Value **: 

Returns` MissionStatus ` when ` on_task_done ` is not passed; returns ` task_id ` when passed\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, AreaInfo

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_semanticMapBuild([AreaInfo(area_id="r1", area_name="客厅")])
```

**CLI Example**: 

```Bash
bajie_sdk run eco_semanticMapBuild --json '{"area_info": [{"area_id": "r1", "area_name": "客厅"}]}' -o st.json
```

#### 5\.2\.3 `eco_manageSemanticMap`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_manageSemanticMap(cmd: 'Union[SemanticMapManagerCmd, int]', object_info: 'SemanticMapManagerObjectInfo', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[SemanticMapManagerResponse], None]]' = None) -> 'Union[SemanticMapManagerResponse, str]'
```

**Overview**: 

Semantic map addition, deletion, modification, and query\.

> Note:

1. When ADD, the id is a new UUID \(this id will appear in the eco\_robotFurniture list after successful addition\)\. 

2. When QUERY / MODIFY / DELETE`SemanticMapManagerObjectInfo.id` is sourced from `eco_robotFurniture()` returns `FurnitureItem.fid`\. When QUERY, it can be null \(query all\) or filled with fid \(query a single item\)\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`cmd`|`SemanticMapManagerCmd` \(such as `ADD` / `QUERY` / `MODIFY` / `DELETE`, etc\.\) or an integer compatible with the enumeration\.|
|`object_info`|`SemanticMapManagerObjectInfo`（`id`, `model_level`, `mssid`, `model_name`, `name`, `content`, `direction`, `app_is_custom`）。|
|`timeout_sec`|Maximum number of seconds to wait for a response/end\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

When `on_task_done` is not passed, returns `SemanticMapManagerResponse`; when passed, returns `task_id`\.

**Python Example**: 

```Python
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

**CLI Example**: 

```Bash
## 查询全部
bajie_sdk run eco_manageSemanticMap --json '{"cmd": "QUERY", "object_info": {}}' -o resp.json
## 按 fid 查询
bajie_sdk run eco_manageSemanticMap --json '{"cmd": "QUERY", "object_info": {"id": "fid"}}' -o resp.json
## 添加新家具
bajie_sdk run eco_manageSemanticMap --json '{"cmd": "ADD", "object_info": {"id": {}, "model_level": "CARRIER", "model_name": "桌子", "direction": [[1.0, 0.0], [1.0, 1.0]], "content": [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]], "name": "书桌"}}'
```

#### 5\.2\.4 `eco_moveChassis`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_moveChassis(move_distance: 'float', move_angle: 'float', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Chassis relative motion\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`move_distance`|Translation distance \(m\)|
|`move_angle`|Rotation angle \(rad\)\.|
|`timeout_sec`|The maximum number of seconds to wait for ` finish `\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

Returns` MissionStatus ` when ` on_task_done ` is not passed; returns ` task_id ` when passed\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_moveChassis(0.1, 0.0)  # 约前进 10 cm
```

**CLI Example**: 

```Bash
bajie_sdk run eco_moveChassis --json '{"move_distance": 0.1, "move_angle": 0.0}' -o st.json
```

#### 5\.2\.5 `eco_navigateToPoint`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_navigateToPoint(req: 'PointNavigationRequest', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Fixed\-point navigation in the map coordinate system\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|`PointNavigationRequest`（`x`, `y`, `yaw`）|
|`timeout_sec`|The maximum number of seconds to wait ` for finish `\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

Returns` MissionStatus ` when ` on_task_done ` is not passed; returns ` task_id ` when passed\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, PointNavigationRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_navigateToPoint(PointNavigationRequest(x=1.0, y=2.0, yaw=0.0))
```

**CLI Example**: 

```Bash
bajie_sdk run eco_navigateToPoint --json '{"req": {"x": 1.0, "y": 2.0, "yaw": 0.0}}' -o st.json
```

#### 5\.2\.6 `eco_navigateToSemanticArea`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_navigateToSemanticArea(req: 'SemanticNavigationRequest', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Semantic Area Navigation\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|`SemanticNavigationRequest`（`area_id`, `area_name`）|
|`timeout_sec`|The maximum number of seconds to wait for ` finish `\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

Returns` MissionStatus ` when ` on_task_done ` is not passed; returns ` task_id ` when passed\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, SemanticNavigationRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_navigateToSemanticArea(
    SemanticNavigationRequest(area_id="", area_name="客厅")
)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_navigateToSemanticArea --json '{"req": {"area_id": "", "area_name": "客厅"}}' -o st.json
```

#### 5\.2\.7 `eco_leaveDock`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_leaveDock(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Unplug the charging cable \(leave the charging station\)\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_leaveDock()
```

**CLI Example**: 

```Bash
bajie_sdk run eco_leaveDock --json '{"timeout_sec": 30.0}' -o st.json
```

#### 5\.2\.8 `eco_relocate`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_relocate(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Relocate\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_relocate()
```

**CLI Example**: 

```Bash
bajie_sdk run eco_relocate --json '{"timeout_sec": 30.0}' -o st.json
```

#### 5\.2\.9 `eco_startRecharge`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_startRecharge(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Return to charging\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, Command
import time

robot = BajieRobot("10.88.41.120", auto_connect=True)
task_id = robot.eco_startRecharge(timeout_sec=300.0, on_task_done=lambda _: None)
time.sleep(10)
robot.eco_missionControl(Command.CANCEL, "eco_startRecharge", task_id)
```

**CLI Example**: 

```Bash
## 第1步：eco_startRecharge
bajie_sdk run eco_startRecharge --json '{"timeout_sec": 300.0}' --background -o task_id.json
sleep 10
## 第2步：eco_missionControl
bajie_sdk run eco_missionControl --json '{"cmd": "CANCEL", "task_id": @(task_id.json), "arg2": @(task_id.json)}'
```

### 5\.3 Image and Pose

This section of the interface requires ` robot.Connect() ` first\. 

#### 5\.3\.1 `eco_captureImages`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_captureImages(camera_type: 'Union[CameraType, int]', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[RGBDViewWithPose], None]]' = None) -> 'Union[RGBDViewWithPose, str]'
```

**Overview**: 

Acquire a frame of camera image\.`camera_type`: `CameraType.ARM`\(1\) is the arm camera, `CameraType.HEAD`\(2\) is the head camera\.

> `RGBDViewWithPose.tf_goal` coordinate system depends on the camera type:
**Head Camera \(HEAD\)**'s `tf_goal` is in the `map` coordinate system;
**Arm Camera \(ARM\)**'s `tf_goal` is in the `base_footprint` coordinate system\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`camera_type`|`CameraType` or an integer compatible with the enumeration\.|
|`timeout_sec`|The maximum number of seconds to wait for the image ` finish `\. |
|`on_task_done`|If given, a callback is triggered at the end of the task \(without blocking until finish\), and this method immediately returns `task_id`\. |

**Return Value **: 

Returns `RGBDViewWithPose `when no `on_task_done `is passed; returns `task_id `when passed\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
img = robot.eco_captureImages(CameraType.ARM)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o img.json
```

#### 5\.3\.2 `eco_computeObjectPose`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_computeObjectPose(named_bbox: 'Union[Sequence[NamedBBox], Sequence[tuple[int, int, int, int]]]', view: 'RGBDViewWithPose', *, timeout_sec: 'float' = 6.0, on_task_done: 'Optional[Callable[[List[ObjectPose3D]], None]]' = None) -> 'Union[List[ObjectPose3D], str]'
```

**Overview**: 

Calculate the 6D pose of the object within the bounding box based on the bounding box, RGBD, and map coordinate system pose\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`named_bbox`|`NamedBBox` 列表，或 `list[tuple[int,int,int,int]]`。|
|`view`|`RGBDViewWithPose`（`rgb_image` / `depth_image` / `tf_goal`）。|
|`timeout_sec`|The maximum number of seconds to wait for ` pose_results ` in ` finish `\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

When not passing `on_task_done`, it returns `list[ObjectPose3D]`; when passing it, it returns `task_id`\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType, NamedBBox

robot = BajieRobot("10.88.41.120", auto_connect=True)
view = robot.eco_captureImages(CameraType.ARM)
poses = robot.eco_computeObjectPose(
    named_bbox=[NamedBBox(id=0, name="obj", bbox=(10, 20, 100, 200))],
    view=view, timeout_sec=5.0,
)
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o view.json
## 第2步：eco_computeObjectPose
bajie_sdk run eco_computeObjectPose --json '{"named_bbox": [{"id": 0, "name": "obj", "bbox": [10, 20, 100, 200]}], "view": @(view.json), "timeout_sec": 5.0}' -o poses.json
```

### 5\.4 High\-level Tasks

This section of the interface requires ` robot.Connect() ` first\. 

#### 5\.4\.1 `eco_locatePerson`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_locatePerson(req: 'FindPersonRequest', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Search for people in the specified area\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|`FindPersonRequest`（`user_name`, `user_id`）。|
|`timeout_sec`|The maximum number of seconds to wait for ` finish `\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

Returns` MissionStatus ` when ` on_task_done ` is not passed; returns ` task_id ` when passed\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, FindPersonRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_locatePerson(FindPersonRequest(user_name="张三"))
```

**CLI Example**: 

```Bash
bajie_sdk run eco_locatePerson --json '{"req": {"user_name": "张三"}}' -o st.json
```

#### 5\.4\.2 `eco_robotSpeech`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_robotSpeech(text: 'str') -> 'str'
```

**Overview**: 

App Voice Announcement \(TTS\), fire\-and\-forget mode\.

Immediately return after sending the broadcast request without waiting for the server's response\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`text`|Broadcast content\.|

**Return Value **: 

task\_id \(format `speech_<uuid_hex>`\) is used to track this broadcast request\.
:raises RuntimeError: Raised when sending fails\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot

robot = BajieRobot("10.88.41.120", auto_connect=True)
tid = robot.eco_robotSpeech("请让一让")
```

**CLI Example**: 

```Bash
bajie_sdk run eco_robotSpeech --json '{"text": "请让一让"}' -o tid.json
```

#### 5\.4\.3 `eco_findObject`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_findObject(req: 'SearchRequest', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[SearchResponse], None]]' = None) -> 'Union[SearchResponse, str]'
```

**Overview**: 

Search for objects within the area; you can pass the 6D box of the storage basket in `filter_boxes` to exclude the objects already stored inside the basket\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|`SearchRequest`：`object`\(`ObjectInfo`\)、`area`\(`AreaInfo`\)、`filter_boxes`\(可选 `ObjectPose3D` 列表\)。|
|`timeout_sec`|The maximum number of seconds to wait for the search results ` finish `\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

Returns `SearchResponse` when `on_task_done` is not passed; returns `task_id` when passed\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, SearchRequest, ObjectInfo, AreaInfo

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_findObject(
    SearchRequest(object=ObjectInfo(obj_name="玩具"), area=AreaInfo(area_name="玩具收纳区"))
)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_findObject --json '{"req": {"object": {"obj_name": "玩具"}, "area": {"area_name": "玩具收纳区"}}}' -o st.json
```

#### 5\.4\.4 `eco_prepareRobotPose`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_prepareRobotPose(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

The fuselage enters the mission preparation attitude \(invoked at the start of the mission\)\. 

**Python Example**: 

```Python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_prepareRobotPose(timeout_sec=60.0)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_prepareRobotPose --json '{"timeout_sec": 60.0}' -o st.json
```

#### 5\.4\.5 `eco_finishRobotPose`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_finishRobotPose(*, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Restore the aircraft's end attitude \(called when the mission ends\)\. 

**Python Example**: 

```Python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_finishRobotPose()
```

**CLI Example**: 

```Bash
bajie_sdk run eco_finishRobotPose --json '{"timeout_sec": 30.0}' -o st.json
```

#### 5\.4\.6 `eco_lookto`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_lookto(req: 'Union[ObjectPose3D, tuple[int, int, int, int]]', *, view: 'Optional[RGBDViewWithPose]' = None, camera_type: 'Union[CameraType, int]' = <CameraType.ARM: 1>, timeout_sec: 'float' = 30.0) -> 'MissionStatus'
```

**Overview**: 

Observation alignment: Unified hand/head` look to ` capability\. When passing bbox, it needs to be coordinated with ` view `, and the SDK will first use ` eco_computeObjectPose ` to calculate the pose \(internal fixed timeout of 6 seconds\)\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|`ObjectPose3D` 或单 bbox `(x1, y1, x2, y2)`。|
|`view`|Required when `req` is bbox, corresponding to the RGBD view required for pose estimation\.|
|`camera_type`|Observation camera type;`CameraType.ARM` for hand observation,`CameraType.HEAD` for head observation\.|
|`timeout_sec`|The maximum number of seconds to wait for ` finish `\. |

**Return Value **: 

`MissionStatus` Mission status\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_view = robot.eco_captureImages(CameraType.HEAD)
st = robot.eco_lookto((120, 80, 260, 300), view=head_view, camera_type=CameraType.ARM, timeout_sec=30.0)
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_lookto
bajie_sdk run eco_lookto --json '{"req": [120, 80, 260, 300], "view": @(head_view.json), "camera_type": "ARM", "timeout_sec": 30.0}' -o st.json
```

#### 5\.4\.7 `eco_pick`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_pick(view: 'RGBDViewWithPose', bbox: 'NamedBBox', *, timeout_sec: 'float' = 300.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Grasping: Perform precise grasping\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`view`|Arm Camera`RGBDViewWithPose` \(rgb/depth/tf\_goal\)\. |
|`bbox`|`NamedBBox` \( `name`, four corner pixel coordinates, etc\. \) |
|`timeout_sec`|Maximum number of seconds to wait ` for finish ` \(crawling may be slow\)\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

Returns` MissionStatus ` when ` on_task_done ` is not passed; returns ` task_id ` when passed\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
arm_view = robot.eco_captureImages(CameraType.ARM)
items = list(robot.eco_detect_objects(
    DetectObjectsRequest(rgb_image=arm_view.rgb_image.img, labels=["玩具"]),
).items)
if items:
    st = robot.eco_pick(arm_view, items[0].to_named_bbox_for_grab(), timeout_sec=300.0)
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o arm_view.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(arm_view.json).rgb_image.img, "labels": ["玩具"]}}' -o items.json
## 第3步：eco_pick
bajie_sdk run eco_pick --json '{"view": @(arm_view.json), "bbox": @(items.json).items[0], "timeout_sec": 300.0}' -o st.json
```

#### 5\.4\.8 `eco_match_obj_views`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_match_obj_views(source_img: 'np.ndarray', ref_bbox: 'tuple[int, int, int, int]', target_img: 'np.ndarray', target_detections: 'Sequence[ObjectDetection]') -> 'NamedBBox'
```

**Overview**: 

Cross\-view matching target box: Find the item most similar to the reference box in the Object Detection list\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`source_img`|Reference perspective RGB image \(usually a head view\)\.|
|`ref_bbox`|Reference Box `(x1, y1, x2, y2)`\. |
|`target_img`|Target perspective RGB image \(usually an arm view\)\.|
|`target_detections`|Target Perspective Detection Results List \(`Object Detection`\); at least 1 item\.|

**Return Value **: 

The item with the closest semantic meaning to the reference frame ` NamedBBox ` \(used for subsequent precise grasping\)\. 

**Python Example**: 

```Python
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

**CLI Example**: 

```Bash
## 第1步：eco_captureImages (HEAD)
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_captureImages (ARM)
bajie_sdk run eco_captureImages --json '{"camera_type": "ARM"}' -o arm_view.json
## 第3步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(arm_view.json).rgb_image.img, "labels": ["玩具"]}}' -o arm_items.json
## 第4步：eco_match_obj_views
bajie_sdk run eco_match_obj_views --json '{"source_img": @(head_view.json).rgb_image.img, "ref_bbox": [120, 80, 260, 300], "target_img": @(arm_view.json).rgb_image.img, "target_detections": @(arm_items.json).items}' -o best.json
```

#### 5\.4\.9 `eco_pick_with_arm_review`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_pick_with_arm_review(view: 'RGBDViewWithPose', object_bbox: 'NamedBBox', *, timeout_sec: 'float' = 100.0) -> 'MissionStatus'
```

**Overview**: 

Grasping \(Arm Perspective\): After hand observation, recheck and match on the arm diagram, then execute precise grasping\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`view`|Reference View `RGBDViewWithPose` \(usually the result of head photography\)\. |
|`object_bbox`|`NamedBBox`: `id`, `name` \(used for arm diagram detection labels; falls back to `object` when empty string\), `bbox`\. |
|`timeout_sec`|Single mission timeout \(seconds\)\.|

**Return Value **: 

`MissionStatus` Mission Status\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType, DetectObjectsRequest

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_view = robot.eco_captureImages(CameraType.HEAD)
items = list(robot.eco_detect_objects(
    DetectObjectsRequest(rgb_image=head_view.rgb_image.img, labels=["玩具"]),
).items)
if items:
    st = robot.eco_pick_with_arm_review(head_view, items[0].to_named_bbox_for_grab(), timeout_sec=120.0)
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_view.json
## 第2步：eco_detect_objects
bajie_sdk run eco_detect_objects --json '{"req": {"rgb_image": @(head_view.json).rgb_image.img, "labels": ["玩具"]}}' -o items.json
## 第3步：eco_pick_with_arm_review
bajie_sdk run eco_pick_with_arm_review --json '{"view": @(head_view.json), "object_bbox": @(items.json).items[0], "timeout_sec": 120.0}' -o st.json
```

#### 5\.4\.10 `eco_place_3D`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_place_3D(req: 'ObjectPose3D', *, timeout_sec: 'float' = 300.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Precise Placement: Automatically navigate and complete placement according to 6D pose\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`req`|`ObjectPose3D`：`position`\(`Vec3f`\)、`orientation`\(`Quatf`\)、`box_length`\(`Vec3f`\)、`frame_id`。|
|`timeout_sec`|The maximum number of seconds to wait for ` finish `\. |
|`on_task_done`|If given, a callback will be triggered at the end of the task \(without blocking until finish\), and this method will immediately return `task_id`\. |

**Return Value **: 

Returns` MissionStatus ` when ` on_task_done ` is not passed; returns ` task_id ` when passed\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, ObjectPose3D, Vec3f, Quatf

robot = BajieRobot("10.88.41.120", auto_connect=True)
req = ObjectPose3D(position=Vec3f(x=1.0, y=2.0, z=0.0), orientation=Quatf(),
                   box_length=Vec3f(0.12, 0.1, 0.05), frame_id="map")
st = robot.eco_place_3D(req)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_place_3D --json '{"req": {"position": {"x": 1.0, "y": 2.0, "z": 0.0}, "orientation": {}, "box_length": {}, "frame_id": "map"}}' -o st.json
```

#### 5\.4\.11 `eco_place_with_view`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_place_with_view(view: 'RGBDViewWithPose', place_ref: 'Sequence[float]', recapture_place: 'bool' = False, *, timeout_sec: 'float' = 300.0) -> 'MissionStatus'
```

**Overview**: 

Placement:`recapture_place=False` performs semantic placement;`True` performs precise placement based on reprojection\.

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`view`|Header`RGBDViewWithPose`\.|
|`place_ref`|Length **2** \(single point\) or length **4** \(box, taking the center point\) pixel coordinates\.|
|`recapture_place`|`False` Semantic placement;`True` Precise placement\.|
|`timeout_sec`|The maximum number of seconds to wait for ` finish `\. |

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, CameraType

robot = BajieRobot("10.88.41.120", auto_connect=True)
head_rgbd = robot.eco_captureImages(CameraType.HEAD)
st = robot.eco_place_with_view(head_rgbd, [153.0, 230.0, 403.0, 384.0], recapture_place=False)
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_rgbd.json
## 第2步：eco_place_with_view
bajie_sdk run eco_place_with_view --json '{"view": @(head_rgbd.json), "place_ref": [153.0, 230.0, 403.0, 384.0], "recapture_place": false}' -o st.json
```

#### 5\.4\.12 `eco_place_in`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_place_in(view: 'RGBDViewWithPose', carrier_bbox: 'Sequence[float]', *, timeout_sec: 'float' = 300.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Recommended content in the container:`carrier_bbox` Take the first 4 items as the container box, and use them according to `[x1, y1, x2, y2]`\.

**Python Example**: 

```Python
robot = BajieRobot("10.88.41.120", auto_connect=True)
head_rgbd = robot.eco_captureImages(CameraType.HEAD)
st = robot.eco_place_in(head_rgbd, [153.0, 230.0, 403.0, 384.0])
```

**CLI Example**: 

```Bash
## 第1步：eco_captureImages
bajie_sdk run eco_captureImages --json '{"camera_type": "HEAD"}' -o head_rgbd.json
## 第2步：eco_place_in
bajie_sdk run eco_place_in --json '{"view": @(head_rgbd.json), "carrier_bbox": [153.0, 230.0, 403.0, 384.0]}' -o st.json
```

### 5\.5 Ontology Control

This section of the interface requires ` robot.Connect() ` first\. 

#### 5\.5\.1 `eco_setRobotHeight`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_setRobotHeight(value: 'float', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Lift height control\. ` value ` range \[0, 0\.44\] meters; 0 = lowest, 0\.44 = highest; out\-of\-range values are automatically truncated and an alarm is issued\. 

**Python Example**: 

```Python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_setRobotHeight(0.22)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_setRobotHeight --json '{"value": 0.22}' -o st.json
```

#### 5\.5\.2 `eco_setRobotHead`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_setRobotHead(value: 'float', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Head pitch control\.`value` range \[0, 1\.3\] radians; 0 = lowest, 0\.83 = horizontal, 1\.3 = highest; out\-of\-range values are automatically truncated and an alarm is triggered\.

**Python Example**: 

```Python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_setRobotHead(0.83)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_setRobotHead --json '{"value": 0.83}' -o st.json
```

#### 5\.5\.3 `eco_setRobotArmMode`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_setRobotArmMode(mode: 'Union[RobotArmCtrlMode, int]', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Mechanical Arm/Gripper Mode Control\.

`mode` values: `ARM_HOME(2)` Home / `GRIPPER_OPEN(6)` Open / `GRIPPER_CLOSE(7)` Close / `UNBOX_POSE(9)` Unboxing Pose / `GRIPPER_MAINTENANCE(40)` Maintenance Pose\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, RobotArmCtrlMode

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_setRobotArmMode(RobotArmCtrlMode.ARM_HOME)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_setRobotArmMode --json '{"mode": "ARM_HOME"}' -o st.json
```

#### 5\.5\.4 `eco_setRobotPose`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_setRobotPose(object_pose: 'Sequence[PoseFrame]', *, timeout_sec: 'float' = 30.0, on_task_done: 'Optional[Callable[[MissionStatus], None]]' = None) -> 'Union[MissionStatus, str]'
```

**Overview**: 

Manipulator pose control\. Each item uses `header.frame_id`, `position`, `orientation`\. `header.frame_id` is recommended to use the camera coordinate system \(e\.g\., `arm_camera_depth_optical_frame`\)\. 

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, PoseFrame, FrameHeader, Vec3f, Quatf

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_setRobotPose([
    PoseFrame(header=FrameHeader(frame_id="arm_camera_color_optical_frame"),
              position=Vec3f(0.3, 0.0, 0.5), orientation=Quatf()),
])
```

**CLI Example**: 

```Bash
bajie_sdk run eco_setRobotPose --json '{"object_pose": [{"header": {"frame_id": "arm_camera_color_optical_frame"}, "position": {}, "orientation": {}}]}' -o st.json
```

### 5\.6 Task Pause / Resume / Cancel

This section of the interface requires ` robot.Connect() ` first\. 

#### 5\.6\.1 `eco_cancelAllMissions`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_cancelAllMissions(*, timeout_sec: 'float' = 30.0) -> 'MissionStatus'
```

**Overview**: 

Cancel all tasks\. 

**Python Example**: 

```Python
robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_cancelAllMissions()
```

**CLI Example**: 

```Bash
bajie_sdk run eco_cancelAllMissions --json '{"timeout_sec": 30.0}' -o st.json
```

#### 5\.6\.2 `eco_missionControl`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_missionControl(cmd: 'Command', task_id: 'str', *, timeout_sec: 'float' = 30.0) -> 'MissionStatus'
```

**Overview**: 

Unified mission control: pause, resume, or cancel\.

> Starting from SDK 0\.3\.0, the return type has been upgraded from `bool` to `MissionStatus`, which includes error codes and version information\. 

**Field** \(Request / Input Parameter\): 

|Name|Description|
|---|---|
|`cmd`|仅接受 `Command.PAUSE`、`Command.RESUME`、`Command.CANCEL`。|
|`task_id`|Task instance ID, must be consistent with the target task\. |
|`timeout_sec`|Maximum number of seconds to wait for ` response `\. |

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, Command

robot = BajieRobot("10.88.41.120", auto_connect=True)
st = robot.eco_missionControl(Command.PAUSE, "semantic_navigation_0")
st = robot.eco_missionControl(Command.RESUME, "semantic_navigation_0")
st = robot.eco_missionControl(Command.CANCEL, "semantic_navigation_0")
```

**CLI Example**: 

```Bash
bajie_sdk run eco_missionControl --json '{"cmd": "PAUSE", "task_id": "semantic_navigation_0"}' -o st.json
bajie_sdk run eco_missionControl --json '{"cmd": "RESUME", "task_id": "semantic_navigation_0"}' -o st.json
bajie_sdk run eco_missionControl --json '{"cmd": "CANCEL", "task_id": "semantic_navigation_0"}' -o st.json
```

### 5\.7 Machine Status

This section of the interface requires ` robot.Connect() ` first\. After the SDK connects successfully, it automatically retrieves the RobotInfo cache, which is subsequently updated with server\-side pushes\. 

#### 5\.7\.1 `eco_robotInfo`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_robotInfo() -> 'RobotInfo'
```

**Overview**: 

Current merged ` robot_info ` cache snapshot\. 

**Return Value **: 

`RobotInfo`, Field:

- `workState` — `WorkState`（`task_id`、`name`、`cmd`）；

- `battery` — `BatteryInfo`（`value`、`isCharge`、`mode`）；

- `alarm` — `List[int]` List of exception codes;

- `pos` — `Position`（`room`、`x`、`y`、`yaw`）；

- `mapinfo` — `MapInfo` \( `mid`, `mname`, pixel size, resolution, map data, etc\. \); 

- `furniture` — `Furniture` \( `info: List[SemanticMapManagerObjectInfo]`\) All semantic map furniture/area information\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot

robot = BajieRobot("10.88.41.120", auto_connect=True)
info = robot.eco_robotInfo()
if info.workState:
    print(info.workState.cmd)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_robotInfo --json '{}' -o info.json
```

#### 5\.7\.2 `eco_refreshRobotInfo`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_refreshRobotInfo(topics: 'Optional[List[str]]' = None, *, timeout_sec: 'float' = 5.0) -> 'None'
```

**Overview**: 

Manual refresh \(optional subset of `topics `\); legal values are in `RobotInfoTopic `\. Full refresh when `topics `are empty\.

**Python Example**: 

```Python
from bajie_sdk import BajieRobot, RobotInfoTopic

robot = BajieRobot("10.88.41.120", auto_connect=True)
robot.eco_refreshRobotInfo(timeout_sec=5.0)
robot.eco_refreshRobotInfo(
    topics=[RobotInfoTopic.WORK_STATE.value, RobotInfoTopic.BATTERY.value],
    timeout_sec=3.0,
)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_refreshRobotInfo --json '{"timeout_sec": 5.0}'
bajie_sdk run eco_refreshRobotInfo --json '{"topics": ["workState", "battery"], "timeout_sec": 3.0}'
```

#### 5\.7\.3 `eco_robotWorkState` \~ `eco_robotFurniture`

The following interfaces all read the corresponding sub\-states from the cache: 

|Interface|Return Type |Key Field|
|---|---|---|
|`eco_robotWorkState()`|`Optional[WorkState]`|`task_id`, `name`, `cmd`|
|`eco_robotBattery()`|`Optional[BatteryInfo]`|`value`, `isCharge`, `mode`|
|`eco_robotAlarm()`|`Optional[List[int]]`|Exception Code List|
|`eco_robotPosition()`|`Optional[Position]`|`room`, `x`, `y`, `yaw`|
|`eco_robotMapInfo()`|`Optional[MapInfo]`|`mid`, `mname`, `totalWidth`, `totalHeight`, raster data|
|`eco_robotFurniture()`|`Optional[Furniture]`|`info: List[SemanticMapManagerObjectInfo]`|

Returns `None` when not obtained\.

#### 5\.7\.4 `eco_bindRobotInfo` / `eco_unbindRobotInfo`

**Signature**\(`bajie_robot.py`\):

```Python
def eco_bindRobotInfo(callback: 'Callable[[str, Any], None]', fields: 'Optional[List[str]]' = None) -> 'None'
def eco_unbindRobotInfo(callback: 'Callable[[str, Any], None]') -> 'None'
```

**Overview**: 

Bind/Unbind `robot_info` field change callback\. `fields` is `None`, it listens to all fields, in which case `field_name` is `"robot_info"`, `new_value` is a full snapshot\. 

**Python Example**: 

```Python
from typing import Any
from bajie_sdk import BajieRobot

def on_change(field: str, val: Any) -> None:
    print(f"{field} -> {val}")

robot = BajieRobot("10.88.41.120", auto_connect=True)
robot.eco_bindRobotInfo(on_change)                        # 监听全部
robot.eco_bindRobotInfo(on_change, fields=["workState"])  # 仅 workState
robot.eco_unbindRobotInfo(on_change)
```

**CLI Example**: 

```Bash
bajie_sdk run eco_bindRobotInfo --json '{"callback": "on_change"}'
bajie_sdk run eco_bindRobotInfo --json '{"callback": "on_change", "fields": ["workState"]}'
bajie_sdk run eco_unbindRobotInfo --json '{"callback": "on_change"}'
```

---

## 6\. Learning Path Recommendations

If you are using the Python SDK for the first time, it is recommended to gradually master it in the following order: 

|Phase|Objective|Learning Content|
|---|---|---|
|**1\. Installation Verification**|SDK installed|Read §2 Preparation, and complete installation and verification|
|**2\. First connection**|Connect to the robot |Run the first program in §3\.1|
|**3\. Basic Control**|Move \+ Take Photo |Read §5\.2\.4 `eco_moveChassis` and §5\.3\.1 `eco_captureImages`|
|**4\. Proficient in CLI**|Call APIs without writing code|Read §4 CLI Usage Instructions|
|**5\. Perception and Detection**|Identify objects|Read §5\.1\.1 `eco_detect_objects`|
|**6\. Pick and Place**|Complete Pick\-and\-Place Closed Loop |Read §3\.3 Grasp/Place Process and §5\.4|
|**7\. Mapping and Navigation**|Autonomous Robot Movement|Read §5\.2\.1, §5\.2\.5, §5\.2\.6|
|**8\. Desktop Organization \(VLM\)**|AI\-driven organization|Read §5\.1\.6 `eco_vlm_desk_sort_plan` and its upstream and downstream interfaces|
|**9\. Advanced Usage**|Asynchronous Callback, State Subscription|Read §3\.4 Asynchronous Callback Mode, §3\.5 Connection Events and Error Handling, §5\.7 Machine State|

> **Troubleshooting Reference**: Whenever you encounter a problem, first refer to §7 Troubleshooting\.

---

## 7\. Troubleshooting

### 7\.1 `Connect()` Failed

- Check if the robot's IP address is correct\. 

- Confirm whether the computer and the robot are on the same network\.

- Confirm whether the WebSocket address is written as `ws://Robot IP:9900/`\. 

- Check if the robot\-side body control service \(WebSocket/9900\) is running normally\. 

### 7\.2 CLI cannot find the command

First, verify whether the SDK is installed: 

```Bash
python3 -m bajie_sdk -h
```

If it can run, but ` bajie_sdk ` this short command is unavailable, it means the alias/completion is not configured\. You can continue to use: 

```Bash
python3 -m bajie_sdk run eco_robotInfo --json '{}'
```

### 7\.3 Don't know how to write JSON parameters

Priority use:

```Bash
bajie_sdk run eco_xxx --json-template
bajie_sdk -m eco_xxx
bajie_sdk -t TypeName
```

### 7\.4 Task Failed

Priority Printing:

```Python
print(status.task_id)
print(status.error_info)
```

If ` error_info ` contains an error code, first check the error code level, then decide on the handling plan: 

|Level|Meaning |Recommended handling|
|---|---|---|
|`0`|Success |No processing required |
|`1`|Warning/Recoverable Error|Retry 1\-3 times, or check parameters, network, and service response |
|`2`|Error|Conduct a scene\-based investigation and stop the current process if necessary |
|`3`|Serious Error|First, conduct an environmental analysis, remove obstacles or adjust goals, and then retry |

### 7\.5 Task Timeout

**Timeout does not necessarily cancel the task the robot is currently executing**\. If you want to cancel it actively:

```Python
# 取消所有任务
robot.eco_cancelAllMissions()

# 取消指定任务
from bajie_sdk import Command
robot.eco_missionControl(Command.CANCEL, task_id)
```

### 7\.6 Installation Issues

|Question|Process|
|---|---|
|`pip install bajie-sdk` Package not found|PyPI has no public packages, please use the `.whl` file for installation \(see §2\.2\)|
|After installation ` import bajie_sdk ` fails |Confirm that the used `python3` is consistent with the one installed; the wheel must match the platform/Python version|
|Difficulty downloading dependencies during pip installation |Use `pip install --no-deps./bajie_sdk-*.whl` to skip dependency installation|
|`PYTHONPATH` issues \(editable installation\)|In the `python_ws_sdk `directory execute `export PYTHONPATH =. `or `pip install -e.`|

### 7\.7 Quick Reference for Common Error Codes

#### Success and General Status 

|Error Code |Decimal|Level|Meaning |Process|
|---|---|---|---|---|
|`0x00000000`|`0`|`0`|Success |No processing required |
|`0x00000208`|`520`|`-`|General Timeout|Check network, robot service, and current task status |
|`0x00000242`|`578`|`-`|Timeout occurred while retrieving machine status|Retry status query, check connection|
|`0x00000243`|`579`|`-`|The machine is charging and has low battery |Wait for charging or avoid performing high\-power\-consuming tasks |
|`0x00000244`|`580`|`-`|The machine is not charged and has low battery |Priority Recharge|
|`0x00000245`|`581`|`-`|Machine status is not idle|Query `eco_robotWorkState()`, cancel the current task if necessary|
|`0x00000253`|`595`|`-`|Machine is busy or has insufficient battery |Wait for idle, cancel the task, or return to charging first|

#### Task Scheduling

|Error Code |Decimal|Level|Meaning |Process|
|---|---|---|---|---|
|`0x00002000`|`8192`|`1`|Task recovery failed|Confirm whether the task still exists|
|`0x00002001`|`8193`|`1`|Task pause failed|Check if the task is running |
|`0x00002003`|`8195`|`1`|Task is not running|Do not continue to pause/resume/cancel this task|
|`0x00002004`|`8196`|`1`|Invalid task parameters|Check the request structure and field types |
|`0x00002005`|`8197`|`1`|The task is already running |Wait for the current task to finish, or cancel it first |
|`0x0000201c`|`8220`|`1`|The machine is charging |Stake before moving/grabbing|

#### Movement, Mapping, Navigation

|Error Code |Decimal|Level|Meaning |Process|
|---|---|---|---|---|
|`0x00002710`|`10000`|`1`|Chassis movement parameters are invalid|检查 `move_distance`、`move_angle`|
|`0x00002711`|`10001`|`1`|Chassis movement timeout|Check the ground and obstacles |
|`0x00002720`|`10016`|`1`|Navigation parameter is invalid|Check the target point or area parameters |
|`0x00002721`|`10017`|`1`|Navigation task timed out|Check if the target is too far away and if the path is blocked|
|`0x00002730`|`10032`|`1`|SLAM Exploration Service Not Responding|Check the status of the mapping service and retry |
|`0x00002740`|`10048`|`1`|Semantic Exploration Parameter is Invalid|Check Area Information|
|`0x00002770`|`10096`|`1`|Semantic Map Management Parameter is Invalid|Check`id`,`model_level`, contour, and orientation|
|`0x00007002`|`28674`|`3`|Unable to search for the global route |Check the starting point, Critical Path, sensor noise, and obstacles|
|`0x00007004`|`28676`|`3`|Route blocked by an object|Clear the removable obstacles and then try again|
|`0x00007006`|`28678`|`3`|The machine has fallen into a fatal obstacle layer |Check for obstacles around the machine and handle them manually if necessary |

#### Image, Vision, VLM

|Error Code |Decimal|Level|Meaning |Process|
|---|---|---|---|---|
|`0x00002790`|`10128`|`1`|Visual model parameters are invalid|Check the image, bbox, and request structure |
|`0x00002791`|`10129`|`1`|Visual Model Task Timed Out|Check VLM/Visual Service and network, and appropriately increase the timeout|
|`0x00002792`|`10131`|`1`|Visual Model Service Not Responding|Check service status and retry|
|`0x00002793`|`10132`|`1`|No valid object was recognized |Take photos from different angles, adjust labels, and improve lighting|
|`0x000027A0`|`10144`|`1`|Invalid image request parameters|Check` camera_type `|
|`0x000027A1`|`10145`|`1`|Image request timed out|Check the camera service and retry|

#### Robotic Arm, Grasping, Placement

|Error Code |Decimal|Level|Meaning |Process|
|---|---|---|---|---|
|`0x000027E0`|`10208`|`1`|Self\-Adaptation Service for Robotic Arm Observation is Unresponsive|Retry; Check the robotic arm service|
|`0x000027E1`|`10209`|`1`|Self\-Adaptation parameters for robotic arm observation are invalid|检查 bbox、view、camera\_type|
|`0x000027E2`|`10210`|`1`|Self\-Adaptation Timeout for Robotic Arm Observation|Change the angle or try again|
|`0x00002840`|`10304`|`1`|Height Control Task Timeout|Check if the lifting mechanism is blocked|
|`0x00002842`|`10306`|`1`|Height control parameter is invalid|`value` should be within `[0, 0.44]`|
|`0x00009000`|`36864`|`1`|Mechanical arm execution failed|Retry; enter the general exception process after multiple failures|
|`0x00009001`|`36865`|`1`|Mechanical Arm Task Timeout|Check if it is blocked|
|`0x00009002`|`36866`|`1`|Mechanical arm trajectory planning failed|Clear Obstacles|
|`0x00009004`|`36868`|`3`|Target point exceeds the Scope of Work of the robotic arm|Change the target position or realign the robot|
|`0x00009006`|`36870`|`1`|Mechanical arm inverse kinematics solution failed|Re\-observe once and generate a new grasping pose|
|`0x00009015`|`36885`|`1`|The tray is full and cannot be placed |Clean the tray or change its placement position|
|`0x00009021`|`36897`|`1`|The robotic arm failed to grasp the object |Re\-observe the grasping, or invoke the item search/nearby item search logic|

#### Recharge, Unload, Battery Level

|Error Code |Decimal|Level|Meaning |Process|
|---|---|---|---|---|
|`0x00002880`|`10368`|`1`|Return charging task timed out|Check the environment around the charging pile and try again |
|`0x00002881`|`10369`|`1`|Recharge service is unresponsive|Check the status of the recharge service |
|`0x000028b0`|`10416`|`1`|Battery level is too low |Priority Recharge|
|`0x000028b1`|`10417`|`1`|Power reading timeout|Retry reading battery level|
|`0x000028c0`|`10432`|`1`|Piling Timeout |Check if there are any obstacles around the charging pile|
|`0x000028c3`|`10435`|`1`|Abnormal pile driving current |Stop the task and check the charging contact/chassis status|
|`0x00033001`|`208897`|`1`|Reflective sheet and infrared signal not found |Check charging piles, reflective sheets, infrared signals, and obstructions |

