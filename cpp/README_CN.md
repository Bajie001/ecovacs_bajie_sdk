# 八界机器人 SDK 开发文档（C++）

- 本文档面向最终使用者，介绍如何获取并安装 `robot_sdk`（C++）、如何快速调用机器人能力，以及如何查看接口与字段说明。
- 目前为 sdk 的 **1.1.0** 版本。
- 建议开发者选用 ubuntu 系统 x86_64 的 PC 进行开发及调试，其余环境可自行配置，本文以 ubuntu 系统为例进行讲解。

---

## 目录

- 1. 概述
    - 1.1 SDK 是什么
    - 1.2 连接地址
    - 1.3 建图前提
    - 1.4 SDK 技术栈

- 2. 环境要求
    - 2.1 系统与工具
    - 2.2 安装基础构建环境
    - 2.3 交叉编译工具（ARM）
    - 2.4 ARM 版 OpenCV 说明
    - 2.5 SDK 依赖文件

- 3. 获取交付包
    - 3.1 目录结构
    - 3.2 各目录用途

- 4. 构建及运行
    - 4.1 快速构建
    - 4.2 使用 CMake 直接构建
    - 4.3 ARM 交叉编译说明
    - 4.4 运行 demo
    - 4.5 清理构建产物

- 5. 快速开始
    - 5.1 连接与基础调用
    - 5.2 协作式取消（Ctrl+C 响应）
    - 5.3 关于 CLI
    - 5.4 常见 API 调用模式

- 6. `robot_sdk::eco_*` 接口一览（全量）

    - 6.1 感知
        - 6.1.1 `eco_DetectObjects`
        - 6.1.2 `eco_ImageEmbeddingExtract`
        - 6.1.3 `eco_FusedSimilarityHybrid`
        - 6.1.4 `eco_PairwiseMatch`
        - 6.1.5 `eco_ShoesIntentPlan`
        - 6.1.6 `eco_DeskIntentPlan`
        - 6.1.7 `eco_DeskIntentPerception`
        - 6.1.8 `eco_DeskIntentMatch`
        - 6.1.9 `eco_DeskIntentPrePerception`
        - 6.1.10 `eco_DeskIntentJudgeAction`
        - 6.1.11 `eco_PutWhere`

    - 6.2 建图与导航
        - 6.2.1 `eco_AutoMapCreate`
        - 6.2.2 `eco_SemanticMapCreate`
        - 6.2.3 `eco_SemanticMapManager`
        - 6.2.4 `eco_ChassisMove`
        - 6.2.5 `eco_PointNavigation`
        - 6.2.6 `eco_SemanticNavigation`
        - 6.2.7 `eco_DockDown`
        - 6.2.8 `eco_MapRelocation`
        - 6.2.9 `eco_Recharge`

    - 6.3 图像与位姿
        - 6.3.1 `eco_ImageQuery`
        - 6.3.2 `eco_GetPose`

    - 6.4 高层任务
        - 6.4.1 `eco_FindPerson`
        - 6.4.2 `eco_Search`
        - 6.4.3 `eco_RobotPreparePose`
        - 6.4.4 `eco_RobotEndingPose`
        - 6.4.5 `eco_LookTo`
        - 6.4.6 `eco_AccurateGrab`
        - 6.4.7 `eco_MatchObjViews`
        - 6.4.8 `eco_PickWithArmReview`
        - 6.4.9 `eco_AccuratePlace`
        - 6.4.10 `eco_PlaceWithView`
        - 6.4.11 `eco_PlaceIn`
        - 6.4.12 `eco_Speech`

    - 6.5 本体控制
        - 6.5.1 `eco_RobotHeightCtrl`
        - 6.5.2 `eco_RobotHeadCtrl`
        - 6.5.3 `eco_RobotArmCtrl`
        - 6.5.4 `eco_RobotPoseCtrl`

    - 6.6 任务暂停/恢复/取消
        - 6.6.1 `eco_StopAll`
        - 6.6.2 `eco_PauseMission` / `eco_ResumeMission` / `eco_CancelMission`

    - 6.7 机器状态
        - 6.7.1 `eco_RobotInfo`
        - 6.7.2 `eco_RefreshRobotInfo`
        - 6.7.3 `eco_RobotWorkState` \~ `eco_RobotFurniture`
        - 6.7.4 `eco_RegisterRobotInfoCallback` / `eco_UnregisterRobotInfoCallback`

    - 6.8 C++ 文档额外接口
        - 6.8.1 `eco_GetDefaultLinkUrl`
        - 6.8.2 `eco_SetCancellationFlag`

- 7. 学习路径建议

- 8. 故障排查
    - 8.1 连接失败
    - 8.2 找不到动态库
    - 8.3 ARM 构建失败
    - 8.4 任务无法中断
    - 8.5 HTTP 返回空指针

---

## 1. 概述

在开始使用 SDK 之前，先了解以下关键概念。

### 1.1 SDK 是什么

`robot_sdk` 是八界机器人 C++ 开发套件，提供：
- **机身控制**：通过 WebSocket 连接机器人，控制移动、机械臂、导航等
- **感知能力**：通过 HTTP 调用视觉检测、VLM 等 AI 服务
- **状态查询**：获取机器人电量、位置、告警等实时状态

### 1.2 连接地址

|项目|说明|
|---|---|
|机身控制|WebSocket，默认 `ws://10.10.10.11:9900`|
|感知 HTTP 服务|从同一主机 IP 按端口拼接（OVD=`30081`, DeskIntent=`8555`, PutWhere=`9527`, ImgMatch=`6002`）|
|环境变量|使用 **`ECO_ROBOT_HOST`** 设置机器人 IP|
|默认 IP|`10.10.10.11`（以机器人屏幕底部 `http://10.10.10.11:17890` 显示的 IP 为准）|

**设置示例**：
```Bash
export ECO_ROBOT_HOST=192.168.1.100
```

### 1.3 建图前提

- **导航、语义区域、家具地图**相关能力通常需要先建图。可通过屏幕按钮、可视化平台或 `eco_AutoMapCreate` API 建图。
- **VLM 相关能力**需要先在机器人可视化平台配置页面设置 VLM API KEY。

### 1.4 SDK 技术栈

|项目|说明|
|---|---|
|命名空间|`robot_sdk`|
|开发语言|C++17|
|构建工具|CMake 3.14+|
|运行时|Linux（推荐 Ubuntu x86_64）|
|可选依赖|OpenCV 4.x（`shoe_sorting_demo` 图像处理与可视化需要）|
|内部依赖的系统库|Boost\(system\)、zlib、pthread|

---

## 2. 环境要求

### 2.1 系统与工具

- Linux（推荐 Ubuntu x86_64）
- CMake 3.14 及以上
- C++17 编译器（g++ 8+）
- `make`
- OpenCV 4.x（`shoe_sorting_demo` 的图像解码与可视化需要）
- 已准备好的 `robot_sdk` 头文件与动态库（由交付包 `depend/` 提供）

### 2.2 安装基础构建环境

本文以 Ubuntu x86_64 为例。若只在当前 PC 上构建并运行默认的 `x86` 版本，安装下面这些基础工具即可：

```Bash
sudo apt update
sudo apt install -y build-essential cmake make libopencv-dev
```

安装完成后可检查版本：

```Bash
cmake --version
g++ --version
make --version
pkg-config --modversion opencv4
```

### 2.3 交叉编译工具（ARM）

如果需要在 x86_64 PC 上构建给机器人运行的 `arm` 版本，即执行 `make build ARCH=arm` 或 CMake 中传入 `-DARCH=arm`，还需要额外安装 aarch64 交叉编译工具链：

```Bash
sudo apt install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

检查安装：

```Bash
aarch64-linux-gnu-g++ --version
```

> `aarch64-linux-gnu-g++` 不是默认 `x86` 构建的必需项，只有 `ARCH=arm` 交叉编译时才需要。若编译器不在 `PATH` 中，可在构建时通过 `CPP_SDK_ARM_C_COMPILER` 和 `CPP_SDK_ARM_CXX_COMPILER` 手动指定，见 4.3 ARM 交叉编译说明。

### 2.4 ARM 版 OpenCV 说明

交叉编译 `arm` 版本时，OpenCV 也需要使用目标架构的库。上面安装的 `libopencv-dev` 默认是宿主机 x86_64 版本，只能用于 `x86` 构建。如果需要构建 `shoe_sorting_demo` 的 `arm` 版本，请准备 ARM/aarch64 版本的 OpenCV。

如果使用 Ubuntu 多架构包，建议先模拟安装：

```Bash
sudo dpkg --add-architecture arm64
sudo apt update
apt-get install -s libopencv-dev:arm64
sudo apt install -y libopencv-dev:arm64
```

如果模拟结果显示会卸载现有 `amd64` OpenCV 包，建议改用独立目录安装自行交叉编译好的 OpenCV，构建时通过 `OpenCV_DIR` 指向 ARM 版 OpenCV 的 CMake 配置目录，见 4.3 ARM 交叉编译说明。

### 2.5 SDK 依赖文件

仓库默认已包含以下依赖（无需额外获取）：

- 头文件：`depend/include/robot_sdk/*.h`
- x86 动态库：`depend/lib/x86/librobot_sdk.so`（实际指向 `librobot_sdk.so.0.4.2`）
- arm 动态库：`depend/lib/arm/librobot_sdk.so`（实际指向 `librobot_sdk.so.0.4.2`）

---

## 3. 获取交付包

### 3.1 目录结构

```Plaintext
.
├── CMakeLists.txt
├── Makefile
├── cmake/
│   ├── robot_sdk_prebuilt.cmake
│   └── robot_sdk_target.cmake
├── delivery_demo/
│   ├── CMakeLists.txt
│   └── delivery_demo.cpp
├── desk_demo/
│   ├── CMakeLists.txt
│   └── desk_demo.cpp
├── toy_storage_demo/
│   ├── CMakeLists.txt
│   └── toy_storage_demo.cpp
├── shoe_sorting_demo/
│   ├── CMakeLists.txt
│   └── shoe_sorting_demo.cpp
└── depend/
    ├── include/robot_sdk/
    └── lib/
        ├── x86/
        └── arm/
```

### 3.2 各目录用途

|路径|用途|
|---|---|
|`depend/include/robot_sdk/`|SDK 公开头文件（`robot_sdk.h`、`robot_func.h`、`robot_info.h`、`ovd.h`、`types.h` 等）|
|`depend/lib/x86/librobot_sdk.so`|x86 动态库（版本 0.4.2）|
|`depend/lib/arm/librobot_sdk.so`|ARM/aarch64 动态库（版本 0.4.2）|
|`delivery_demo/`|玩具递送场景 demo|
|`desk_demo/`|桌面整理场景 demo|
|`toy_storage_demo/`|玩具收纳场景 demo|
|`shoe_sorting_demo/`|鞋子整理场景 demo（依赖 OpenCV）|
|`cmake/`|CMake 导入配置文件|

---

## 4. 构建及运行

### 4.1 快速构建

若没有特殊要求，使用 **快速构建** 即可。

默认构建 `x86`：

```Bash
make build
```

若想显式指定架构：

```Bash
make build ARCH=x86
make build ARCH=arm
```

构建产物输出到：

- `build/<arch>`：CMake 构建目录
- `out/<arch>`：可执行文件输出目录，自动打包运行所需的 `librobot_sdk.so*`

例如：

```Bash
out/x86/toy_storage_demo
out/x86/delivery_demo
out/x86/desk_demo
out/x86/shoe_sorting_demo
out/x86/librobot_sdk.so.0
```

`out/<arch>/` 整个目录可以作为自包含运行目录直接分发。

### 4.2 使用 CMake 直接构建

如果你不想通过 `Makefile` 快速构建，也可以直接调用 CMake：

构建 `x86`：

```Bash
cmake -S . -B build/x86 -DARCH=x86 -DCPP_SDK_OUTPUT_DIR=$PWD/out/x86
cmake --build build/x86
```

构建 `arm`：

```Bash
cmake -S . -B build/arm -DARCH=arm -DCPP_SDK_OUTPUT_DIR=$PWD/out/arm
cmake --build build/arm
```

### 4.3 ARM 交叉编译说明

当 `ARCH=arm` 时，工程会切换到 `aarch64-linux-gnu` 工具链。若编译器不在 `PATH` 中，可手动指定：

```Bash
cmake -S . -B build/arm \
  -DARCH=arm \
  -DCPP_SDK_ARM_C_COMPILER=/path/to/aarch64-linux-gnu-gcc \
  -DCPP_SDK_ARM_CXX_COMPILER=/path/to/aarch64-linux-gnu-g++
cmake --build build/arm
```

`shoe_sorting_demo` 依赖 OpenCV。交叉编译时请确保 CMake 找到的是 ARM/aarch64 版本的 OpenCV，而不是宿主机 x86_64 版本。若使用自行准备的 ARM OpenCV，可显式指定：

```Bash
cmake -S . -B build/arm \
  -DARCH=arm \
  -DOpenCV_DIR=/path/to/opencv-arm/lib/cmake/opencv4 \
  -DCPP_SDK_OUTPUT_DIR=$PWD/out/arm
cmake --build build/arm
```

如果 ARM OpenCV 安装在系统多架构目录中：

```Bash
cmake -S . -B build/arm \
  -DARCH=arm \
  -DOpenCV_DIR=/usr/lib/aarch64-linux-gnu/cmake/opencv4 \
  -DCPP_SDK_OUTPUT_DIR=$PWD/out/arm
cmake --build build/arm
```

> 目标设备运行 `shoe_sorting_demo` 时也需要能找到 ARM/aarch64 版本的 OpenCV 动态库。可以在目标设备上安装对应 OpenCV 运行库，或随程序一起部署所需的 OpenCV `.so` 文件。

### 4.4 运行 demo

设置机器人地址（通过环境变量统一设置）：

```Bash
export ECO_ROBOT_HOST=10.10.10.11
```

各 demo 都支持可选的"连接地址"参数；未传参时，会使用 SDK 中获取连接地址的函数 `robot_sdk::eco_GetDefaultLinkUrl()`查看环境变量，未设置时默认为10.10.10.11。

运行玩具收纳：

```Bash
./out/x86/toy_storage_demo
./out/x86/toy_storage_demo ws://127.0.0.1:9900
```

程序运行中可通过 `Ctrl+C` 中断。

### 4.5 清理构建产物

```Bash
make clean
```

会删除 `build/` 和 `out/`。

---

## 5. 快速开始

### 5.1 连接与基础调用

以下是最简可运行示例：连接机器人、执行准备姿态、恢复结束姿态、断开连接。

```C++
#include <robot_sdk/robot_sdk.h>
#include <iostream>

int main() {
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode st = robot_sdk::eco_RobotPreparePose();
  if (st.code != 0) {
    std::cerr << "prepare failed: " << st.code << " " << st.msg << "\n";
    client.Disconnect();
    return 2;
  }

  st = robot_sdk::eco_RobotEndingPose();
  client.Disconnect();
  return st.code == 0 ? 0 : 3;
}
```

### 5.2 协作式取消（Ctrl+C 响应）

长时间任务需要响应中断时，使用 `eco_SetCancellationFlag` 配合信号处理：

```C++
#include <csignal>
#include <atomic>

std::atomic<bool> stop{false};

void signal_handler(int) { stop = true; }

int main() {
  std::signal(SIGINT, signal_handler);
  robot_sdk::eco_SetCancellationFlag(&stop);

  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  // 长时间任务 —— Ctrl+C 后内部等待尽快返回 code==4（interrupted）
  robot_sdk::ErrorCode st = robot_sdk::eco_AutoMapCreate(600000);

  client.Disconnect();
  return 0;
}
```

> 提示：`eco_SetCancellationFlag` 设置后，SDK 内部等待循环会在标志为 `true` 时尽快返回。这不替代机器人端任务取消——如需确保机器人任务停止，应配合 `eco_StopAll()` 或 `eco_CancelMission(task_id)`。

### 5.3 关于 CLI

**C++ SDK 当前不提供通用 CLI**（Python `python -m bajie_sdk ...` 的对应功能）。C++ 开发者建议：

- 直接运行交付 demo（见 4.4 运行 demo）
- 在 demo 或业务代码基础上，用 `main(int argc, char** argv)` 自行封装命令行入口
- 如需快速查询接口字段，可参考 Python SDK 的 CLI 用法（见 Python 开发文档）

### 5.4 常见 API 调用模式

所有 `eco_*` 任务接口（除 HTTP 感知能力外）都要求先建立连接：

```C++
auto& client = robot_sdk::Client::GetInstance();
client.Connect(robot_sdk::eco_GetDefaultLinkUrl());
// ... 调用任务接口 ...
client.Disconnect();
```

- 不限制超时时间的方法为，将超时参数设置为 `0`。
- 同步接口直接阻塞等待任务完成；异步接口（`*Async` 后缀）立即返回 `task_id`，通过回调接收结果。
- 大多数 `robot_func` 的对外接口都支持末尾可选参数 `timeout_ms`，默认值见 `kDefaultTaskCompletionTimeoutMs = 180000`（任务完成超时）和 `kDefaultTaskResponseTimeoutMs = 10000`（应答超时）。

---

## 6. `robot_sdk::eco_*` 接口一览（全量）

本节按功能模块组织 SDK 所有公开接口。各模块的接口说明包括：完整签名、字段说明、返回值说明。

> **使用提示**：
- 除 HTTP 能力（§6.1）外，大多数 `eco_*` 任务接口都要求先建立连接：`robot_sdk::Client::GetInstance().Connect(...)`。
- HTTP 感知接口（§6.1）不依赖 `Client::Connect`，默认访问 `ECO_ROBOT_HOST` 指定 IP 上的对应端口；所有 HTTP 接口都提供默认服务地址版本和带 `base_url` 的重载。

### 6.1 感知

以下 HTTP 感知能力不依赖 `Client::Connect(...)`。默认访问 `ECO_ROBOT_HOST` 指定 IP 上的对应端口；`base_url` 用于显式指定 HTTP 服务基地址，`timeout_ms` 默认 `10000`。

```Bash
export ECO_ROBOT_HOST=192.168.1.100
```

#### 6.1.1 `eco_DetectObjects`

**签名**（`ovd.h`）：

```C++
std::unique_ptr<DetectObjectsResponse> eco_DetectObjects(
    const DetectObjectsRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<DetectObjectsResponse> eco_DetectObjects(
    const DetectObjectsRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DetectObjectsSimple(
    const DetectObjectsRequest& req,
    std::vector<ObjectDetection>& items,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DetectObjectsSimple(
    const DetectObjectsRequest& req,
    std::vector<ObjectDetection>& items,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

开放词汇检测（OVD / SHOE），默认访问 `/ovd`，也可通过 `req.entry` 切换到 `/shoe`。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req`|`DetectObjectsRequest`，字段见下表。|
|`items`|仅 Simple 版本使用，成功时输出检测结果。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**「****`req`****」子字段**：

|字段|说明|
|---|---|
|`rgb_image`|base64图片，可直接由`eco_ImageQuery`获取。|
|`labels`|待检测目标标签列表。|
|`position_region`|位置区域。|
|`payload`|承载物。|
|`ovd_property.color/shape/person`|颜色、形状、人物等属性约束。|
|`ovd_obj_thresh`|OVD 目标阈值，默认 `0.35`。|
|`box_obj_thresh`|框目标阈值，默认 `0.35`。|
|`entry`|服务端点，`OvdEndpoint::OVD` 对应 `/ovd`，`OvdEndpoint::SHOE` 对应 `/shoe`。|

**返回值**：

完整版本返回 `std::unique_ptr<DetectObjectsResponse>`：网络或解析失败时可能为 `nullptr`；非空时通过 `code` / `msg` 判断业务结果，成功时读取 `items` 和 `items_unknown`。Simple 版本返回 `bool`，`true` 表示成功。单个 `ObjectDetection` 包含 `index`、`uuid`、`name`、`bbox`（`[x1,y1,x2,y2]`）和 `ovd_property`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, image, 30000);
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DetectObjectsRequest req;
  req.rgb_image = image.rgb_image.data;
  req.labels = {"玩具"};
  req.entry = robot_sdk::OvdEndpoint::OVD;

  std::unique_ptr<robot_sdk::DetectObjectsResponse> resp =
      robot_sdk::eco_DetectObjects(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code << ", items=" << resp->items.size() << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.2 `eco_ImageEmbeddingExtract`

**签名**（`img_match.h`）：

```C++
std::unique_ptr<ImageEmbeddingResponse> eco_ImageEmbeddingExtract(
    const ImageEmbeddingRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<ImageEmbeddingResponse> eco_ImageEmbeddingExtract(
    const ImageEmbeddingRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_ImageEmbeddingExtractSimple(
    const ImageEmbeddingRequest& req,
    std::vector<std::vector<float>>& embeddings,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_ImageEmbeddingExtractSimple(
    const ImageEmbeddingRequest& req,
    std::vector<std::vector<float>>& embeddings,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

图像特征提取。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req.image_list`|图像 base64 列表。|
|`embeddings`|仅 Simple 版本使用，成功时输出向量列表。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**返回值**：

完整版本返回 `std::unique_ptr<ImageEmbeddingResponse>`；Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, image, 30000);
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::ImageEmbeddingRequest req;
  req.image_list = {image.rgb_image.data};

  std::unique_ptr<robot_sdk::ImageEmbeddingResponse> resp =
      robot_sdk::eco_ImageEmbeddingExtract(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code
            << ", embeddings=" << resp->embeddings.size() << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.3 `eco_FusedSimilarityHybrid`

**签名**（`img_match.h`）：

```C++
std::unique_ptr<FusedSimilarityResponse> eco_FusedSimilarityHybrid(
    const HybridFusedSimRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<FusedSimilarityResponse> eco_FusedSimilarityHybrid(
    const HybridFusedSimRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_FusedSimilarityHybridSimple(
    const HybridFusedSimRequest& req,
    double& similarity,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_FusedSimilarityHybridSimple(
    const HybridFusedSimRequest& req,
    double& similarity,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

融合相似度计算。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req.image_a`|可选图像输入 A。|
|`req.embedding_a`|可选向量输入 A。|
|`req.image_b`|可选图像输入 B。|
|`req.embedding_b`|可选向量输入 B。|
|`similarity`|仅 Simple 版本使用，成功时输出相似度。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**返回值**：

完整版本返回 `std::unique_ptr<FusedSimilarityResponse>`；Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData head_image;
  robot_sdk::ImageQueryNotifyData arm_image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, head_image, 30000);
  if (ec.code == 0) {
    ec = robot_sdk::eco_ImageQuery(robot_sdk::CameraType::ARM, arm_image, 30000);
  }
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::HybridFusedSimRequest req;
  req.image_a = head_image.rgb_image.data;
  req.image_b = arm_image.rgb_image.data;

  std::unique_ptr<robot_sdk::FusedSimilarityResponse> resp =
      robot_sdk::eco_FusedSimilarityHybrid(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code
            << ", similarity=" << resp->similarity << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.4 `eco_PairwiseMatch`

**签名**（`img_match.h`）：

```C++
std::unique_ptr<PairwiseResponse> eco_PairwiseMatch(
    const PairwiseRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<PairwiseResponse> eco_PairwiseMatch(
    const PairwiseRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_PairwiseMatchSimple(
    const PairwiseRequest& req,
    std::vector<PairwiseMatchedItem>& items,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_PairwiseMatchSimple(
    const PairwiseRequest& req,
    std::vector<PairwiseMatchedItem>& items,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

配对匹配。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req.user_id`|可选用户标识。|
|`req.images`|图像与检测框集合。|
|`items`|仅 Simple 版本使用，成功时输出匹配结果。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**返回值**：

完整版本返回 `std::unique_ptr<PairwiseResponse>`；Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, image, 30000);
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DetectObjectsRequest detect_req;
  detect_req.rgb_image = image.rgb_image.data;
  detect_req.labels = {"鞋"};
  detect_req.entry = robot_sdk::OvdEndpoint::SHOE;
  std::unique_ptr<robot_sdk::DetectObjectsResponse> detections =
      robot_sdk::eco_DetectObjects(detect_req);
  if (!detections || detections->code != 0 || detections->items.empty()) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::PairwiseSceneImage scene;
  scene.image = image.rgb_image.data;
  for (const auto& det : detections->items) {
    robot_sdk::PairwiseItemBBox item;
    item.id = det.uuid.empty() ? std::to_string(det.index) : det.uuid;
    item.item = det.name;
    item.color = det.ovd_property.color;
    item.shape = det.ovd_property.shape;
    item.person = det.ovd_property.person;
    item.bbox = det.bbox;
    scene.items.push_back(item);
  }

  robot_sdk::PairwiseRequest req;
  req.user_id = "demo_user";
  req.images = {scene};

  std::unique_ptr<robot_sdk::PairwiseResponse> resp =
      robot_sdk::eco_PairwiseMatch(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code << ", pairs=" << resp->data.size() << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.5 `eco_ShoesIntentPlan`

**签名**（`shoes_intent.h`）：

```C++
std::unique_ptr<ShoesIntentPlanResponse> eco_ShoesIntentPlan(
    const ShoesIntentPlanRequest& req,
    bool move_all,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<ShoesIntentPlanResponse> eco_ShoesIntentPlan(
    const ShoesIntentPlanRequest& req,
    bool move_all,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_ShoesIntentPlanSimple(
    const ShoesIntentPlanRequest& req,
    bool move_all,
    std::vector<ShoesIntentStep>& steps,
    std::vector<ShoesIntentFinalBbox>& final_bboxes,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_ShoesIntentPlanSimple(
    const ShoesIntentPlanRequest& req,
    bool move_all,
    std::vector<ShoesIntentStep>& steps,
    std::vector<ShoesIntentFinalBbox>& final_bboxes,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

鞋子意图规划。`move_all=true` 时调用 `/v038/test/shoes_intent/plan` 生成全量整理方案；`move_all=false` 时调用 `/v038/test/shoes_intent/check` 检查鞋子是否需要调整。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req`|`ShoesIntentPlanRequest`，字段见下表。|
|`move_all`|`true` 表示生成全量整理方案；`false` 表示检查模式。|
|`steps`|仅 Simple 版本使用，成功时输出步骤序列。|
|`final_bboxes`|仅 Simple 版本使用，成功时输出最终布局。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**「****`req`****」子字段**：

|字段|说明|
|---|---|
|`shoes`|鞋子列表，每项包含 `bbox`、`pair_id` 和 `is_tilted`。|
|`shoes[].bbox`|单只鞋检测框，格式 `[xmin, ymin, xmax, ymax]`。|
|`shoes[].pair_id`|成双 ID；未配对可为 `0`。|
|`shoes[].is_tilted`|鞋子是否摆歪，仅 `move_all=false` 检查模式使用。|
|`place_line`|墙线点列，每点为 `[x, y]`。|
|`place_plane`|临时摆放区点列。|

**返回值**：

完整版本返回 `std::unique_ptr<ShoesIntentPlanResponse>`：非空时通过 `code` / `msg` 判断业务结果，成功时读取 `steps` 和 `final_bboxes`。Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, image, 30000);
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DetectObjectsRequest detect_req;
  detect_req.rgb_image = image.rgb_image.data;
  detect_req.labels = {"鞋"};
  detect_req.entry = robot_sdk::OvdEndpoint::SHOE;
  std::unique_ptr<robot_sdk::DetectObjectsResponse> detections =
      robot_sdk::eco_DetectObjects(detect_req);
  if (!detections || detections->code != 0 || detections->items.empty()) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::ShoesIntentPlanRequest req;
  int pair_id = 1;
  for (const auto& det : detections->items) {
    robot_sdk::ShoesIntentShoeItem shoe;
    shoe.bbox = det.bbox;
    shoe.pair_id = pair_id++;
    shoe.is_tilted = false;
    req.shoes.push_back(shoe);
  }
  req.place_line = {{50, 300}, {300, 300}};
  req.place_plane = {{50, 320}, {300, 320}, {300, 420}, {50, 420}};

  std::unique_ptr<robot_sdk::ShoesIntentPlanResponse> resp =
      robot_sdk::eco_ShoesIntentPlan(req, true);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code << ", steps=" << resp->steps.size() << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.6 `eco_DeskIntentPlan`

**签名**（`desk_intent.h`）：

```C++
std::unique_ptr<DeskIntentPlanResponse> eco_DeskIntentPlan(
    const DeskIntentPlanRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<DeskIntentPlanResponse> eco_DeskIntentPlan(
    const DeskIntentPlanRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentPlanSimple(
    const DeskIntentPlanRequest& req,
    std::vector<DeskIntentPlanStep>& steps,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentPlanSimple(
    const DeskIntentPlanRequest& req,
    std::vector<DeskIntentPlanStep>& steps,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

桌面物品摆放意图规划，返回带语义说明的逐步 `from_bbox` / `to_bbox`。`judge_input` 与 `memory_input` 为可选输入，分别用于桌面整理评估结果和用户历史偏好等记忆信息。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req.image`|base64图像。|
|`req.user_input`|用户自然语言整理意图。|
|`req.perception`|桌面感知结果列表，每项包含 `name` 和 `bbox`。|
|`req.judge_input`|可选评估输入，例如"桌面中间区域需要留空"。|
|`req.memory_input`|可选记忆输入，例如"用户偏好将玩偶集中放在左侧"。|
|`steps`|仅 Simple 版本使用，成功时输出步骤序列。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**返回值**：

完整版本返回 `std::unique_ptr<DeskIntentPlanResponse>`：非空时通过 `code` / `msg` 判断业务结果，成功时读取 `user_input`、`plan` 和 `steps`。Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, image, 30000);
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DeskIntentPerceptionRequest perception_req;
  perception_req.image = image.rgb_image.data;
  std::unique_ptr<robot_sdk::DeskIntentPerceptionResponse> perception =
      robot_sdk::eco_DeskIntentPerception(perception_req);
  if (!perception || perception->code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DeskIntentPlanRequest req;
  req.image = image.rgb_image.data;
  req.user_input = "帮我把桌面整理整齐";
  req.perception = perception->objects;

  std::unique_ptr<robot_sdk::DeskIntentPlanResponse> resp =
      robot_sdk::eco_DeskIntentPlan(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code << ", steps=" << resp->steps.size() << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.7 `eco_DeskIntentPerception`

**签名**（`desk_intent.h`）：

```C++
std::unique_ptr<DeskIntentPerceptionResponse> eco_DeskIntentPerception(
    const DeskIntentPerceptionRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<DeskIntentPerceptionResponse> eco_DeskIntentPerception(
    const DeskIntentPerceptionRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentPerceptionSimple(
    const DeskIntentPerceptionRequest& req,
    std::vector<DeskIntentPerceptionObject>& objects,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentPerceptionSimple(
    const DeskIntentPerceptionRequest& req,
    std::vector<DeskIntentPerceptionObject>& objects,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

桌面感知，返回桌面及物品名称与像素框。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req.image`|base64图片。|
|`objects`|仅 Simple 版本使用，成功时输出物品列表，每项包含 `name` 和 `bbox`。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**返回值**：

完整版本返回 `std::unique_ptr<DeskIntentPerceptionResponse>`：非空时通过 `code` / `msg` 判断业务结果，成功时读取 `objects`。Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, image, 30000);
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DeskIntentPerceptionRequest req;
  req.image = image.rgb_image.data;

  std::unique_ptr<robot_sdk::DeskIntentPerceptionResponse> resp =
      robot_sdk::eco_DeskIntentPerception(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code
            << ", objects=" << resp->objects.size() << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.8 `eco_DeskIntentMatch`

**签名**（`desk_intent.h`）：

```C++
std::unique_ptr<DeskIntentMatchResponse> eco_DeskIntentMatch(
    const DeskIntentMatchRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<DeskIntentMatchResponse> eco_DeskIntentMatch(
    const DeskIntentMatchRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentMatchSimple(
    const DeskIntentMatchRequest& req,
    std::vector<int>& out_bbox,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentMatchSimple(
    const DeskIntentMatchRequest& req,
    std::vector<int>& out_bbox,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

将头部相机图中的 bbox 映射到手部相机图像中的 bbox。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req.arm_image`|手部相机图像 base64。|
|`req.base_image`|头部相机图像 base64。|
|`req.bbox`|头部相机图中待匹配框 `[x1, y1, x2, y2]`。|
|`out_bbox`|仅 Simple 版本使用，成功时输出手部相机图中的 4 元 bbox。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**返回值**：

完整版本返回 `std::unique_ptr<DeskIntentMatchResponse>`：非空时通过 `code` / `msg` 判断业务结果，成功时读取 `bbox`。Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData head_image;
  robot_sdk::ImageQueryNotifyData arm_image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, head_image, 30000);
  if (ec.code == 0) {
    ec = robot_sdk::eco_ImageQuery(robot_sdk::CameraType::ARM, arm_image, 30000);
  }
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DeskIntentMatchRequest req;
  req.base_image = head_image.rgb_image.data;
  req.arm_image = arm_image.rgb_image.data;
  req.bbox = {100, 50, 300, 250};

  std::unique_ptr<robot_sdk::DeskIntentMatchResponse> resp =
      robot_sdk::eco_DeskIntentMatch(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code << ", bbox_size=" << resp->bbox.size() << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.9 `eco_DeskIntentPrePerception`

**签名**（`desk_intent.h`）：

```C++
std::unique_ptr<DeskIntentPrePerceptionResponse> eco_DeskIntentPrePerception(
    const DeskIntentPrePerceptionRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<DeskIntentPrePerceptionResponse> eco_DeskIntentPrePerception(
    const DeskIntentPrePerceptionRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentPrePerceptionSimple(
    const DeskIntentPrePerceptionRequest& req,
    double& angle,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentPrePerceptionSimple(
    const DeskIntentPrePerceptionRequest& req,
    double& angle,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

桌面预感知，根据多张不同头部角度的图片推荐桌面视野最大的头部摄像机角度。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req.images`|输入图片列表，每项包含 `image_base64` 和拍摄角度 `angle`。|
|`angle`|仅 Simple 版本使用，成功时输出推荐角度。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**返回值**：

完整版本返回 `std::unique_ptr<DeskIntentPrePerceptionResponse>`：非空时通过 `code` / `msg` 判断业务结果，成功时读取 `angle`。Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::RobotHeadCtrlData head;
  head.value = 0.3f;
  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotHeadCtrl(head, 30000);

  robot_sdk::ImageQueryNotifyData view1;
  if (ec.code == 0) {
    ec = robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, view1, 30000);
  }

  head.value = 0.8f;
  if (ec.code == 0) {
    ec = robot_sdk::eco_RobotHeadCtrl(head, 30000);
  }

  robot_sdk::ImageQueryNotifyData view2;
  if (ec.code == 0) {
    ec = robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, view2, 30000);
  }
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DeskIntentPrePerceptionRequest req;
  req.images = {{view1.rgb_image.data, 0.3}, {view2.rgb_image.data, 0.8}};

  std::unique_ptr<robot_sdk::DeskIntentPrePerceptionResponse> resp =
      robot_sdk::eco_DeskIntentPrePerception(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code << ", angle=" << resp->angle << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.10 `eco_DeskIntentJudgeAction`

**签名**（`desk_intent.h`）：

```C++
std::unique_ptr<DeskIntentJudgeActionResponse> eco_DeskIntentJudgeAction(
    const DeskIntentJudgeActionRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<DeskIntentJudgeActionResponse> eco_DeskIntentJudgeAction(
    const DeskIntentJudgeActionRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentJudgeActionSimple(
    const DeskIntentJudgeActionRequest& req,
    bool& is_tidy,
    std::string& reason,
    double& score,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_DeskIntentJudgeActionSimple(
    const DeskIntentJudgeActionRequest& req,
    bool& is_tidy,
    std::string& reason,
    double& score,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

桌面整理结果判定，对应 Python `eco_vlm_judge`：输入整理后的桌面图片与用户原始需求，判断是否符合整理目标。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req`|`DeskIntentJudgeActionRequest`，字段见下表。|
|`is_tidy`|仅 Simple 版本使用，成功时输出是否符合整理需求。|
|`reason`|仅 Simple 版本使用，成功时输出判定原因。|
|`score`|仅 Simple 版本使用，成功时输出评分。|
|`error_msg`|仅 Simple 版本使用，失败时输出错误描述。|
|`timeout_ms`|HTTP 超时毫秒。|

**「****`req`****」子字段**：

|字段|说明|
|---|---|
|`tidy_image`|base64图片 。|
|`user_input`|用户原始整理需求。|

**返回值**：

完整版本返回 `std::unique_ptr<DeskIntentJudgeActionResponse>`：非空时通过 `code` / `msg` 判断业务结果，成功时读取 `is_tidy`、`reason` 和 `score`。Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData tidy_image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, tidy_image, 30000);
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DeskIntentJudgeActionRequest req;
  req.tidy_image = tidy_image.rgb_image.data;
  req.user_input = "帮我把桌面整理整齐";

  std::unique_ptr<robot_sdk::DeskIntentJudgeActionResponse> resp =
      robot_sdk::eco_DeskIntentJudgeAction(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code
            << ", is_tidy=" << resp->is_tidy
            << ", score=" << resp->score << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

#### 6.1.11 `eco_PutWhere`

**签名**（`put_where.h`）：

```C++
std::unique_ptr<PutWhereResponse> eco_PutWhere(
    const PutWhereRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
std::unique_ptr<PutWhereResponse> eco_PutWhere(
    const PutWhereRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_PutWhereSimple(
    const PutWhereRequest& req,
    std::string& final_answer,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_PutWhereSimple(
    const PutWhereRequest& req,
    std::string& final_answer,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**概述**：

放置推荐，返回完整响应结构。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`req`|`PutWhereRequest`，字段见下表。|
|`final_answer`|仅 Simple 版本使用，成功时输出最终推荐文本。|
|`timeout_ms`|HTTP 超时毫秒。|

**「****`req`****」子字段**：

|字段|说明|
|---|---|
|`carrier`|承载物名称或描述。|
|`carrier_direct`|承载物方向信息。|
|`image`|base64图片。|
|`image_height`|图像高。|
|`image_width`|图像宽。|
|`placeholder`|要放置的物品名。|
|`summary`|摘要信息， 例如"把玩具放到桌子上"。不能为空。|

**返回值**：

完整版本返回 `std::unique_ptr<PutWhereResponse>`：网络或解析失败时可能为 `nullptr`；非空时通过 `code` / `msg` 判断业务结果，成功时读取 `final_answer`。Simple 版本返回 `bool`，`true` 表示成功。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData scene;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, scene, 30000);
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::PutWhereRequest req;
  req.carrier = "桌子";
  req.image = scene.rgb_image.data;
  req.placeholder = "玩具";
  req.summary = "把玩具放到桌子上";

  std::unique_ptr<robot_sdk::PutWhereResponse> resp =
      robot_sdk::eco_PutWhere(req);
  client.Disconnect();
  if (!resp) return 1;
  std::cout << "code=" << resp->code
            << ", answer=" << resp->final_answer << std::endl;
  return resp->code == 0 ? 0 : 1;
}
```

### 6.2 建图与导航

本节接口需要先 `Client::Connect(...)`。

#### 6.2.1 `eco_AutoMapCreate`

**签名**（`robot_func.h`）：

```C++
ErrorCode eco_AutoMapCreate(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_AutoMapCreateAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**概述**：

自动建图任务。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`out_task_id`|仅异步版本使用，输出任务 ID。|
|`on_done`|仅异步版本使用，任务结束后回调 `ErrorCode`。|
|`timeout_ms`|超时毫秒，默认 `180000`。|

**返回值**：

`ErrorCode`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_AutoMapCreate(600000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.2.2 `eco_SemanticMapCreate`

**签名**（`robot_func.h`）：

```C++
ErrorCode eco_SemanticMapCreate(
    const SemanticMapCreateData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_SemanticMapCreateAsync(
    const SemanticMapCreateData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**概述**：

语义建图任务。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`data.area_info`|语义区域列表，每项包含 `area_id`、`area_name`。|
|`out_task_id`|仅异步版本使用，输出任务 ID。|
|`on_done`|仅异步版本使用，任务结束后回调 `ErrorCode`。|
|`timeout_ms`|超时毫秒。|

**返回值**：

`ErrorCode`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::SemanticMapCreateData data;
  data.area_info = {{"living_room", "客厅"}, {"toy_area", "玩具区"}};

  robot_sdk::ErrorCode ec = robot_sdk::eco_SemanticMapCreate(data, 180000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.2.3 `eco_SemanticMapManager`

**签名**（`robot_func.h`）：

```C++
ErrorCode eco_SemanticMapManager(
    const SemanticMapManagerData& data,
    SemanticMapManagerNotifyData* notify_out,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_SemanticMapManagerAsync(
    const SemanticMapManagerData& data,
    std::string& out_task_id,
    SemanticMapManagerCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**概述**：

语义地图对象增删改查。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`data.cmd`|管理命令，`0` 添加，`1` 查询，`2` 修改，`4` 删除。|
|`data.object_info`|地图对象信息，包含 `id`、`model_level`、`mssid`、`model_name`、`name`、`content`、`direction` 等字段。|
|`notify_out`|仅同步版本使用，输出 `objects_info`，可传 `nullptr`。|
|`out_task_id`|仅异步版本使用，输出任务 ID。|
|`on_done`|仅异步版本使用，回调 `ErrorCode` 和 `SemanticMapManagerNotifyData`。|
|`timeout_ms`|超时毫秒。|

**返回值**：

`ErrorCode`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::FurnitureInfo furniture;
  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotFurniture(furniture);
  if (ec.code != 0 || furniture.info.empty()) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::SemanticMapManagerData data;
  data.cmd = 1;
  data.object_info.id = furniture.info.front().fid;

  robot_sdk::SemanticMapManagerNotifyData notify;
  ec = robot_sdk::eco_SemanticMapManager(data, &notify, 30000);
  std::cout << "code=" << ec.code
            << ", objects=" << notify.objects_info.size() << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.2.4 `eco_ChassisMove`

**签名**（`robot_func.h`）：

```C++
ErrorCode eco_ChassisMove(
    const ChassisMoveData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_ChassisMoveAsync(
    const ChassisMoveData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**概述**：

底盘移动控制。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`data.move_distance`|移动距离，单位米。|
|`data.move_angle`|旋转角度，单位弧度。|
|`out_task_id`|仅异步版本使用，输出任务 ID。|
|`on_done`|仅异步版本使用，任务结束后回调 `ErrorCode`。|
|`timeout_ms`|超时毫秒。|

**返回值**：

`ErrorCode`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ChassisMoveData data;
  data.move_distance = 0.2f;
  data.move_angle = 0.0f;

  robot_sdk::ErrorCode ec = robot_sdk::eco_ChassisMove(data, 30000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.2.5 `eco_PointNavigation`

**签名**（`robot_func.h`）：

```C++
ErrorCode eco_PointNavigation(
    const PointNavigationData& nav_data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_PointNavigationAsync(
    const PointNavigationData& nav_data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**概述**：

定点导航。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`nav_data.x`|目标 `x` 坐标，单位米。|
|`nav_data.y`|目标 `y` 坐标，单位米。|
|`nav_data.yaw`|目标朝向，单位弧度。|
|`out_task_id`|仅异步版本使用，输出任务 ID。|
|`on_done`|仅异步版本使用，任务结束后回调 `ErrorCode`。|
|`timeout_ms`|超时毫秒。|

**返回值**：

`ErrorCode`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::PointNavigationData nav;
  nav.x = 1.0f;
  nav.y = 2.0f;
  nav.yaw = 0.0f;

  robot_sdk::ErrorCode ec = robot_sdk::eco_PointNavigation(nav, 180000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.2.6 `eco_SemanticNavigation`

**签名**（`robot_func.h`）：

```C++
ErrorCode eco_SemanticNavigation(
    const SemanticNavigationData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_SemanticNavigationAsync(
    const SemanticNavigationData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**概述**：

语义导航。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`data.area_id`|语义区域 ID。|
|`data.area_name`|语义区域名称。|
|`out_task_id`|仅异步版本使用，输出任务 ID。|
|`on_done`|仅异步版本使用，任务结束后回调 `ErrorCode`。|
|`timeout_ms`|超时毫秒。|

**返回值**：

`ErrorCode`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::SemanticNavigationData nav;
  nav.area_name = "客厅";

  ec = robot_sdk::eco_SemanticNavigation(nav, 180000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.2.7 `eco_DockDown`

**签名**（`robot_func.h`）：

```C++
ErrorCode eco_DockDown(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_DockDownAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**概述**：

下桩。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`out_task_id`|仅异步版本使用，输出任务 ID。|
|`on_done`|仅异步版本使用，任务结束后回调 `ErrorCode`。|
|`timeout_ms`|超时毫秒。|

**返回值**：

`ErrorCode`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_DockDown(180000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.2.8 `eco_MapRelocation`

**签名**（`robot_func.h`）：

```C++
ErrorCode eco_MapRelocation(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_MapRelocationAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**概述**：

地图重定位。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`out_task_id`|仅异步版本使用，输出任务 ID。|
|`on_done`|仅异步版本使用，任务结束后回调 `ErrorCode`。|
|`timeout_ms`|超时毫秒。|

**返回值**：

`ErrorCode`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_MapRelocation(120000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.2.9 `eco_Recharge`

**签名**（`robot_func.h`）：

```C++
ErrorCode eco_Recharge(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RechargeAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**概述**：

回充。

**字段**（请求 / 入参）：

|名称|说明|
|---|---|
|`out_task_id`|仅异步版本使用，输出任务 ID。|
|`on_done`|仅异步版本使用，任务结束后回调 `ErrorCode`。|
|`timeout_ms`|超时毫秒。|

**返回值**：

`ErrorCode`。

**C++ 示例**：

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_Recharge(180000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

### 6.3 Image and Pose

APIs in this section require `Client::Connect(...)` first.

#### 6.3.1 `eco_ImageQuery`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_ImageQuery(
    const ImageQueryData& data,
    ImageQueryNotifyData& result,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_ImageQueryAsync(
    const ImageQueryData& data,
    std::string& out_task_id,
    ImageQueryCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Image capture.

**Fields** (request / input):

|Name|Description|
|---|---|
|`data.camera_type` / `camera_type`|Camera type: `CameraType::ARM` arm camera, `CameraType::HEAD` head camera.|
|`result`|Sync version only; outputs `rgb_image`, `depth_image`, `tf_goal`, `camera_info_k`.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` and `ImageQueryNotifyData`.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, image, 30000);
  std::cout << "code=" << ec.code
            << ", rgb_size=" << image.rgb_image.data.size() << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.3.2 `eco_GetPose`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_GetPose(
    const GetPoseData& data,
    GetPoseNotifyData& notify_out,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_GetPoseAsync(
    const GetPoseData& data,
    std::string& out_task_id,
    GetPoseCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Pose estimation.

**Fields** (request / input):

|Name|Description|
|---|---|
|`data.bbox`|List of 2D bounding boxes.|
|`data.rgb_image`|RGB image.|
|`data.depth_image`|Depth image.|
|`data.tf_map`|Pose information in the map coordinate frame.|
|`notify_out`|Sync version only; outputs `pose_results`.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` and `GetPoseNotifyData`.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, image, 30000);
  if (ec.code != 0) return 1;

  robot_sdk::DetectObjectsRequest detect_req;
  detect_req.rgb_image = image.rgb_image.data;
  detect_req.labels = {"玩具"};
  std::unique_ptr<robot_sdk::DetectObjectsResponse> detections =
      robot_sdk::eco_DetectObjects(detect_req);
  if (!detections || detections->code != 0 || detections->items.empty()) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::GetPoseData data;
  data.rgb_image = image.rgb_image;
  data.depth_image = image.depth_image;
  data.tf_map = image.tf_goal;
  const auto& bbox = detections->items.front().bbox;
  data.bbox = {{detections->items.front().index,
                static_cast<float>(bbox[0]),
                static_cast<float>(bbox[1]),
                static_cast<float>(bbox[2]),
                static_cast<float>(bbox[3])}};

  robot_sdk::GetPoseNotifyData result;
  ec = robot_sdk::eco_GetPose(data, result, 30000);
  std::cout << "code=" << ec.code
            << ", poses=" << result.pose_results.size() << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

### 6.4 High-Level Tasks

APIs in this section require `Client::Connect(...)` first.

#### 6.4.1 `eco_FindPerson`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_FindPerson(
    const FindPersonData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_FindPersonAsync(
    const FindPersonData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Find-person task.

**Fields** (request / input):

|Name|Description|
|---|---|
|`data.user_name`|User name.|
|`data.user_id`|User ID.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::FindPersonData data;
  data.user_name = "张三";

  robot_sdk::ErrorCode ec = robot_sdk::eco_FindPerson(data, 180000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.2 `eco_Search`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_Search(
    const SearchData& search_data,
    BoxData& result,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_SearchAsync(
    const SearchData& search_data,
    std::string& out_task_id,
    SearchCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Search for objects.

**Fields** (request / input):

|Name|Description|
|---|---|
|`search_data.object`|Semantic description of the object, including `item`, `color`, `shape`.|
|`search_data.filter_boxes`|List of filter boxes.|
|`search_data.area`|Area constraint, including `area_id`, `area_name`.|
|`result`|Sync version only; outputs matched `BoxData`.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` and `BoxData`.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::SearchData search;
  search.object.item = "玩具";
  search.area.area_name = "玩具收纳区";

  robot_sdk::BoxData result;
  robot_sdk::ErrorCode ec = robot_sdk::eco_Search(search, result, 180000);
  std::cout << "code=" << ec.code
            << ", frame_id=" << result.frame_id << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.3 `eco_RobotPreparePose`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_RobotPreparePose(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RobotPreparePoseAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Robot prepare pose.

**Fields** (request / input):

|Name|Description|
|---|---|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotPreparePose(60000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.4 `eco_RobotEndingPose`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_RobotEndingPose(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RobotEndingPoseAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Robot ending pose.

**Fields** (request / input):

|Name|Description|
|---|---|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotEndingPose(60000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.5 `eco_LookTo`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_LookTo(
    LookToTarget target,
    const BoxData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_LookToAsync(
    LookToTarget target,
    const BoxData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Observation alignment (`hand_observe` / `head_observe`); use `target` to specify hand or head.

**Fields** (request / input):

|Name|Description|
|---|---|
|`target`|Body part for observation; `LookToTarget::HAND` for hand, `LookToTarget::HEAD` for head.|
|`data.position/orientation/box_length/frame_id`|6D box of the observation target.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData head_image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, head_image, 30000);
  if (ec.code != 0) {
    client.Disconnect();
    return 1;
  }

  robot_sdk::DetectObjectsRequest detect_req;
  detect_req.rgb_image = head_image.rgb_image.data;
  detect_req.labels = {"玩具"};
  std::unique_ptr<robot_sdk::DetectObjectsResponse> detections =
      robot_sdk::eco_DetectObjects(detect_req);
  if (!detections || detections->code != 0 || detections->items.empty()) {
    client.Disconnect();
    return 1;
  }

  const auto& bbox = detections->items.front().bbox;
  robot_sdk::GetPoseData pose_req;
  pose_req.rgb_image = head_image.rgb_image;
  pose_req.depth_image = head_image.depth_image;
  pose_req.tf_map = head_image.tf_goal;
  pose_req.bbox = {{detections->items.front().index,
                    static_cast<float>(bbox[0]),
                    static_cast<float>(bbox[1]),
                    static_cast<float>(bbox[2]),
                    static_cast<float>(bbox[3])}};

  robot_sdk::GetPoseNotifyData pose_result;
  ec = robot_sdk::eco_GetPose(pose_req, pose_result, 30000);
  if (ec.code != 0 || pose_result.pose_results.empty()) {
    client.Disconnect();
    return 1;
  }

  const auto& pose = pose_result.pose_results.front();
  robot_sdk::BoxData box;
  box.frame_id = pose.frame_id;
  box.position = pose.position;
  box.orientation = pose.orientation;
  box.box_length = pose.box_length;

  ec = robot_sdk::eco_LookTo(robot_sdk::LookToTarget::HAND, box, 60000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.6 `eco_AccurateGrab`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_AccurateGrab(
    const AccurateGrabData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_AccurateGrabAsync(
    const AccurateGrabData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Accurate grab.

**Fields** (request / input):

|Name|Description|
|---|---|
|`data.rgb_image`|RGB image.|
|`data.depth_image`|Depth image.|
|`data.tf_goal`|Pose in the coordinate frame.|
|`data.bbox`|Bounding box of the grab target.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData arm_image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::ARM, arm_image, 30000);
  if (ec.code != 0) return 1;

  robot_sdk::DetectObjectsRequest detect_req;
  detect_req.rgb_image = arm_image.rgb_image.data;
  detect_req.labels = {"玩具"};
  std::unique_ptr<robot_sdk::DetectObjectsResponse> detections =
      robot_sdk::eco_DetectObjects(detect_req);
  if (!detections || detections->code != 0 || detections->items.empty()) {
    client.Disconnect();
    return 1;
  }

  const auto& det = detections->items.front();
  const auto& bbox = det.bbox;

  robot_sdk::AccurateGrabData data;
  data.rgb_image = arm_image.rgb_image;
  data.depth_image = arm_image.depth_image;
  data.tf_goal = arm_image.tf_goal;
  data.bbox = {det.name,
               static_cast<float>(bbox[0]),
               static_cast<float>(bbox[1]),
               static_cast<float>(bbox[2]),
               static_cast<float>(bbox[3])};

  ec = robot_sdk::eco_AccurateGrab(data, 300000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.7 `eco_MatchObjViews`

**Signature** (`img_match.h`):

```C++
bool eco_MatchObjViews(
    const ImageQueryNotifyData& head_image,
    const std::vector<int>& ref_bbox,
    const ImageQueryNotifyData& arm_image,
    const std::vector<ObjectDetection>& arm_items,
    ObjectDetection& out,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
bool eco_MatchObjViews(
    const ImageQueryNotifyData& head_image,
    const std::vector<int>& ref_bbox,
    const ImageQueryNotifyData& arm_image,
    const std::vector<ObjectDetection>& arm_items,
    ObjectDetection& out,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);
```

**Overview**:

Cross-view object matching: given a head-view reference bbox, arm-view image, and arm-view detections, select the same object.

**Fields** (request / input):

|Name|Description|
|---|---|
|`head_image`|Head camera RGBD image, typically from `eco_ImageQuery(CameraType::HEAD)`.|
|`ref_bbox`|Reference bounding box in the head camera image, `[x1,y1,x2,y2]`.|
|`arm_image`|Arm camera RGBD image, typically from `eco_ImageQuery(CameraType::ARM)`.|
|`arm_items`|Arm-view detection results; elements are of type `ObjectDetection`.|
|`out`|On success, outputs the matched arm-view target.|
|`error_msg`|Used by the version with error message; outputs error description on failure.|
|`timeout_ms`|HTTP timeout in milliseconds.|

**Return value**:

`bool`: `true` means matching succeeded; the result is written to `out`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData head_image;
  robot_sdk::ImageQueryNotifyData arm_image;
  if (robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, head_image, 30000).code != 0)
    return 1;
  if (robot_sdk::eco_ImageQuery(robot_sdk::CameraType::ARM, arm_image, 30000).code != 0)
    return 1;

  robot_sdk::DetectObjectsRequest detect_req;
  detect_req.rgb_image = arm_image.rgb_image.data;
  detect_req.labels = {"玩具"};
  std::vector<robot_sdk::ObjectDetection> arm_items;
  std::string error_msg;
  if (!robot_sdk::eco_DetectObjectsSimple(detect_req, arm_items, error_msg)) return 1;

  robot_sdk::ObjectDetection matched;
  const bool ok = robot_sdk::eco_MatchObjViews(
      head_image, {100, 120, 220, 260}, arm_image, arm_items, matched, error_msg);
  std::cout << "ok=" << ok << ", name=" << matched.name << std::endl;

  client.Disconnect();
  return ok ? 0 : 1;
}
```

#### 6.4.8 `eco_PickWithArmReview`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_PickWithArmReview(
    const ImageQueryNotifyData& view,
    const BoundingBox& object_bbox,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_PickWithArmReviewAsync(
    const ImageQueryNotifyData& view,
    const BoundingBox& object_bbox,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Grab composite flow with arm review: first perform hand observation using the reference-view bbox, then recapture with the arm camera, re-detect with OVD, match across views, and finally execute `eco_AccurateGrab`.

**Fields** (request / input):

|Name|Description|
|---|---|
|`view`|Reference-view image, typically the result of `eco_ImageQuery(CameraType::HEAD)`.|
|`object_bbox`<br>|Reference-view target bounding box; `name` is used as the OVD label for the arm image; defaults to `object` when empty.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Task completion timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData head_image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, head_image, 30000);
  if (ec.code != 0) return 1;

  robot_sdk::DetectObjectsRequest detect_req;
  detect_req.rgb_image = head_image.rgb_image.data;
  detect_req.labels = {"玩具"};
  std::unique_ptr<robot_sdk::DetectObjectsResponse> detections =
      robot_sdk::eco_DetectObjects(detect_req);
  if (!detections || detections->code != 0 || detections->items.empty()) {
    client.Disconnect();
    return 1;
  }

  const auto& det = detections->items.front();
  const auto& box = det.bbox;
  robot_sdk::BoundingBox bbox{det.name,
                              static_cast<float>(box[0]),
                              static_cast<float>(box[1]),
                              static_cast<float>(box[2]),
                              static_cast<float>(box[3])};
  ec = robot_sdk::eco_PickWithArmReview(head_image, bbox, 300000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.9 `eco_AccuratePlace`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_AccuratePlace(
    const AccuratePlaceData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_AccuratePlaceAsync(
    const AccuratePlaceData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Accurate place.

**Fields** (request / input):

|Name|Description|
|---|---|
|`data.object`|Semantic description of the object.|
|`data.position`|Placement position.|
|`data.orientation`|Placement orientation.|
|`data.box_length`|Target dimensions.|
|`data.frame_id`|Target coordinate frame.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::AccuratePlaceData data;
  data.object.item = "玩具";
  data.frame_id = "map";
  data.position = {1.0, 2.0, 0.0};
  data.box_length = {0.12, 0.1, 0.05};

  ec = robot_sdk::eco_AccuratePlace(data, 300000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.10 `eco_PlaceWithView`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_PlaceWithView(
    const ImageQueryNotifyData& view,
    const std::vector<float>& place_ref,
    bool recapture_place = false,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_PlaceWithViewAsync(
    const ImageQueryNotifyData& view,
    const std::vector<float>& place_ref,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_PlaceWithViewAsync(
    const ImageQueryNotifyData& view,
    const std::vector<float>& place_ref,
    bool recapture_place,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Place using head-view pixel point or pixel box: by default uses `semantic_place`; when `recapture_place=true`, back-projects pixels to 3D and executes `accurate_place`.

**Fields** (request / input):

|Name|Description|
|---|---|
|`view`|Head camera RGBD image, typically from `eco_ImageQuery(CameraType::HEAD)`.|
|`place_ref`|Length 2 for pixel point `[x,y]`; length 4 for pixel box `[x1,y1,x2,y2]`; the center point is used internally.|
|`recapture_place`|`false` runs `semantic_place`; `true` back-projects pixels and runs `accurate_place`.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData head_image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, head_image, 30000);
  if (ec.code != 0) return 1;

  ec = robot_sdk::eco_PlaceWithView(head_image, {153.0f, 230.0f, 403.0f, 384.0f});
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.11 `eco_PlaceIn`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_PlaceIn(
    const ImageQueryNotifyData& view,
    const std::vector<float>& carrier_bbox,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_PlaceInAsync(
    const ImageQueryNotifyData& view,
    const std::vector<float>& carrier_bbox,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Recommended placement inside a container: uses the first 4 elements of `carrier_bbox` as the container box and places according to the "inside" protocol of `semantic_place`.

**Fields** (request / input):

|Name|Description|
|---|---|
|`view`|Head camera RGBD image, typically from `eco_ImageQuery(CameraType::HEAD)`.|
|`carrier_bbox`|Container bounding box; at least 4 values, used as `[x1,y1,x2,y2]`.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Task completion timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ImageQueryNotifyData head_image;
  robot_sdk::ErrorCode ec =
      robot_sdk::eco_ImageQuery(robot_sdk::CameraType::HEAD, head_image, 30000);
  if (ec.code != 0) return 1;

  ec = robot_sdk::eco_PlaceIn(head_image, {153.0f, 230.0f, 403.0f, 384.0f});
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.4.12 `eco_Speech`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_Speech(const std::string& text);
```

**Overview**:

Robot app text-to-speech. Suitable for demo flow completion, task errors, or on-site prompts.

**Fields** (request / input):

|Name|Description|
|---|---|
|`text`|Text content to speak.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_Speech("请让一让");
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

### 6.5 Body Control

APIs in this section require `Client::Connect(...)` first.

#### 6.5.1 `eco_RobotHeightCtrl`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_RobotHeightCtrl(
    const RobotHeightCtrlData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RobotHeightCtrlAsync(
    const RobotHeightCtrlData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Robot body height control.

**Fields** (request / input):

|Name|Description|
|---|---|
|`data.value`|Height value, range `[0, 0.44]` meters.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::RobotHeightCtrlData data;
  data.value = 0.22f;

  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotHeightCtrl(data, 60000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.5.2 `eco_RobotHeadCtrl`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_RobotHeadCtrl(
    const RobotHeadCtrlData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RobotHeadCtrlAsync(
    const RobotHeadCtrlData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Robot head control.

**Fields** (request / input):

|Name|Description|
|---|---|
|`data.value`|Head angle, range `[0, 1.3]` radians.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::RobotHeadCtrlData data;
  data.value = 0.83f;

  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotHeadCtrl(data, 60000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.5.3 `eco_RobotArmCtrl`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_RobotArmCtrl(
    const RobotArmCtrlData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RobotArmCtrlAsync(
    const RobotArmCtrlData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Robot arm control.

**Fields** (request / input):

|Name|Description|
|---|---|
|`data.mode`|Control mode; common values: `2` homing, `6` gripper open, `7` gripper close, `9` box-opening pose, `40` maintenance pose.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::RobotArmCtrlData data;
  data.mode = 2;

  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotArmCtrl(data, 60000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.5.4 `eco_RobotPoseCtrl`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_RobotPoseCtrl(
    const RobotPoseCtrlData& data,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RobotPoseCtrlAsync(
    const RobotPoseCtrlData& data,
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Manipulator pose control.

**Fields** (request / input):

|Name|Description|
|---|---|
|`data.object_pose`|Target pose list; each item includes `frame_id`, `position`, `orientation`.|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Timeout in milliseconds.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::RobotPoseItem pose;
  pose.frame_id = "arm_camera_color_optical_frame";
  pose.position = {0.2f, 0.0f, 0.3f};
  pose.orientation.w = 1.0f;

  robot_sdk::RobotPoseCtrlData data;
  data.object_pose = {pose};

  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotPoseCtrl(data, 60000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

### 6.6 Task Pause / Resume / Cancel

APIs in this section require `Client::Connect(...)` first.

#### 6.6.1 `eco_StopAll`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_StopAll(int timeout_ms = kDefaultTaskResponseTimeoutMs);
ErrorCode eco_StopAllAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskResponseTimeoutMs);
```

**Overview**:

Cancel all tasks.

**Fields** (request / input):

|Name|Description|
|---|---|
|`out_task_id`|Async version only; outputs task ID.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes.|
|`timeout_ms`|Response timeout in milliseconds; default `10000`.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_StopAll();
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.6.2 `eco_PauseMission` / `eco_ResumeMission` / `eco_CancelMission`

**Signature** (`robot_func.h`):

```C++
ErrorCode eco_PauseMission(const std::string& task_id);
ErrorCode eco_ResumeMission(const std::string& task_id);
ErrorCode eco_CancelMission(const std::string& task_id);
```

**Overview**:

Pause, resume, or cancel a specific task; corresponds to Python `eco_missionControl` `pause` / `resume` / `cancel`. For tasks controlled via pause/resume, the SDK's `timeout_ms` while waiting for completion pauses during pause and resumes counting after resume.

**Fields** (request / input):

|Name|Description|
|---|---|
|`task_id`|Task ID, typically from `out_task_id` returned by an async API.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  const std::string task_id = "替换为异步接口返回的task_id";
  robot_sdk::ErrorCode ec = robot_sdk::eco_PauseMission(task_id);
  if (ec.code == 0) ec = robot_sdk::eco_ResumeMission(task_id);
  if (ec.code == 0) ec = robot_sdk::eco_CancelMission(task_id);

  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;
  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

### 6.7 Robot Status

APIs in this section require `Client::Connect(...)` first. After the SDK connects, it automatically fetches full RobotInfo once and caches it; subsequent `robot_info` reports update the cache incrementally.

#### 6.7.1 `eco_RobotInfo`

**Signature** (`robot_info.h`):

```C++
ErrorCode eco_RobotInfo(RobotInfoData& out);
```

**Overview**:

Get the latest robot status cache maintained internally by the SDK.

**Fields** (request / input):

|Name|Description|
|---|---|
|`out`|Outputs the latest full cache snapshot.|

**Return value**:

`ErrorCode`: `code==0` means the cache was retrieved; non-zero means the cache is not yet initialized or internal state is unavailable.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::RobotInfoData info;
  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotInfo(info);
  std::cout << "code=" << ec.code
            << ", battery=" << info.battery.value << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.7.2 `eco_RefreshRobotInfo`

**Signature** (`robot_info.h`):

```C++
ErrorCode eco_RefreshRobotInfo(const std::vector<std::string>& topics);
```

**Overview**:

Manually refresh specified fields in the SDK's internal RobotInfo cache. When `topics` is empty, refreshes all fields.

**Fields** (request / input):

|Name|Description|
|---|---|
|`topics`|List of status topics to refresh, e.g. `battery`, `pos`, `workState`; empty means refresh all.|

**Return value**:

`ErrorCode`: `code==0` means the oneshot fetch succeeded and was merged into the cache.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_RefreshRobotInfo({"battery", "pos"});
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.7.3 `eco_RobotWorkState` \~ `eco_RobotFurniture`

The following APIs read the corresponding sub-state from the cache:

|API|Output type|Key fields|
|---|---|---|
|`eco_RobotWorkState(WorkState& out)`|`WorkState`|`task_id`, `name`, `cmd`|
|`eco_RobotBattery(BatteryInfo& out)`|`BatteryInfo`|`value`, `is_charge`, `mode`|
|`eco_RobotAlarm(std::vector<int>& out)`|Alarm code list|Exception codes|
|`eco_RobotPosition(RobotPos& out)`|`RobotPos`|`room`, `x`, `y`, `yaw`|
|`eco_RobotMapInfo(MapInfo& out)`|`MapInfo`|`mid`, `mname`, dimensions, origin, grid data|
|`eco_RobotFurniture(FurnitureInfo& out)`|`FurnitureInfo`|Furniture list (`fid`, `fname`, `mssid`)|

All return `ErrorCode`; non-zero means the cache is not yet initialized or the corresponding field is unavailable.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::WorkState work_state;
  robot_sdk::BatteryInfo battery;
  std::vector<int> alarm;
  robot_sdk::RobotPos pos;
  robot_sdk::MapInfo map;
  robot_sdk::FurnitureInfo furniture;

  robot_sdk::ErrorCode ec = robot_sdk::eco_RobotWorkState(work_state);
  if (ec.code == 0) ec = robot_sdk::eco_RobotBattery(battery);
  if (ec.code == 0) ec = robot_sdk::eco_RobotAlarm(alarm);
  if (ec.code == 0) ec = robot_sdk::eco_RobotPosition(pos);
  if (ec.code == 0) ec = robot_sdk::eco_RobotMapInfo(map);
  if (ec.code == 0) ec = robot_sdk::eco_RobotFurniture(furniture);

  std::cout << "code=" << ec.code
            << ", work=" << work_state.cmd
            << ", battery=" << battery.value
            << ", furniture=" << furniture.info.size() << std::endl;

  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

#### 6.7.4 `eco_RegisterRobotInfoCallback` / `eco_UnregisterRobotInfoCallback`

**Signature** (`robot_info.h`):

```C++
ErrorCode eco_RegisterRobotInfoCallback(
    std::string& out_registration_id,
    RobotInfoReportCallback on_report);
ErrorCode eco_UnregisterRobotInfoCallback(const std::string& registration_id);
```

**Overview**:

Register / unregister local `robot_info` incremental callbacks. Callback parameter is `RobotInfoPartial`.

**Fields** (request / input):

|Name|Description|
|---|---|
|`out_registration_id`|Outputs local registration ID.|
|`on_report`|Callback parameter is `RobotInfoPartial`.|
|`registration_id`|Registration ID returned at registration time; used when unregistering.|

**Return value**:

`ErrorCode`.

**C++ example**:

```C++
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  std::string registration_id;
  robot_sdk::ErrorCode ec = robot_sdk::eco_RegisterRobotInfoCallback(
      registration_id,
      [](const robot_sdk::RobotInfoPartial& partial) {
        if (partial.battery) {
          std::cout << "battery=" << partial.battery->value << std::endl;
        }
      });

  if (ec.code == 0) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ec = robot_sdk::eco_UnregisterRobotInfoCallback(registration_id);
  }

  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;
  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

### 6.8 Additional C++ APIs

The following APIs exist only in the C++ public headers and are therefore grouped at the end.

#### 6.8.1 `eco_GetDefaultLinkUrl`

**Signature** (`client.h`):

```C++
std::string eco_GetDefaultLinkUrl();
```

**Overview**:

Returns the SDK default connection URL. Built from environment variable `ECO_ROBOT_HOST` (default `10.10.10.11`) and port `9900`.

**C++ example**:

```C++
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  std::cout << robot_sdk::eco_GetDefaultLinkUrl() << std::endl;
  return 0;
}
```

#### 6.8.2 `eco_SetCancellationFlag`

**Signature** (`robot_func.h`):

```C++
void eco_SetCancellationFlag(std::atomic<bool>* flag);
```

**Overview**:

Sets a cooperative cancellation flag. When the external `atomic<bool>` pointer is set to `true`, SDK internal wait loops end as soon as possible (returning `code==4` interrupted). See §5.2 Cooperative Cancellation for usage.

**C++ example**:

```C++
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <robot_sdk/robot_sdk.h>

std::atomic<bool> g_cancelled{false};

void OnSignal(int) {
  g_cancelled.store(true);
}

int main() {
  setenv("ECO_ROBOT_HOST", "10.10.10.11", 1);
  std::signal(SIGINT, OnSignal);
  robot_sdk::eco_SetCancellationFlag(&g_cancelled);

  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(robot_sdk::eco_GetDefaultLinkUrl())) return 1;

  robot_sdk::ErrorCode ec = robot_sdk::eco_AutoMapCreate(600000);
  std::cout << "code=" << ec.code << ", msg=" << ec.msg << std::endl;

  robot_sdk::eco_SetCancellationFlag(nullptr);
  client.Disconnect();
  return ec.code == 0 ? 0 : 1;
}
```

---

## 7. Learning Path

If you are using the C++ SDK for the first time, follow this order:

|Stage|Goal|Topics|
|---|---|---|
|**1. Prepare environment**|Able to compile|Read §1 Overview → §2 Requirements → §3 Obtaining the package|
|**2. Build and run**|Demo runs successfully|Read §4 Build and Run|
|**3. First connection**|Code connects to the robot|Run the example in §5.1 Connection and Basic Calls|
|**4. Basic control**|Move + capture images|Read §6.2.4 `eco_ChassisMove` and §6.3.1 `eco_ImageQuery`|
|**5. Perception and detection**|Recognize objects|Read §6.1.1 `eco_DetectObjects`|
|**6. Pick and place**|Complete pick-and-place loop|Read §6.4.5 `eco_LookTo` → §6.4.6 `eco_AccurateGrab` → §6.4.11 `eco_PlaceIn`; simplified flow in §6.4.8 `eco_PickWithArmReview`|
|**7. Mapping and navigation**|Autonomous robot movement|Read §6.2.1, §6.2.5, §6.2.6|
|**8. Desk organization (VLM)**|AI-driven organization|Read §6.1.6 `eco_DeskIntentPlan` and its composition chain|
|**9. Advanced usage**|Async, status subscription|Read §5.2 (cancellation), §6.7 (status callbacks)|

> **Troubleshooting**: If you run into problems at any point, see §8 Troubleshooting first.

---

## 8. Troubleshooting

Common issues grouped by category for quick diagnosis.

### 8.1 Connection failure

- Check `ws://<robot_ip>:9900/` connectivity, on-device services, and firewall.
- Confirm `ECO_ROBOT_HOST` is set to the IP shown on the robot screen.
- Confirm the robot-side body control service (WebSocket/9900) is running normally.

### 8.2 Shared library not found

Typical symptom:

```Plaintext
error while loading shared libraries: librobot_sdk.so
```

Remediation:
- Distribute `out/<arch>/` as a whole (including `librobot_sdk.so.0`).
- Set `LD_LIBRARY_PATH`:

```Bash
export LD_LIBRARY_PATH=$PWD/out/x86:$LD_LIBRARY_PATH
```

- Or set rpath at compile time:

```Bash
# 编译期指定 rpath
g++ ... -Wl,-rpath,'$ORIGIN/../lib' -lrobot_sdk
```

### 8.3 ARM build failure

- Check the `aarch64-linux-gnu` toolchain and CMake compiler settings.
- Confirm `ARCH=arm` is set correctly.
- When cross-compiling OpenCV, confirm `OpenCV_DIR` points to libraries for the correct architecture.

### 8.4 Task cannot be interrupted

Use `eco_SetCancellationFlag` for cooperative cancellation, together with signal handling:

```C++
std::atomic<bool> cancelled{false};
robot_sdk::eco_SetCancellationFlag(&cancelled);
// Ctrl+C 处理器中设置:
cancelled = true;
```

To ensure robot-side tasks are also terminated, use `eco_StopAll()` or `eco_CancelMission(task_id)` as well.

### 8.5 HTTP returns null pointer

- Check the HTTP service address and that `ECO_ROBOT_HOST` is correct.
- Check that request fields are complete and image encoding is valid base64.
- When the full API (e.g. `eco_PutWhere`) returns `nullptr`, it is usually a network/parsing issue; try the Simple version first to inspect `error_msg`.
