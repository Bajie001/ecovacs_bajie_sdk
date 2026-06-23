# Release 1.1.0

> C++ SDK / demo 相对于 0.4.2 的新增内容。

## 新增功能

- **C++ SDK 预编译库升级至 1.1.0**：`depend/lib/{x86,arm}` 更新为 `librobot_sdk.so.1.1.0`，并同步保留 `librobot_sdk.so` 链接名，使用方仍可通过 `-lrobot_sdk` 或 CMake 目标链接
- **OVD base_url 行为说明补充**：`eco_DetectObjects` / `eco_DetectObjectsSimple` 通过 `base_url` 指定 OVD 服务地址时，`yolo_label` 会复用该地址的 host 并切换到端口 `30005`

## 接口变更

| 变更说明 |
|---------|
|http请求的bug fix|

## 旧版本日志

### Release 0.4.2

> C++ SDK / demo 相对于 0.4.0 的新增内容。

#### 新增功能

- **C++ SDK 预编译库升级至 0.4.2**：`depend/lib/{x86,arm}` 更新为 `librobot_sdk.so.0.4.2`，并同步 CMake 导入目标文件，构建时会链接 0.4.2 版本动态库

#### 接口变更

| 变更说明 |
|---------|
| `ImageQueryData::camera_type` 改为强类型枚举 `CameraType`，取值为 `CameraType::ARM` / `CameraType::HEAD` |
| `eco_ImageQuery` / `eco_ImageQueryAsync` 新增直接接收 `CameraType` 的重载，调用方可使用 `eco_ImageQuery(CameraType::HEAD, result)` |
| 移除手推建图接口 |

#### 文档变更

- C++ 主 README 更新至 SDK 0.4.2，并同步 `eco_ImageQuery` 的 `CameraType` 参数说明
- demo 代码中的拍照调用已由 `camera_type=1/2` 适配为 `CameraType::ARM/HEAD`

### Release 0.4.0

> C++ SDK / demo 相对于 0.3.0 的新增内容。

#### 新增功能

- **C++ SDK 预编译库升级至 0.4.0**：`depend/lib/{x86,arm}` 更新为 `librobot_sdk.so.0.4.0`，并同步 CMake 导入目标文件，构建时会链接 0.4.0 版本动态库
- **语音播报接口**：新增 `eco_Speech`，支持通过 SDK 触发机器人 app 语音播报
- **demo 语音播报结果反馈**：`toy_storage_demo` 与 `delivery_demo` 在流程结束时调用 `eco_Speech` 播报成功、失败或中断结果，便于现场演示确认任务状态
- **开放源码许可证**：项目根目录补充 Apache-2.0 `LICENSE` 与 `NOTICE`，明确开源发布授权信息

#### 接口变更

| 变更说明 |
|---------|
| `FindPersonData` 保留 `user_name` / `user_id`，不再包含未使用的区域信息字段 |
| `eco_Speech(const std::string& text)` 新增为同步语音播报接口，返回 `ErrorCode` |

#### 文档变更

- C++ 主 README 更新至 SDK 0.4.0，补充 `eco_Speech` 接口说明
- 头文件版本注释更新为 `librobot_sdk.so.0.4.0`

### Release 0.3.0

> C++ SDK / demo 相对于 0.2.0 的新增内容。

#### 新增功能

- **C++ SDK 预编译库升级至 0.3.0**：`depend/lib/{x86,arm}` 更新为 `librobot_sdk.so.0.3.0`，同步 0.3.0 版本头文件与动态库
- **统一机器人主机环境变量**：默认连接 URL 与 HTTP 感知服务统一使用 `ECO_ROBOT_HOST`，替代旧的 `ANDROID_HOST` / `LINUX_HOST` 分散配置
- **RobotInfo 独立模块**：新增 `robot_info.h`，支持刷新并读取 SDK 内部维护的机器人状态缓存，包括工作状态、电量、告警、位置、地图与家具信息
- **OVD 标签处理增强**：OVD 请求前支持通过 `yolo_label` 服务做父类扩充与模糊对齐；服务不可用时退化为本地父类扩充，并过滤到 OVD 标签白名单
- **图像匹配能力增强**：图像特征提取、混合相似度、Pairwise 匹配接口补充 Simple 调用形式，并新增跨视图目标框匹配 `eco_MatchObjViews`
- **桌面整理 C++ demo 接入 SDK DeskIntent**：`desk_demo` 的桌面意图规划与 bbox 匹配由占位实现改为调用 `eco_DeskIntentPlanSimple` / `eco_DeskIntentMatchSimple`
- **桌面整理重试机制**：`desk_demo` 对语义导航、机身高度控制、头部控制、拍照、DeskIntent 等关键调用增加重试封装，提升现场执行稳定性
- **玩具收纳 / 配送 demo 增强**：`toy_storage_demo` 与 `delivery_demo` 改用 OVD 检测、PutWhere 推荐与 SDK 放置接口完成抓取放置闭环，并补充抓取 / 放置重试逻辑
- **连接中断处理**：玩具收纳 / 配送 demo 在 `Client::Connect` 前注册取消标志，连接阻塞阶段收到 `Ctrl+C` 可立即退出
- **PutWhere 结果解析辅助**：新增 `common/sdk_response_helpers.h`，统一 PutWhere 错误处理、SDK 错误格式化与图片 payload 日志输出
- **OpenCV 构建支持说明**：C++ 主 README 补充 `shoe_sorting_demo` 图像解码与可视化所需的 OpenCV 4.x 依赖说明
- **demo 文档补齐**：新增 `toy_storage_demo/README.md`、`delivery_demo/README.md`，并更新 `desk_demo/README.md` 与 C++ 主 README 的构建、运行、环境变量和接口说明
