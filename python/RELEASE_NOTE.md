# Release 1.1.0

## 重大变更（Breaking Changes）

- **SlamMapCreateCmd / eco_slamMapBuild 移入私有协议**：`SlamMapCreateCmd` 从公开 API（`types.py`、`__init__.py`）移除，`eco_slamMapBuild` 从 `BajieRobot` 公共接口移除。二者迁移至 `private_protocol/private_missions.py` 的 `PrivateMixin.slamMapCreate`，不再作为公开发布接口。
- **eco_robotSpeech 改为 fire-and-forget**：不再等待回复与返回 `MissionStatus`，调用后立即返回 `task_id` 字符串。旧版阻塞式调用需改用 `eco_missionControl` 或直接使用私有协议。

## WebSocket 可靠性增强

- **mission 两阶段等待**：`_wait_mission` 改为先等握手 response（校验错误码），再等 finish，杜绝"已发但未握手"的静默失败
- **断连自动重连**：发送 mission 前检测 WebSocket 断连，自动重连一次后重试发送
- **连接状态日志**：WebSocket 连接/关闭/错误时输出 `[bajie_sdk]` 前缀日志到 stderr，方便排查
- **断连唤醒**：`_on_close` 时 `notify_all` 唤醒所有等待线程，避免连接断开后无限阻塞
- **断连超时提示**：任务等待期间检测到断连，错误信息明确提示"连接断开"与排查建议

## 接口变更

| 变更说明 |
|---------|
| `SlamMapCreateCmd` 从公开 API 移除，迁移至 `private_protocol.private_missions` |
| `eco_slamMapBuild()` 从 `BajieRobot` 移除，改为 `PrivateMixin.slamMapCreate()` |
| `eco_robotSpeech(text, *, code, timeout_sec, on_task_done)` → `eco_robotSpeech(text)` — fire-and-forget，立即返回 `str` |
| `project_place_line_and_plane()` 新增 `ground_options` 参数 — 地面检测配置（搜索矩形、有效点比例、水平阈值等 10 项可调参数） |

## 协议容错

- **多余字段防护**：`_safe_init()` 过滤服务端协议中多余的 key，只传 dataclass 声明的字段构造对象，防止未知字段导致 `__init__` 异常

## 文档变更

- **文档注释格式统一**：`.. note::` / `.. attention::` RST 指令改为 `>` Markdown blockquote 风格，提升 IDE hover 可读性
- **eco_pairwise 文档增强**：补充多帧配对语义说明、参数结构与返回值映射关系
- **eco_calc_similarity 文档优化**：输入约束说明格式调整

## 内部优化

- `uuid.uuid4().hex` → `uuid.uuid4().hex[:8]`，缩短内部 token/task_id 长度
- `ping_interval` 设为 0 禁用自动 ping，依赖 TCP keepalive

# Release 0.4.0

## 新增功能

- **开源许可证**：LICENSE 切换为 Apache-2.0，项目正式开源发布
- **eco_robotSpeech 语音播报**：新增语音播报接口，支持 app 模式下文本转语音输出
- **validate_rgbd_view 校验**：新增 RGBD 视图校验函数，统一感知接口输入的合法性检查

## 接口变更

| 变更说明 |
|---------|
| `eco_match_obj_views` 参数简化：`source_view`/`target_view` → `source_img`/`target_img` |
| `FindPersonRequest` 移除未使用的 `area_info` 字段 |
| `FurnitureItem` 类型替换为 `SemanticMapManagerObjectInfo` |

# Release 0.3.1

## 新增功能

- **PairedBBox 数据类型**：新增 `PairedBBox(bbox, pair_id)` 结构化配对数据，替代裸 `((x1,y1,x2,y2), id)` 元组，提升可读性与类型安全
- **鞋子放置规划双模式**：`eco_shoe_placement_plan` 新增 `move_all` 参数，支持 `plan`（仅规划）和 `check`（规划+检查）两种模式
- **告警码模块**：新增 `alarm_codes.py` 模块与 `CLI -a` 查询命令，支持查看当前所有活跃告警码
- **CLI 错误区分**：任务超时（408）与任务已在运行（8197）分别给出不同提示与处理建议
- **JSON 模板枚举行内注释**：`gen_json_template` 输出枚举成员的 `name`，并递归扫描嵌套 dataclass，生成 `#` 行内注释标注每个枚举字段的可用选项
- **CLI `-j`/`-t` 短标志**：`run.py` 新增 `-j`（`--json`）和 `-t`（`--json-template`）短标志，兼容旧长选项
- **JSON 行注释支持**：`json_inline_refs` 新增 `_strip_json_comments` 过滤 JSON 中的 `#` 行注释，输入更灵活
- **CLI 结果保存提示**：`run --output_path` 保存成功后在 stderr 输出路径确认
- **CLI 补全优化**：`completion.sh run` 子命令补全 `-j`/`-t`，`--json` 已选后不重复弹出文件列表
- **mission 失败告警码展示**：mission 失败时 `code=9991` 自动展示当前活跃告警码列表

## 接口变更

| 变更说明 |
|---------|
| `eco_shoe_placement_plan` 新增 `move_all` 参数（plan/check 双模式） |
| `DetectObjectsResponse` 移除 `labels` 回显字段 |
| `PairedBBox` 类型代替裸 `tuple` 参数 |

## 文档变更

- 同步更新 SDK 开发文档至 v0.3.1
- 新增 WebSocket 通信协议独立文档

## 旧版本日志

### Release 0.3.0

## 新增功能

- **CLI `--json` 可选**：缺省使用默认参数，无需为全默认方法传 `--json '{}'`；缺少必填参数时自动提示接口文档与补全指令
- **CLI 错误提示增强**：任务 8197（已在运行）自动提示取消/查状态命令；缺少 `--ws-url` 时明确区分 Linux 板 IP 与屏幕安卓 IP
- **CLI `on_task_done` 提示**：调用返回后自动提示阻塞/后台模式说明
- **接口变动检测增强**：`diff_interfaces.py` 新增模块级常量（`OVD_LABELS` 等）和字段默认值变更检测
- **OVD 标签扩展**：新增"语义地图"、"玩具"、"衣物"、"鞋子" 4 个父类标签

## 接口变更

| 类型 | 变更说明 |
|------|---------|
| `DetectObjectsRequest.task_flag` | 删除 |
| `PutWhereSummaryRequest.summary` | 默认值 `""` → `"null"` |
| `eco_missionControl` | 返回类型 `bool` → `MissionStatus`；新增 `timeout_sec` 参数；返回值含错误码与版本信息 |

## 文档变更

- 文档中 "HTTP 层" 等传输层描述改为 "SDK"，隐藏底层实现
- 快速开始页明确标注 Linux 板 IP 通过网页配置，非屏幕安卓 IP

### Release 0.2.0

## 新增功能

- **VLM 桌面整理集成**：`eco_vlm_desk_sort_plan` / `eco_vlm_perception` / `eco_vlm_judge` 等 5 个感知端点，支持桌面场景理解与抓放规划
- **鞋子放置规划**：新增 `eco_shoe_placement_plan`，统一鞋子抓取与放置流程
- **OVD 标签校验**：SDK 侧对标签合法性做校验，防止非法标签透传至服务端
- **地图数据解码**：`MapInfo.value` 自动从 base64(zstd) 解码为 `numpy.ndarray`
- **CLI 增强**：支持 `--json-template` 输出请求模板、`--background` 后台任务模式、`@(file)` 图片内联引用、PNG 直接保存
- **错误码查询**：`python3 -m bajie_sdk -e <关键词>` 支持 581 条错误码模糊搜索与详情查看
- **统一任务管控**：新增 `eco_missionControl`，支持通过 eco_* 接口名直接下发任务

## 接口变更

| 旧接口 | 状态 | 新接口/说明 |
|--------|------|-------------|
| `eco_project_shoe_place_line` | 移除 | 合并入 `eco_shoe_placement_plan` |
| `place_plane` | 重命名 | `tmp_palce_area` |
| `project_place_line_and_plane` | 重命名 | `project_shoe_place_line` |
| `eco_manageSemanticPart` | 迁移 | 移至私有协议 |
| `eco_desk_intent_plan` / `eco_desk_intent_perception` 等 | 重命名 | `eco_vlm_*` 系列 |
| 任务回调（5 个方法） | 变更 | 改为 `on_task_done` 非阻塞回调模式 |

协议变更：
- `eco_manageSemanticMap` 请求字段精简，删除 `type`、`app_*` 字段
- `content`、`direction` 字段改为米单位二维数组
- `put_where` 的 `user_id` 标识 SDK

## 新增依赖

- `numpy>=1.24.0` — 地图数据解码
- `opencv-python>=4.6.0` — 图片处理
- `zstandard>=0.23.0` — zstd 解压缩
