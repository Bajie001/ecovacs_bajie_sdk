# README

# BaJie Robot SDK Development Document \(C\+\+\) 

- This document is intended for end users, introducing how to obtain and install ` robot_sdk ` \(C\+\+\), how to quickly call robot capabilities, and how to view interface and field descriptions\. 

- Currently, it is the **1\.1\.0** version of the SDK\.

- It is recommended that developers choose a PC with the Ubuntu system x86\_64 for development and debugging, and other environments can be configured by themselves\. This article uses the Ubuntu system as an example for explanation\. 

---

## Table of Contents

1. Overview

- 1\.1 What is the SDK?

- 1\.2 Connection Address

- 1\.3 Mapping Prerequisites

- 1\.4 SDK Technology Stack

2. Environmental Requirements

- 2\.1 Systems and Tools

- 2\.2 Install the basic build environment

- 2\.3 Cross Compilation Tools \(ARM\)

- 2\.4 ARM Version OpenCV Instructions

- 2\.5 SDK Dependency Files

3. Obtain the Delivery Package 

- 3\.1 Directory Structure

- 3\.2 Purpose of Each Directory

4. Build and Run

- 4\.1 Quick Construction

- 4\.2 Directly Build Using CMake

- 4\.3 ARM Cross Compilation Instructions

- 4\.4 Run the Demo

- 4\.5 Clean up build artifacts

5. Quick Start

- 5\.1 Connection and Basic Invocation

- 5\.2 Cooperative Cancellation \(Ctrl\+C Response\)

- 5\.3 About CLI

- 5\.4 Common API Call Patterns

6. `robot_sdk::eco_*` Interface Overview \(Full\)

- 6\.1 Perception

    - 6\.1\.1 `eco_DetectObjects`

    - 6\.1\.2 `eco_ImageEmbeddingExtract`

    - 6\.1\.3 `eco_FusedSimilarityHybrid`

    - 6\.1\.4 `eco_PairwiseMatch`

    - 6\.1\.5 `eco_ShoesIntentPlan`

    - 6\.1\.6 `eco_DeskIntentPlan`

    - 6\.1\.7 `eco_DeskIntentPerception`

    - 6\.1\.8 `eco_DeskIntentMatch`

    - 6\.1\.9 `eco_DeskIntentPrePerception`

    - 6\.1\.10 `eco_DeskIntentJudgeAction`

    - 6\.1\.11 `eco_PutWhere`

- 6\.2 Mapping and Navigation

    - 6\.2\.1 `eco_AutoMapCreate`

    - 6\.2\.2 `eco_SemanticMapCreate`

    - 6\.2\.3 `eco_SemanticMapManager`

    - 6\.2\.4 `eco_ChassisMove`

    - 6\.2\.5 `eco_PointNavigation`

    - 6\.2\.6 `eco_SemanticNavigation`

    - 6\.2\.7 `eco_DockDown`

    - 6\.2\.8 `eco_MapRelocation`

    - 6\.2\.9 `eco_Recharge`

- 6\.3 Image and Pose

    - 6\.3\.1 `eco_ImageQuery`

    - 6\.3\.2 `eco_GetPose`

- 6\.4 High\-level Tasks

    - 6\.4\.1 `eco_FindPerson`

    - 6\.4\.2 `eco_Search`

    - 6\.4\.3 `eco_RobotPreparePose`

    - 6\.4\.4 `eco_RobotEndingPose`

    - 6\.4\.5 `eco_LookTo`

    - 6\.4\.6 `eco_AccurateGrab`

    - 6\.4\.7 `eco_MatchObjViews`

    - 6\.4\.8 `eco_PickWithArmReview`

    - 6\.4\.9 `eco_AccuratePlace`

    - 6\.4\.10 `eco_PlaceWithView`

    - 6\.4\.11 `eco_PlaceIn`

    - 6\.4\.12 `eco_Speech`

- 6\.5 Ontology Control

    - 6\.5\.1 `eco_RobotHeightCtrl`

    - 6\.5\.2 `eco_RobotHeadCtrl`

    - 6\.5\.3 `eco_RobotArmCtrl`

    - 6\.5\.4 `eco_RobotPoseCtrl`

- 6\.6 Task Pause/Resume/Cancel

    - 6\.6\.1 `eco_StopAll`

    - 6\.6\.2 `eco_PauseMission` / `eco_ResumeMission` / `eco_CancelMission`

- 6\.7 Machine Status

    - 6\.7\.1 `eco_RobotInfo`

    - 6\.7\.2 `eco_RefreshRobotInfo`

    - 6\.7\.3 `eco_RobotWorkState` \~ `eco_RobotFurniture`

    - 6\.7\.4 `eco_RegisterRobotInfoCallback` / `eco_UnregisterRobotInfoCallback`

- 6\.8 C\+\+ Documentation Additional Interfaces

    - 6\.8\.1 `eco_GetDefaultLinkUrl`

    - 6\.8\.2 `eco_SetCancellationFlag`

7. Learning Path Recommendations

8. Troubleshooting

- 8\.1 Connection Failed

- 8\.2 Dynamic library not found

- 8\.3 ARM Build Failed

- 8\.4 The task cannot be interrupted

- 8\.5 HTTP returns a null pointer

---

## 1\. Overview

Before you start using the SDK, familiarize yourself with the following key concepts\. 

### 1\.1 What is the SDK?

`robot_sdk` is the Bajie Robot C\+\+ Development Kit, providing:

- **Body Control**: Connect to the robot via WebSocket to control movement, robotic arm, navigation, etc\.

- **Perceptual Ability**: Invoke AI services such as visual inspection and VLM through HTTP calls

- **Status Query**: Obtain real\-time status such as robot battery level, position, and alarms

### 1\.2 Connection Address

|Project|Instructions|
|---|---|
|Airframe Control|WebSocket, default `ws://10.10.10.11:9900`|
|Perceive HTTP Service|Concatenate by port from the same host IP \(OVD=`30081`, DeskIntent=`8555`, PutWhere=`9527`, ImgMatch=`6002`\) |
|Environment Variable|Use **`ECO_ROBOT_HOST`** to set the robot IP|
|Default IP|`10.10.10.11` \(subject to the IP displayed at the bottom of the robot screen `http://10.10.10.11:17890`\) |

**Setting Example**: 

```Bash
export ECO_ROBOT_HOST=192.168.1.100
```

### 1\.3 Mapping Prerequisites

- **Navigation, semantic area, and furniture map** related capabilities usually require prior mapping\. Mapping can be done via screen buttons, visualization platforms, or `eco_AutoMapCreate` API\.

- **VLM\-related capabilities ** require setting the VLM API KEY first on the configuration page of the robot visualization platform\. 

### 1\.4 SDK Technology Stack

|Project|Instructions|
|---|---|
|Namespace|`robot_sdk`|
|Development Language|C\+\+17|
|Build Tools|CMake 3\.14\+|
|Runtime|Linux \(Ubuntu x86\_64 recommended\)|
|Optional Dependencies|OpenCV 4\.x \(`shoe_sorting_demo` Image Processing and Visualization Requirements\)|
|System libraries with internal dependencies|Boost\(system\)、zlib、pthread|

---

## 2\. Environmental Requirements

### 2\.1 Systems and Tools

- Linux \(Ubuntu x86\_64 recommended\)

- CMake 3\.14 or higher

- C\+\+17 Compiler \(g\+\+ 8\+\)

- `make`

- OpenCV 4\.x \(required for image decoding and visualization of `shoe_sorting_demo`\) 

- Prepared `robot_sdk` Header Files and Dynamic Libraries \(provided by the delivery package `depend/`\) 

### 2\.2 Install the basic build environment

This article takes Ubuntu x86\_64 as an example\. If you only want to build and run the default `x86` version on the current PC, installing the following basic tools will suffice: 

```Bash
sudo apt update
sudo apt install -y build-essential cmake make libopencv-dev
```

After installation, you can check the version: 

```Bash
cmake --version
g++ --version
make --version
pkg-config --modversion opencv4
```

### 2\.3 Cross Compilation Tools \(ARM\)

If you need to build the `arm` version for the robot to run on an x86\_64 PC, that is, execute `make build ARCH=arm` or pass `-DARCH=arm` in CMake, you also need to additionally install the aarch64 cross\-compilation toolchain: 

```Bash
sudo apt install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

Check Installation:

```Bash
aarch64-linux-gnu-g++ --version
```

> `aarch64-linux-gnu-g++` is not a requirement for the default `x86` build, and is only needed when `ARCH=arm` cross\-compiling\. If the compiler is not in `PATH`, it can be manually specified during the build via `CPP_SDK_ARM_C_COMPILER` and `CPP_SDK_ARM_CXX_COMPILER`, see 4\.3 ARM Cross\-Compilation Instructions\.

### 2\.4 ARM Version OpenCV Instructions

When cross\-compiling the `arm` version, OpenCV also needs to use libraries of the target architecture\. The `libopencv-dev` installed above is the host x86\_64 version by default and can only be used for `x86` builds\. If you need to build the `shoe_sorting_demo` `arm` version, please prepare the ARM/aarch64 version of OpenCV\.

If you are using Ubuntu multi\-architecture packages, it is recommended to simulate the installation first: 

```Bash
sudo dpkg --add-architecture arm64
sudo apt update
apt-get install -s libopencv-dev:arm64
sudo apt install -y libopencv-dev:arm64
```

If the simulation results show that the existing ` amd64 ` OpenCV package will be uninstalled, it is recommended to use an independent directory to install the self \- cross \- compiled OpenCV instead\. During the build process, point to the CMake configuration directory of the ARM version of OpenCV via ` OpenCV_DIR `, see 4\.3 ARM Cross \- Compilation Instructions\. 

### 2\.5 SDK Dependency Files

The repository already includes the following dependencies by default \(no additional acquisition required\): 

- 头文件：`depend/include/robot_sdk/*.h`

- x86 Dynamic Library:`depend/lib/x86/librobot_sdk.so` \(actually points to `librobot_sdk.so.0.4.2`\)

- ARM Dynamic Library:`depend/lib/arm/librobot_sdk.so` \(actually points to `librobot_sdk.so.0.4.2`\)

---

## 3\. Obtain the Delivery Package

### 3\.1 Directory Structure

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

### 3\.2 Purpose of Each Directory

|Path|Usage|
|---|---|
|`depend/include/robot_sdk/`|SDK Public Header Files \(`robot_sdk.h`, `robot_func.h`, `robot_info.h`, `ovd.h`, `types.h`, etc\.\)|
|`depend/lib/x86/librobot_sdk.so`|x86 Dynamic Library \(Version 0\.4\.2\)|
|`depend/lib/arm/librobot_sdk.so`|ARM/aarch64 Dynamic Library \(Version 0\.4\.2\)|
|`delivery_demo/`|Toy Delivery Scenario Demo |
|`desk_demo/`|Desktop Organization Scenario Demo|
|`toy_storage_demo/`|Toy Storage Scene Demo |
|`shoe_sorting_demo/`|Shoes Sorting Scene Demo \(Depends on OpenCV\) |
|`cmake/`|CMake Import Configuration File|

---

## 4\. Build and Run

### 4\.1 Quick Construction

If there are no special requirements, using ** Quick Build ** is sufficient\. 

Default build `x86`: 

```Bash
make build
```

To explicitly specify the architecture:

```Bash
make build ARCH=x86
make build ARCH=arm
```

Build artifacts are output to:

- `build/<arch>`: CMake build directory

- `out/<arch>`: The output directory for executable files, automatically packaging the `librobot_sdk.so* required for running`

For example: 

```Bash
out/x86/toy_storage_demo
out/x86/delivery_demo
out/x86/desk_demo
out/x86/shoe_sorting_demo
out/x86/librobot_sdk.so.0
```

`out/<arch>/` The entire directory can be directly distributed as a self\-contained runtime directory\.

### 4\.2 Directly Build Using CMake

If you don't want to quickly build via ` Makefile `, you can also directly call CMake: 

Build` x86 `: 

```Bash
cmake -S . -B build/x86 -DARCH=x86 -DCPP_SDK_OUTPUT_DIR=$PWD/out/x86
cmake --build build/x86
```

Compile `arm`: 

```Bash
cmake -S . -B build/arm -DARCH=arm -DCPP_SDK_OUTPUT_DIR=$PWD/out/arm
cmake --build build/arm
```

### 4\.3 ARM Cross Compilation Instructions

When `ARCH=arm`, the project will switch to the `aarch64-linux-gnu` toolchain\. If the compiler is not in `PATH`, you can manually specify it:

```Bash
cmake -S . -B build/arm \
  -DARCH=arm \
  -DCPP_SDK_ARM_C_COMPILER=/path/to/aarch64-linux-gnu-gcc \
  -DCPP_SDK_ARM_CXX_COMPILER=/path/to/aarch64-linux-gnu-g++
cmake --build build/arm
```

`shoe_sorting_demo` depends on OpenCV\. When cross\-compiling, please ensure that CMake finds the ARM/aarch64 version of OpenCV, not the host x86\_64 version\. If you use your own prepared ARM OpenCV, you can explicitly specify it:

```Bash
cmake -S . -B build/arm \
  -DARCH=arm \
  -DOpenCV_DIR=/path/to/opencv-arm/lib/cmake/opencv4 \
  -DCPP_SDK_OUTPUT_DIR=$PWD/out/arm
cmake --build build/arm
```

If ARM OpenCV is installed in the system multi\-architecture directory: 

```Bash
cmake -S . -B build/arm \
  -DARCH=arm \
  -DOpenCV_DIR=/usr/lib/aarch64-linux-gnu/cmake/opencv4 \
  -DCPP_SDK_OUTPUT_DIR=$PWD/out/arm
cmake --build build/arm
```

> When the target device runs `shoe_sorting_demo`, it also needs to be able to find the ARM/aarch64 version of the OpenCV dynamic library\. You can install the corresponding OpenCV runtime library on the target device or deploy the required OpenCV `.so` files along with the program\. 

### 4\.4 Run the Demo

Set the robot address \(uniformly set via environment variables\):

```Bash
export ECO_ROBOT_HOST=10.10.10.11
```

Each demo supports an optional "connection address" parameter; when no parameter is passed, the function for obtaining the connection address in the SDK will be used `robot_sdk::eco_GetDefaultLinkUrl()` to check the environment variable, which defaults to 10\.10\.10\.11 if not set\.

Running Toy Storage:

```Bash
./out/x86/toy_storage_demo
./out/x86/toy_storage_demo ws://127.0.0.1:9900
```

During program execution, it can be interrupted via `Ctrl+C`\. 

### 4\.5 Clean up build artifacts

```Bash
make clean
```

will delete ` build/ ` and ` out/ `\. 

---

## 5\. Quick Start

### 5\.1 Connection and Basic Invocation

The following is the simplest runnable example: connect to the robot, execute the ready pose, restore the end pose, and disconnect\. 

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

### 5\.2 Cooperative Cancellation \(Ctrl\+C Response\)

When a long\-running task needs to respond to an interruption, use `eco_SetCancellationFlag` in conjunction with signal handling:

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

> Tip: After the `eco_SetCancellationFlag `is set, the SDK internal wait loop will return as soon as possible when the flag is `true `\. This does not replace the robot side task cancellation \- to ensure that the robot task stops, you should cooperate with `eco_StopAll () `or `eco_CancelMission (task_id) `\.

### 5\.3 About CLI

**The C\+\+ SDK currently does not provide a generic CLI ** \(the equivalent functionality of `python -m bajie_sdk...` in Python\)\. Recommendations for C\+\+ developers: 

- Directly run the delivered demo \(see 4\.4 Running the Demo\) 

- Based on the demo or business code, use `main(int argc, char** argv)` to encapsulate the command\-line entry by yourself

- For quick query of interface fields, you can refer to the CLI usage of the Python SDK \(see Python Development Document\)\. 

### 5\.4 Common API Call Patterns

All `eco_*` task interfaces \(excluding HTTP awareness capabilities\) require establishing a connection first:

```C++
auto& client = robot_sdk::Client::GetInstance();
client.Connect(robot_sdk::eco_GetDefaultLinkUrl());
// ... 调用任务接口 ...
client.Disconnect();
```

- The method for not limiting the timeout is to set the timeout parameter to `0`\. 

- The synchronous interface directly blocks and waits for the task to complete; the asynchronous interface \(`*Async` suffix\) immediately returns `task_id` and receives the result via callback\. 

- Most` robot_func ` external interfaces support the optional trailing parameter ` timeout_ms `, with default values specified in ` kDefaultTaskCompletionTimeoutMs = 180000 ` \(task completion timeout\) and ` kDefaultTaskResponseTimeoutMs = 10000 ` \(response timeout\)\. 

---

## 6\. `robot_sdk::eco_*` Interface Overview \(Full\)

This section organizes all public interfaces of the SDK by functional module\. The interface descriptions for each module include: full signature, field description, and return value description\. 

> **Usage Instructions**: 

- Except for HTTP capabilities \(§6\.1\), most `eco_*` task interfaces require establishing a connection first: `robot_sdk::Client::GetInstance().Connect(...) `\. 

- The HTTP\-aware interface \(§6\.1\) does not depend on ` Client::Connect `, and by default accesses the corresponding port on the IP specified by ` ECO_ROBOT_HOST ` ; all HTTP interfaces provide both a default service address version and an overload with ` base_url `\. 

### 6\.1 Perception

The following HTTP awareness capabilities do not depend on ` Client::Connect(...) `\. By default, it accesses the corresponding port on the IP specified by ` ECO_ROBOT_HOST ` ; ` base_url ` is used to explicitly specify the base address of the HTTP service, ` timeout_ms ` defaults to ` 10000 `\. 

```Bash
export ECO_ROBOT_HOST=192.168.1.100
```

#### 6\.1\.1 `eco_DetectObjects`

**Signature**\(`ovd.h`\):

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

**Overview **: 

Open Vocabulary Detection \(OVD / SHOE\), default access `/ovd`, can also be switched to `req.entry` to `/shoe`\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req`|`DetectObjectsRequest`, fields are shown in the table below\.|
|`items`|Only used in the Simple version, outputs the detection results upon success\. |
|`error_msg`|Only used in the Simple version, outputs error description when it fails\.|
|`timeout_ms`|HTTP timeout in milliseconds\.|

**The ****` req `**** subfield **: 

|Field|Instructions|
|---|---|
|`rgb_image`|Base64 images can be directly obtained by`eco_ImageQuery`\.|
|`labels`|List of target labels to be detected\.|
|`position_region`|Location area\. |
|`payload`|Carrier\.|
|`ovd_property.color/shape/person`|Attribute constraints such as color, shape, and character\. |
|`ovd_obj_thresh`|OVD target threshold, default `0.35`\. |
|`box_obj_thresh`|Box target threshold, default `0.35`\. |
|`entry`|Service endpoints,`OvdEndpoint::OVD` corresponds to `/ovd`,`OvdEndpoint::SHOE` corresponds to `/shoe`\.|

**Return Value **: 

The full version returns `std::unique_ptr<DetectObjectsResponse>`, which may be `nullptr` when network or parsing fails; when non\-null, judge the business result via `code` / `msg`, and read `items` and `items_unknown` when successful\. The Simple version returns `bool`, where `true` indicates success\.单个 `ObjectDetection` 包含 `index`、`uuid`、`name`、`bbox`（`[x1,y1,x2,y2]`）和 `ovd_property`。

**C\+\+ Example**: 

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

#### 6\.1\.2 `eco_ImageEmbeddingExtract`

**Signature**\(`img_match.h`\):

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

**Overview**: 

Image feature extraction\. 

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req.image_list`|Image base64 list\.|
|`embeddings`|Only used in the Simple version, outputs a list of vectors upon success\. |
|`error_msg`|Only used in the Simple version, outputs error description when it fails\. |
|`timeout_ms`|HTTP timeout in milliseconds\.|

**Return Value **: 

The full version returns ` std::unique_ptr<ImageEmbeddingResponse> `; the Simple version returns ` bool `, ` true ` indicating success\. 

**C\+\+ Example**: 

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

#### 6\.1\.3 `eco_FusedSimilarityHybrid`

**Signature**\(`img_match.h`\):

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

**Overview**: 

Fusion similarity calculation\. 

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req.image_a`|Optional image input A\.|
|`req.embedding_a`|Optional vector input A\.|
|`req.image_b`|Optional image input B\.|
|`req.embedding_b`|Optional vector input B\. |
|`similarity`|Only used in the Simple version, outputs similarity when successful\. |
|`error_msg`|Only used in the Simple version, outputs error description when it fails\. |
|`timeout_ms`|HTTP timeout in milliseconds\.|

**Return Value **: 

The full version returns `std::unique_ptr<FusedSimilarityResponse>`; the Simple version returns `bool`, `true` indicating success\. 

**C\+\+ Example**: 

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

#### 6\.1\.4 `eco_PairwiseMatch`

**Signature**\(`img_match.h`\):

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

**Overview**: 

Pair matching\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req.user_id`|Optional user identifier\.|
|`req.images`|Collection of images and detection boxes\.|
|`items`|Only used in the Simple version, outputs the matching result when successful\. |
|`error_msg`|Only used in the Simple version, outputs error description when it fails\. |
|`timeout_ms`|HTTP timeout in milliseconds\.|

**Return Value **: 

The full version returns `std::unique_ptr<PairwiseResponse>`; the Simple version returns `bool`, `true` indicating success\. 

**C\+\+ Example**: 

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

#### 6\.1\.5 `eco_ShoesIntentPlan`

**Signature** \(`shoes_intent.h`\) : 

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

**Overview**: 

Shoes Intent Planning\. ` When move_all=true `, call `/v038/test/shoes_intent/plan ` to generate a full\-scale reorganization plan; ` When move_all=false `, call `/v038/test/shoes_intent/check ` to check if shoes need adjustment\. 

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req`|`ShoesIntentPlanRequest `, fields are shown in the table below\. |
|`move_all`|`true` indicates generating a full consolidation plan;`false` indicates check mode\.|
|`steps`|Only used in the Simple version, outputs the step sequence upon success\. |
|`final_bboxes`|Only used in the Simple version, outputs the final layout upon success\. |
|`error_msg`|Only used in the Simple version, outputs error description when it fails\.|
|`timeout_ms`|HTTP timeout in milliseconds\.|

**The ****` req `**** subfield **: 

|Field|Instructions|
|---|---|
|`shoes`|The shoe list, each item contains `bbox`, `pair_id`, and `is_tilted`\. |
|`shoes[].bbox`|Single shoe detection box, format `[xmin, ymin, xmax, ymax]`\. |
|`shoes[].pair_id`|Pairing ID; if unpaired, it can be `0`\. |
|`shoes[].is_tilted`|Whether the shoes are misaligned is only used in the `move_all=false` inspection mode\.|
|`place_line`|Wall line point sequence, each point is `[x, y]`\. |
|`place_plane`|Point sequence of the temporary placement area\.|

**Return Value **: 

The full version returns `std::unique_ptr<ShoesIntentPlanResponse>`: When non\-null, judge the business result via `code` / `msg`, and read `steps` and `final_bboxes` when successful\. The Simple version returns `bool`, `true` indicating success\. 

**C\+\+ Example**: 

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

#### 6\.1\.6 `eco_DeskIntentPlan`

**Signature**\(`desk_intent.h`\):

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

**Overview**: 

Desktop item placement intention planning, return step\-by\-step `from_bbox `/ `to_bbox `with semantic explanation\. `judge_input `and `memory_input `are optional inputs, respectively, used for desktop organizing evaluation results and user historical preferences and other memory information\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req.image`|Base64 image\.|
|`req.user_input`|User natural language collates intent\.|
|`req.perception`|Desktop perception result list, each item contains `name` and `bbox`\.|
|`req.judge_input`|Optional evaluation input, e\.g\., "The middle area of the desktop needs to be left blank"\.|
|`req.memory_input`|Optional memory input, e\.g\., "User preference is to place dolls concentrated on the left side"\.|
|`steps`|Only used in the Simple version, outputs the step sequence upon success\. |
|`error_msg`|Only used in the Simple version, outputs error description when it fails\.|
|`timeout_ms`|HTTP timeout in milliseconds\.|

**Return Value **: 

The full version returns `std::unique_ptr<DeskIntentPlanResponse>`: When non\-null, judge the business result via `code` / `msg`, and read `user_input`, `plan` and `steps` when successful\. The Simple version returns `bool`, `true` indicating success\. 

**C\+\+ Example**: 

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

#### 6\.1\.7 `eco_DeskIntentPerception`

**Signature**\(`desk_intent.h`\):

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

**Overview**: 

Desktop perception, returning the names and pixel boxes of the desktop and items\. 

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req.image`|Base64 image\.|
|`objects`|Only used in the Simple version, outputs a list of items upon success, with each item containing `name` and `bbox`\.|
|`error_msg`|Only used in the Simple version, outputs error description when it fails\. |
|`timeout_ms`|HTTP timeout in milliseconds\.|

**Return Value **: 

The full version returns `std::unique_ptr<DeskIntentPerceptionResponse>`: When non\-null, judge the business result via `code` / `msg`, and read `objects` when successful\. The Simple version returns `bool`, `true` indicating success\. 

**C\+\+ Example**: 

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

#### 6\.1\.8 `eco_DeskIntentMatch`

**Signature**\(`desk_intent.h`\):

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

**Overview**: 

Map the bbox in the head camera image to the bbox in the hand camera image\. 

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req.arm_image`|Base64 of hand camera image\.|
|`req.base_image`|Base64 of the head camera image\.|
|`req.bbox`|The box to be matched in the head camera image `[x1, y1, x2, y2]`\. |
|`out_bbox`|Only used in the Simple version, outputs a 4\-element bbox in the hand camera image upon success\.|
|`error_msg`|Only used in the Simple version, outputs error description when it fails\. |
|`timeout_ms`|HTTP timeout in milliseconds\.|

**Return Value **: 

The full version returns `std::unique_ptr<DeskIntentMatchResponse>`: When non\-null, the business result is determined by `code` / `msg`, and `bbox` is read upon success\. The Simple version returns `bool`, `true` indicating success\. 

**C\+\+ Example**: 

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

#### 6\.1\.9 `eco_DeskIntentPrePerception`

**Signature**\(`desk_intent.h`\):

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

**Overview**: 

Desktop pre\-perception, recommending the head camera angle with the largest desktop field of view based on multiple images with different head angles\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req.images`|Input a list of images, each item containing `image_base64` and shooting angle `angle`\. |
|`angle`|Only used in the Simple version, outputs the recommended angle upon success\. |
|`error_msg`|Only used in the Simple version, outputs error description when it fails\. |
|`timeout_ms`|HTTP timeout in milliseconds\.|

**Return Value **: 

The full version returns `std::unique_ptr<DeskIntentPrePerceptionResponse>`: When non\-null, judge the business result through `code` / `msg`, and read `angle` when successful\. The Simple version returns `bool`, `true` indicates success\. 

**C\+\+ Example**: 

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

#### 6\.1\.10 `eco_DeskIntentJudgeAction`

**Signature**\(`desk_intent.h`\):

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

**Overview**: 

Desktop organization result judgment, corresponding to Python`eco_vlm_judge`: Input the organized desktop image and the user's original requirements, and determine whether they meet the organization goal\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req`|`DeskIntentJudgeActionRequest`, fields are shown in the table below\.|
|`is_tidy`|Only used in the Simple version, outputs whether it meets the sorting requirements upon success\. |
|`reason`|Only used in the Simple version, outputs the judgment reason upon success\. |
|`score`|Only used in the Simple version, outputs a score upon success\. |
|`error_msg`|Only used in the Simple version, outputs error description when it fails\. |
|`timeout_ms`|HTTP timeout in milliseconds\.|

**The ****` req `**** subfield **: 

|Field|Instructions|
|---|---|
|`tidy_image`|base64 image\.|
|`user_input`|User's original organized requirements\.|

**Return Value **: 

The full version returns `std::unique_ptr<DeskIntentJudgeActionResponse>`: When non\-null, the business result is judged via `code` / `msg`, and when successful, `is_tidy`, `reason`, and `score` are read\. The Simple version returns `bool`, `true` indicating success\. 

**C\+\+ Example**: 

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

#### 6\.1\.11 `eco_PutWhere`

**Signature**\(`put_where.h`\):

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

**Overview**: 

Placement recommendation, return the complete response structure\. 

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`req`|`PutWhereRequest`, fields are shown in the table below\.|
|`final_answer`|Only used in the Simple version, outputs the final recommended text upon success\.|
|`timeout_ms`|HTTP timeout in milliseconds\.|

**The ****` req `**** subfield **: 

|Field|Instructions|
|---|---|
|`carrier`|Name or description of the load\. |
|`carrier_direct`|Carries the orientation information of the load\. |
|`image`|Base64 image\.|
|`image_height`|Image height\. |
|`image_width`|Image width\. |
|`placeholder`|Name of the item to be placed\.|
|`summary`|Summary information, such as "Put the toy on the table"\. Cannot be empty\.|

**Return Value **: 

The full version returns `std::unique_ptr<PutWhereResponse>`, which may be `nullptr` when network or parsing fails; when non\-null, the business result is judged by `code` / `msg`, and `final_answer` is read upon success\. The Simple version returns `bool`, where `true` indicates success\. 

**C\+\+ Example**: 

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

### 6\.2 Mapping and Navigation

This section of the interface requires ` Client::Connect(...) `\. 

#### 6\.2\.1 `eco_AutoMapCreate`

**Signature**\(`robot_func.h`\):

```C++
ErrorCode eco_AutoMapCreate(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_AutoMapCreateAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**: 

Automatic Mapping Task 

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`out_task_id`|Only used in the asynchronous version, outputs the task ID\.|
|`on_done`|Used only in the asynchronous version, callback after the task ends`ErrorCode`\.|
|`timeout_ms`|Timeout in milliseconds, default `180000`\. |

**Return Value **: 

`ErrorCode`。

**C\+\+ Example**: 

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

#### 6\.2\.2 `eco_SemanticMapCreate`

**Signature**\(`robot_func.h`\):

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

**Overview**: 

Semantic Mapping Task\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`data.area_info`|Semantic area list, each item contains `area_id`, `area_name`\. |
|`out_task_id`|Only used in the asynchronous version, outputs the task ID\. |
|`on_done`|Used only in the asynchronous version, callback after the task ends`ErrorCode`\.|
|`timeout_ms`|Timeout milliseconds\.|

**Return Value **: 

`ErrorCode`。

**C\+\+ Example**: 

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

#### 6\.2\.3 `eco_SemanticMapManager`

**Signature**\(`robot_func.h`\):

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

**Overview**: 

Addition, deletion, modification, and query of semantic map objects\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`data.cmd`|Management commands,`0` Add,`1` Query,`2` Modify,`4` Delete\.|
|`data.object_info`|Map object information, including `id`, `model_level`, `mssid`, `model_name`, `name`, `content`, `direction` and other fields\.|
|`notify_out`|Only used in the synchronous version, outputs `objects_info`, can pass `nullptr`\.|
|`out_task_id`|Only used in the asynchronous version, outputs the task ID\. |
|`on_done`|Used only in the asynchronous version, callback `ErrorCode` and `SemanticMapManagerNotifyData`\.|
|`timeout_ms`|Timeout milliseconds\.|

**Return Value **: 

`ErrorCode`。

**C\+\+ Example**: 

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

#### 6\.2\.4 `eco_ChassisMove`

**Signature** \( `robot_func.h` \) : 

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

**Overview**: 

Chassis movement control\. 

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`data.move_distance`|Moving distance, unit: meter\. |
|`data.move_angle`|Rotation angle, unit: radian\.|
|`out_task_id`|Only used in the asynchronous version, outputs the task ID\. |
|`on_done`|Only used in asynchronous versions, callback to `ErrorCode `after task completion\.|
|`timeout_ms`|Timeout milliseconds\.|

**Return Value **: 

`ErrorCode`。

**C\+\+ Example**: 

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

#### 6\.2\.5 `eco_PointNavigation`

**Signature** \( `robot_func.h` \) : 

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

**Overview**: 

Fixed\-point navigation\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`nav_data.x`|Target `x` coordinate, unit: meter\.|
|`nav_data.y`|Target `y` coordinate, unit: meter\.|
|`nav_data.yaw`|Target orientation, unit: radian\.|
|`out_task_id`|Only used in the asynchronous version, outputs the task ID\. |
|`on_done`|Only used in asynchronous versions, callback to `ErrorCode `after task completion\.|
|`timeout_ms`|Timeout milliseconds\.|

**Return Value **: 

`ErrorCode`。

**C\+\+ Example**: 

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

#### 6\.2\.6 `eco_SemanticNavigation`

**Signature** \( `robot_func.h` \) : 

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

**Overview**: 

Semantic Navigation\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`data.area_id`|Semantic Region ID\.|
|`data.area_name`|Semantic region name\.|
|`out_task_id`|Only used in the asynchronous version, outputs the task ID\.|
|`on_done`|Only used in the asynchronous version, callback after the task ends `ErrorCode`\. |
|`timeout_ms`|Timeout milliseconds\.|

**Return Value **: 

`ErrorCode`。

**C\+\+ Example**: 

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

#### 6\.2\.7 `eco_DockDown`

**Signature**\(`robot_func.h`\):

```C++
ErrorCode eco_DockDown(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_DockDownAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**: 

Lower the pile\. 

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`out_task_id`|Only used in the asynchronous version, outputs the task ID\.|
|`on_done`|Only used in the asynchronous version, callback after the task ends `ErrorCode`\. |
|`timeout_ms`|Timeout milliseconds\.|

**Return Value **: 

`ErrorCode`。

**C\+\+ Example**: 

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

#### 6\.2\.8 `eco_MapRelocation`

**Signature**\(`robot_func.h`\):

```C++
ErrorCode eco_MapRelocation(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_MapRelocationAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**: 

Map Relocation\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`out_task_id`|Only used in the asynchronous version, outputs the task ID\.|
|`on_done`|Only used in the asynchronous version, callback after the task ends `ErrorCode`\. |
|`timeout_ms`|Timeout milliseconds\.|

**Return Value **: 

`ErrorCode`。

**C\+\+ Example**: 

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

#### 6\.2\.9 `eco_Recharge`

**Signature**\(`robot_func.h`\):

```C++
ErrorCode eco_Recharge(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RechargeAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**: 

Return to charge\.

**Field** \(Request / Input Parameter\): 

|Name|Instructions|
|---|---|
|`out_task_id`|Only used in the asynchronous version, outputs the task ID\. |
|`on_done`|Used only in the asynchronous version, callback after the task ends`ErrorCode`\.|
|`timeout_ms`|Timeout milliseconds\.|

**Return Value **: 

`ErrorCode`。

**C\+\+ Example**: 

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

### 6\.3 Image and Pose

APIs in this section require `Client::Connect(...)` first\.

#### 6\.3\.1 `eco_ImageQuery`

**Signature** \(`robot_func.h`\):

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

Image capture\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`data.camera_type` / `camera_type`|Camera type: `CameraType::ARM` arm camera, `CameraType::HEAD` head camera\.|
|`result`|Sync version only; outputs `rgb_image`, `depth_image`, `tf_goal`, `camera_info_k`\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` and `ImageQueryNotifyData`\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.3\.2 `eco_GetPose`

**Signature** \(`robot_func.h`\):

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

Pose estimation\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`data.bbox`|List of 2D bounding boxes\.|
|`data.rgb_image`|RGB image\.|
|`data.depth_image`|Depth image\.|
|`data.tf_map`|Pose information in the map coordinate frame\.|
|`notify_out`|Sync version only; outputs `pose_results`\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` and `GetPoseNotifyData`\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

### 6\.4 High\-Level Tasks

APIs in this section require `Client::Connect(...)` first\.

#### 6\.4\.1 `eco_FindPerson`

**Signature** \(`robot_func.h`\):

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

Find\-person task\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`data.user_name`|User name\.|
|`data.user_id`|User ID\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.2 `eco_Search`

**Signature** \(`robot_func.h`\):

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

Search for objects\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`search_data.object`|Semantic description of the object, including `item`, `color`, `shape`\.|
|`search_data.filter_boxes`|List of filter boxes\.|
|`search_data.area`|Area constraint, including `area_id`, `area_name`\.|
|`result`|Sync version only; outputs matched `BoxData`\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` and `BoxData`\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.3 `eco_RobotPreparePose`

**Signature** \(`robot_func.h`\):

```C++
ErrorCode eco_RobotPreparePose(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RobotPreparePoseAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Robot prepare pose\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.4 `eco_RobotEndingPose`

**Signature** \(`robot_func.h`\):

```C++
ErrorCode eco_RobotEndingPose(int timeout_ms = kDefaultTaskCompletionTimeoutMs);
ErrorCode eco_RobotEndingPoseAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskCompletionTimeoutMs);
```

**Overview**:

Robot ending pose\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.5 `eco_LookTo`

**Signature** \(`robot_func.h`\):

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

Observation alignment \(`hand_observe` / `head_observe`\); use `target` to specify hand or head\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`target`|Body part for observation; `LookToTarget::HAND` for hand, `LookToTarget::HEAD` for head\.|
|`data.position/orientation/box_length/frame_id`|6D box of the observation target\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.6 `eco_AccurateGrab`

**Signature** \(`robot_func.h`\):

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

Accurate grab\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`data.rgb_image`|RGB image\.|
|`data.depth_image`|Depth image\.|
|`data.tf_goal`|Pose in the coordinate frame\.|
|`data.bbox`|Bounding box of the grab target\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.7 `eco_MatchObjViews`

**Signature** \(`img_match.h`\):

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

Cross\-view object matching: given a head\-view reference bbox, arm\-view image, and arm\-view detections, select the same object\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`head_image`|Head camera RGBD image, typically from `eco_ImageQuery(CameraType::HEAD)`\.|
|`ref_bbox`|Reference bounding box in the head camera image, `[x1,y1,x2,y2]`\.|
|`arm_image`|Arm camera RGBD image, typically from `eco_ImageQuery(CameraType::ARM)`\.|
|`arm_items`|Arm\-view detection results; elements are of type `ObjectDetection`\.|
|`out`|On success, outputs the matched arm\-view target\.|
|`error_msg`|Used by the version with error message; outputs error description on failure\.|
|`timeout_ms`|HTTP timeout in milliseconds\.|

**Return value**:

`bool`: `true` means matching succeeded; the result is written to `out`\.

**C\+\+ example**:

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

#### 6\.4\.8 `eco_PickWithArmReview`

**Signature** \(`robot_func.h`\):

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

Grab composite flow with arm review: first perform hand observation using the reference\-view bbox, then recapture with the arm camera, re\-detect with OVD, match across views, and finally execute `eco_AccurateGrab`\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`view`|Reference\-view image, typically the result of `eco_ImageQuery(CameraType::HEAD)`\.|
|`object_bbox`|Reference\-view target bounding box; `name` is used as the OVD label for the arm image; defaults to `object` when empty\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Task completion timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.9 `eco_AccuratePlace`

**Signature** \(`robot_func.h`\):

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

Accurate place\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`data.object`|Semantic description of the object\.|
|`data.position`|Placement position\.|
|`data.orientation`|Placement orientation\.|
|`data.box_length`|Target dimensions\.|
|`data.frame_id`|Target coordinate frame\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.10 `eco_PlaceWithView`

**Signature** \(`robot_func.h`\):

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

Place using head\-view pixel point or pixel box: by default uses `semantic_place`; when `recapture_place=true`, back\-projects pixels to 3D and executes `accurate_place`\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`view`|Head camera RGBD image, typically from `eco_ImageQuery(CameraType::HEAD)`\.|
|`place_ref`|Length 2 for pixel point `[x,y]`; length 4 for pixel box `[x1,y1,x2,y2]`; the center point is used internally\.|
|`recapture_place`|`false` runs `semantic_place`; `true` back\-projects pixels and runs `accurate_place`\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.11 `eco_PlaceIn`

**Signature** \(`robot_func.h`\):

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

Recommended placement inside a container: uses the first 4 elements of `carrier_bbox` as the container box and places according to the "inside" protocol of `semantic_place`\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`view`|Head camera RGBD image, typically from `eco_ImageQuery(CameraType::HEAD)`\.|
|`carrier_bbox`|Container bounding box; at least 4 values, used as `[x1,y1,x2,y2]`\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Task completion timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.4\.12 `eco_Speech`

**Signature** \(`robot_func.h`\):

```C++
ErrorCode eco_Speech(const std::string& text);
```

**Overview**:

Robot app text\-to\-speech\. Suitable for demo flow completion, task errors, or on\-site prompts\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`text`|Text content to speak\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

### 6\.5 Body Control

APIs in this section require `Client::Connect(...)` first\.

#### 6\.5\.1 `eco_RobotHeightCtrl`

**Signature** \(`robot_func.h`\):

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

Robot body height control\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`data.value`|Height value, range `[0, 0.44]` meters\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.5\.2 `eco_RobotHeadCtrl`

**Signature** \(`robot_func.h`\):

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

Robot head control\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`data.value`|Head angle, range `[0, 1.3]` radians\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.5\.3 `eco_RobotArmCtrl`

**Signature** \(`robot_func.h`\):

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

Robot arm control\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`data.mode`|Control mode; common values: `2` homing, `6` gripper open, `7` gripper close, `9` box\-opening pose, `40` maintenance pose\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.5\.4 `eco_RobotPoseCtrl`

**Signature** \(`robot_func.h`\):

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

Manipulator pose control\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`data.object_pose`|Target pose list; each item includes `frame_id`, `position`, `orientation`\.|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Timeout in milliseconds\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

### 6\.6 Task Pause / Resume / Cancel

APIs in this section require `Client::Connect(...)` first\.

#### 6\.6\.1 `eco_StopAll`

**Signature** \(`robot_func.h`\):

```C++
ErrorCode eco_StopAll(int timeout_ms = kDefaultTaskResponseTimeoutMs);
ErrorCode eco_StopAllAsync(
    std::string& out_task_id,
    RobotTaskCallback on_done,
    int timeout_ms = kDefaultTaskResponseTimeoutMs);
```

**Overview**:

Cancel all tasks\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`out_task_id`|Async version only; outputs task ID\.|
|`on_done`|Async version only; callback with `ErrorCode` when the task completes\.|
|`timeout_ms`|Response timeout in milliseconds; default `10000`\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

#### 6\.6\.2 `eco_PauseMission` / `eco_ResumeMission` / `eco_CancelMission`

**Signature** \(`robot_func.h`\):

```C++
ErrorCode eco_PauseMission(const std::string& task_id);
ErrorCode eco_ResumeMission(const std::string& task_id);
ErrorCode eco_CancelMission(const std::string& task_id);
```

**Overview**:

Pause, resume, or cancel a specific task; corresponds to Python `eco_missionControl` `pause` / `resume` / `cancel`\. For tasks controlled via pause/resume, the SDK's `timeout_ms` while waiting for completion pauses during pause and resumes counting after resume\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`task_id`|Task ID, typically from `out_task_id` returned by an async API\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

### 6\.7 Robot Status

APIs in this section require `Client::Connect(...)` first\. After the SDK connects, it automatically fetches full RobotInfo once and caches it; subsequent `robot_info` reports update the cache incrementally\.

#### 6\.7\.1 `eco_RobotInfo`

**Signature** \(`robot_info.h`\):

```C++
ErrorCode eco_RobotInfo(RobotInfoData& out);
```

**Overview**:

Get the latest robot status cache maintained internally by the SDK\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`out`|Outputs the latest full cache snapshot\.|

**Return value**:

`ErrorCode`: `code==0` means the cache was retrieved; non\-zero means the cache is not yet initialized or internal state is unavailable\.

**C\+\+ example**:

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

#### 6\.7\.2 `eco_RefreshRobotInfo`

**Signature** \(`robot_info.h`\):

```C++
ErrorCode eco_RefreshRobotInfo(const std::vector<std::string>& topics);
```

**Overview**:

Manually refresh specified fields in the SDK's internal RobotInfo cache\. When `topics` is empty, refreshes all fields\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`topics`|List of status topics to refresh, e\.g\. `battery`, `pos`, `workState`; empty means refresh all\.|

**Return value**:

`ErrorCode`: `code==0` means the oneshot fetch succeeded and was merged into the cache\.

**C\+\+ example**:

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

#### 6\.7\.3 `eco_RobotWorkState` \~ `eco_RobotFurniture`

The following APIs read the corresponding sub\-state from the cache:

|API|Output type|Key fields|
|---|---|---|
|`eco_RobotWorkState(WorkState& out)`|`WorkState`|`task_id`, `name`, `cmd`|
|`eco_RobotBattery(BatteryInfo& out)`|`BatteryInfo`|`value`, `is_charge`, `mode`|
|`eco_RobotAlarm(std::vector<int>& out)`|Alarm code list|Exception codes|
|`eco_RobotPosition(RobotPos& out)`|`RobotPos`|`room`, `x`, `y`, `yaw`|
|`eco_RobotMapInfo(MapInfo& out)`|`MapInfo`|`mid`, `mname`, dimensions, origin, grid data|
|`eco_RobotFurniture(FurnitureInfo& out)`|`FurnitureInfo`|Furniture list \(`fid`, `fname`, `mssid`\)|

All return `ErrorCode`; non\-zero means the cache is not yet initialized or the corresponding field is unavailable\.

**C\+\+ example**:

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

#### 6\.7\.4 `eco_RegisterRobotInfoCallback` / `eco_UnregisterRobotInfoCallback`

**Signature** \(`robot_info.h`\):

```C++
ErrorCode eco_RegisterRobotInfoCallback(
    std::string& out_registration_id,
    RobotInfoReportCallback on_report);
ErrorCode eco_UnregisterRobotInfoCallback(const std::string& registration_id);
```

**Overview**:

Register / unregister local `robot_info` incremental callbacks\. Callback parameter is `RobotInfoPartial`\.

**Fields** \(request / input\):

|Name|Description|
|---|---|
|`out_registration_id`|Outputs local registration ID\.|
|`on_report`|Callback parameter is `RobotInfoPartial`\.|
|`registration_id`|Registration ID returned at registration time; used when unregistering\.|

**Return value**:

`ErrorCode`\.

**C\+\+ example**:

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

### 6\.8 Additional C\+\+ APIs

The following APIs exist only in the C\+\+ public headers and are therefore grouped at the end\.

#### 6\.8\.1 `eco_GetDefaultLinkUrl`

**Signature** \(`client.h`\):

```C++
std::string eco_GetDefaultLinkUrl();
```

**Overview**:

Returns the SDK default connection URL\. Built from environment variable `ECO_ROBOT_HOST` \(default `10.10.10.11`\) and port `9900`\.

**C\+\+ example**:

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

#### 6\.8\.2 `eco_SetCancellationFlag`

**Signature** \(`robot_func.h`\):

```C++
void eco_SetCancellationFlag(std::atomic<bool>* flag);
```

**Overview**:

Sets a cooperative cancellation flag\. When the external `atomic<bool>` pointer is set to `true`, SDK internal wait loops end as soon as possible \(returning `code==4` interrupted\)\. See §5\.2 Cooperative Cancellation for usage\.

**C\+\+ example**:

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

## 7\. Learning Path

If you are using the C\+\+ SDK for the first time, follow this order:

|Stage|Goal|Topics|
|---|---|---|
|**1\. Prepare environment**|Able to compile|Read §1 Overview → §2 Requirements → §3 Obtaining the package|
|**2\. Build and run**|Demo runs successfully|Read §4 Build and Run|
|**3\. First connection**|Code connects to the robot|Run the example in §5\.1 Connection and Basic Calls|
|**4\. Basic control**|Move \+ capture images|Read §6\.2\.4 `eco_ChassisMove` and §6\.3\.1 `eco_ImageQuery`|
|**5\. Perception and detection**|Recognize objects|Read §6\.1\.1 `eco_DetectObjects`|
|**6\. Pick and place**|Complete pick\-and\-place loop|Read §6\.4\.5 `eco_LookTo` → §6\.4\.6 `eco_AccurateGrab` → §6\.4\.11 `eco_PlaceIn`; simplified flow in §6\.4\.8 `eco_PickWithArmReview`|
|**7\. Mapping and navigation**|Autonomous robot movement|Read §6\.2\.1, §6\.2\.5, §6\.2\.6|
|**8\. Desk organization \(VLM\)**|AI\-driven organization|Read §6\.1\.6 `eco_DeskIntentPlan` and its composition chain|
|**9\. Advanced usage**|Async, status subscription|Read §5\.2 \(cancellation\), §6\.7 \(status callbacks\)|

> **Troubleshooting**: If you run into problems at any point, see §8 Troubleshooting first\.

---

## 8\. Troubleshooting

Common issues grouped by category for quick diagnosis\.

### 8\.1 Connection failure

- Check `ws://<robot_ip>:9900/` connectivity, on\-device services, and firewall\.

- Confirm `ECO_ROBOT_HOST` is set to the IP shown on the robot screen\.

- Confirm the robot\-side body control service \(WebSocket/9900\) is running normally\.

### 8\.2 Shared library not found

Typical symptom:

```Plaintext
error while loading shared libraries: librobot_sdk.so
```

Remediation:

- Distribute `out/<arch>/` as a whole \(including `librobot_sdk.so.0`\)\.

- Set `LD_LIBRARY_PATH`:

```Bash
export LD_LIBRARY_PATH=$PWD/out/x86:$LD_LIBRARY_PATH
```

- Or set rpath at compile time:

```Bash
# 编译期指定 rpath
g++ ... -Wl,-rpath,'$ORIGIN/../lib' -lrobot_sdk
```

### 8\.3 ARM build failure

- Check the `aarch64-linux-gnu` toolchain and CMake compiler settings\.

- Confirm `ARCH=arm` is set correctly\.

- When cross\-compiling OpenCV, confirm `OpenCV_DIR` points to libraries for the correct architecture\.

### 8\.4 Task cannot be interrupted

Use `eco_SetCancellationFlag` for cooperative cancellation, together with signal handling:

```C++
std::atomic<bool> cancelled{false};
robot_sdk::eco_SetCancellationFlag(&cancelled);
// Ctrl+C 处理器中设置:
cancelled = true;
```

To ensure robot\-side tasks are also terminated, use `eco_StopAll()` or `eco_CancelMission(task_id)` as well\.

### 8\.5 HTTP returns null pointer

- Check the HTTP service address and that `ECO_ROBOT_HOST` is correct\.

- Check that request fields are complete and image encoding is valid base64\.

- When the full API \(e\.g\. `eco_PutWhere`\) returns `nullptr`, it is usually a network/parsing issue; try the Simple version first to inspect `error_msg`\.

