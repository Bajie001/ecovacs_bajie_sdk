#ifndef ROBOT_SDK_TYPES_H_
#define ROBOT_SDK_TYPES_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace robot_sdk {

/** HTTP 服务请求默认超时时间。传入 timeout_ms 须 >= 0；0 表示无限等待。 */
constexpr int kDefaultHttpRequestTimeoutMs = 10000;

/** 通用错误码：与协议 body.data.error_code 对齐时可含 module。 */
struct ErrorCode {
  uint32_t code{0};   /**< 0 表示成功，非 0 见协议错误码表 */
  std::string msg;    /**< 描述信息 */
  std::string module; /**< 可选，模块名 */
};

/** 定点导航请求：x/y 单位 m，yaw 单位 rad（协议字段 yaw）。 */
struct PointNavigationData {
  float x{0};
  float y{0};
  float yaw{0};
};

/** 物体语义描述：名称、颜色、形状。 */
struct ObjectData {
  std::string item;
  std::string color;
  std::string shape;
};

/** 三维位置。 */
struct Position {
  float x{0};
  float y{0};
  float z{0};
};

/** 四元数姿态。 */
struct Orientation {
  float x{0};
  float y{0};
  float z{0};
  float w{0};
};

/** 长方体尺寸。 */
struct BoxLength {
  float x{0};
  float y{0};
  float z{0};
};

/** 6D 框：位姿 + 尺寸 + 坐标系 frame_id。 */
struct BoxData {
  Position position;
  Orientation orientation;
  BoxLength box_length;
  std::string frame_id;
};

/** 观测执行部位：手部或头部。 */
enum class LookToTarget {
  HAND,
  HEAD,
};

/** 相机类型：协议 camera_type，1 手臂相机，2 头部相机。 */
enum class CameraType : int {
  ARM = 1,
  HEAD = 2,
};

/** 区域：id 与名称至少其一非空（依协议）。 */
struct AreaData {
  std::string area_id;
  std::string area_name;
};

/** search 请求：物体描述、过滤框、区域。 */
struct SearchData {
  ObjectData object;
  std::vector<BoxData> filter_boxes;
  AreaData area;
};

/** 工作状态 workState。 */
struct WorkState {
  std::string task_id;
  std::string name;
  std::string cmd;     /**< 如 working / pause / idle */
};

/** 电量信息：value 百分比；is_charge；mode 充电模式。 */
struct BatteryInfo {
  int value{0};
  int is_charge{0};
  int mode{0};
};

/** 机器在地图上的位姿与房间名。 */
struct RobotPos {
  std::string room;
  float x{0};
  float y{0};
  float yaw{0};
};

/** 家具/区域一条。 */
struct FurnitureItem {
  std::string fid;
  std::string fname;
  std::string mssid; /**< 父级 id，空表示无父级 */
};

/** 家具列表。 */
struct FurnitureInfo {
  std::vector<FurnitureItem> info;
};

/** SLAM 地图信息。value 为 zstd 后 base64 结果，length 为原始栅格长度。 */
struct MapInfo {
  std::string mid;
  std::string mname;
  int32_t total_width{0};
  int32_t total_height{0};
  float origin_x{0};
  float origin_y{0};
  int32_t pixel{0};
  std::string value;
  int32_t length{0};
};

/** 机器状态全量快照（用于本地 Merge 累积）。 */
struct RobotInfoData {
  WorkState work_state;
  BatteryInfo battery;
  std::vector<int> alarm;
  RobotPos pos;
  MapInfo mapinfo;
  FurnitureInfo furniture;
};

/**
 * 机器状态增量：server 主动上报 / oneshot 子集时，仅 JSON 中出现的字段有 optional 值。
 */
struct RobotInfoPartial {
  std::optional<WorkState> work_state;
  std::optional<BatteryInfo> battery;
  std::optional<std::vector<int>> alarm;
  std::optional<RobotPos> pos;
  std::optional<MapInfo> mapinfo;
  std::optional<FurnitureInfo> furniture;
};

/** get_pose notify 单条结果。 */
struct PoseResultItem {
  int id{0};
  Position position;
  Orientation orientation;
  BoxLength box_length;
  std::string frame_id;
};

/** get_pose notify 聚合。 */
struct GetPoseNotifyData {
  std::vector<PoseResultItem> pose_results;
};

/** get_pose 请求中的二维框。 */
struct BboxItem {
  int id{0};
  float left_up_x{0};
  float left_up_y{0};
  float right_down_x{0};
  float right_down_y{0};
};

/** 语义建图区域一项。 */
struct SemanticMapAreaItem {
  std::string area_id;
  std::string area_name;
};

struct SemanticMapCreateData {
  std::vector<SemanticMapAreaItem> area_info;
};

/** chassis_move：move_distance 米；move_angle 弧度。 */
struct ChassisMoveData {
  float move_distance{0};
  float move_angle{0};
};

/** semantic_map_manager 的 object_info。 */
struct SemanticMapObjectInfo {
  std::string id; // 家具id/分区id/虚拟墙id
  int model_level{0}; // 1房间 2承载物 3虚拟墙 4自定义收纳区域 5自定义整理区域
  std::string mssid; // 所属分区id
  std::string model_name; // 类型名称，桌子、充电桩、柜子、房间等
  std::string name; // 名称，可自定义
  std::string content; // 轮廓信息-单位厘米，"-1650,1800,700;450,1800,700;..."
  std::string direction; // 家具正面朝向的线段-单位厘米，"-1650,1800,700;450,1800,700;"
};

/** semantic_map_manager 请求：cmd 与 object_info。 */
struct SemanticMapManagerData {
  int cmd{0}; //0-添加 1-查询 2-修改 4-删除
  SemanticMapObjectInfo object_info;
};

/** semantic_map_manager notify。 */
struct SemanticMapManagerNotifyData {
  std::vector<SemanticMapObjectInfo> objects_info;
};

/** semantic_navigation：area_id 和 area_name至少一个不为空，
    若都不为空则需要在语义地图中查询。 */
struct SemanticNavigationData {
  std::string area_id;
  std::string area_name;
};

/** find_person。 */
struct FindPersonData {
  std::string user_name;
  std::string user_id;
};

/** accurate_place：可选 object + 位姿。 */
struct AccuratePlaceData {
  ObjectData object;
  Position position;
  Orientation orientation;
  BoxLength box_length;
  std::string frame_id;
};

/** robot_height_ctrl：value 范围 [0, 0.44] 米，0 置底，0.44 置顶。 */
struct RobotHeightCtrlData {
  float value{0};
};

/** robot_head_ctrl：value 范围 [0, 1.3] 弧度，0 置底，0.83 水平，1.3 置顶。 */
struct RobotHeadCtrlData {
  float value{0};
};

/** robot_arm_ctrl：mode 2 归位，6 夹爪打开，7 夹爪关闭，9 开箱姿态，40 夹爪维护姿态。 */
struct RobotArmCtrlData {
  int mode{0};
};

/** robot_pose_ctrl 的单个指定 6D pose。 */
struct RobotPoseItem {
  std::string frame_id;
  Position position;
  Orientation orientation;
};

/** robot_pose_ctrl：机械臂到达指定 pose。 */
struct RobotPoseCtrlData {
  std::vector<RobotPoseItem> object_pose;
};

/** 时间戳与 frame_id。 */
struct Header {
  struct Stamp {
    int sec{0};
    int nanosec{0};
  } stamp;
  std::string frame_id;
};

/** 图像消息：含 base64 data。 */
struct ImageMsg {
  Header header;
  std::string data;
};

/** 二维像素框。 */
struct BoundingBox {
  std::string name;
  float left_up_x{0};
  float left_up_y{0};
  float right_down_x{0};
  float right_down_y{0};
};

/** 带 header 的位姿。 */
struct PoseStamped {
  Header header;
  struct Pose {
    Position position;
    Orientation orientation;
  } pose;
};

/** accurate_grab 请求。 */
struct AccurateGrabData {
  ImageMsg rgb_image;
  ImageMsg depth_image;
  PoseStamped tf_goal;
  BoundingBox bbox;
};

/** get_pose 请求。 */
struct GetPoseData {
  std::vector<BboxItem> bbox;
  ImageMsg rgb_image;
  ImageMsg depth_image;
  PoseStamped tf_map;
};

/** semantic_place 请求。 */
struct SemanticPlaceData {
  ImageMsg rgb_image;
  ImageMsg depth_image;
  PoseStamped tf_goal;
  std::string direction; /**< 如「里」「上」 */
  std::string meta_data;   /**< 推荐放计算结果 JSON 字符串 */
};

/** image_query：camera_type 1 手臂相机，2 头部相机。 */
struct ImageQueryData {
  CameraType camera_type{CameraType::HEAD};
};

/** image_query notify。 */
struct ImageQueryNotifyData {
  ImageMsg rgb_image;
  ImageMsg depth_image;
  PoseStamped tf_goal;
  std::vector<double> camera_info_k;
};

/** pose_tools 投影请求视图：通常可由 ImageQueryNotifyData 填充，query_id 可留空。 */
struct ProjectPlaceLineAndPlaneView {
  ImageMsg rgb_image;
  ImageMsg depth_image;
  PoseStamped tf_goal;
  std::vector<double> camera_info_k;
  std::string query_id;
};

/** pose_tools 投影请求：多视角深度/位姿 + 承载盒 + 前沿。 */
struct ProjectPlaceLineAndPlaneRequest {
  std::vector<ProjectPlaceLineAndPlaneView> views;
  std::vector<std::vector<float>> carrier_box;
  std::vector<std::vector<float>> front_edge;
};

/** pose_tools 投影返回单行：像素折线/多边形。 */
struct ProjectPlaceLineAndPlaneRow {
  std::vector<std::vector<int>> place_line;
  std::vector<std::vector<int>> place_plane;
  double place_yaw{0.0};
  std::string image_id;
  std::string raw_json;
};

/** pose_tools 投影响应。 */
struct ProjectPlaceLineAndPlaneResponse {
  int code{0};
  std::string msg;
  std::vector<ProjectPlaceLineAndPlaneRow> data;
};

/** 物品在图像上的四角（像素）。 */
struct Location {
  float left_up_x{0};
  float left_up_y{0};
  float left_down_x{0};
  float left_down_y{0};
  float right_up_x{0};
  float right_up_y{0};
  float right_down_x{0};
  float right_down_y{0};
};

/** VLM 返回的单物品。 */
struct ItemData {
  std::string name;
  bool can_grab{false};
  std::string shoe_id;
  Location location;
};

/** VLM 位置请求，是ovd的高阶接口。 */
struct LocationRequest {
  std::vector<std::string> placeholder;
  int image_width{0};
  int image_height{0};
  std::string image; /**< base64 场景图 */
};

/** VLM 位置响应。 */
struct LocationResponse {
  int code{0};
  std::string msg;
  std::vector<ItemData> items;
  std::vector<ItemData> unknown_items;
};

/** OVD 服务端点：通用开放词汇检测或鞋子检测。 */
enum class OvdEndpoint {
  OVD,
  SHOE,
};

/** OVD 属性查询/结果。 */
struct OvdProperty {
  std::string color;
  std::string shape;
  std::string person;
};

/** 开放词汇检测结果条目。 */
struct ObjectDetection {
  int index{0};
  std::string uuid;
  std::string name;
  std::vector<int> bbox; /**< [x1,y1,x2,y2] */
  OvdProperty ovd_property;
};

/** 开放词汇检测请求：POST ``/ovd`` 或 ``/shoe``。 */
struct DetectObjectsRequest {
  std::string rgb_image; /**< PNG base64 或 ``data:image/png;base64,...`` */
  std::vector<std::string> labels;
  std::string position_region; // 语义区域
  std::string payload; // 承载物
  OvdProperty ovd_property;
  double ovd_obj_thresh{0.35}; // ovd 阈值
  double box_obj_thresh{0.35}; // box 阈值
  OvdEndpoint entry{OvdEndpoint::OVD};
};

/** 开放词汇检测响应。 */
struct DetectObjectsResponse {
  int code{0};
  std::string msg;
  std::string rgb_image;
  std::vector<std::string> labels;
  std::vector<ObjectDetection> items;
  std::vector<ObjectDetection> items_unknown;
};

/** PutWhere 请求。 */
struct PutWhereRequest {
  std::string carrier; // 承载物名称
  std::string carrier_direct; // 承载物方向
  std::string image;
  int image_height{400}; // 默认400
  int image_width{640}; // 默认640
  std::string placeholder; // 推荐摆放物品名称
  std::string summary; // 摆放的文案描述
};

/** PutWhere 响应。 */
struct PutWhereResponse {
  int code{0};
  std::string msg;
  std::string final_answer;
};

/** 鞋子摆放意图：单只鞋 bbox + 成双 id。 */
struct ShoesIntentShoeItem {
  std::vector<int> bbox; /**< [xmin, ymin, xmax, ymax] */
  int pair_id{0};
  bool is_tilted{false}; /**< 鞋子是否摆歪；仅 check 模式使用。 */
};

/** 鞋子摆放意图规划请求。 */
struct ShoesIntentPlanRequest {
  std::vector<ShoesIntentShoeItem> shoes;
  std::vector<std::vector<int>> place_line;  /**< 墙线点列，每点 [x,y] */
  std::vector<std::vector<int>> place_plane; /**< 临时摆放区点列 */
};

/** 单步整理动作。 */
struct ShoesIntentStep {
  int step_id{0};
  std::vector<int> from_bbox;
  std::vector<int> to_bbox;
  int pair_id{0};
  int shoe_id{0};
};

/** 最终每只鞋状态。 */
struct ShoesIntentFinalBbox {
  int shoe_id{0};
  int pair_id{0};
  std::string status;
  bool is_single{false};
  std::vector<int> final_bbox;
};

/** 鞋子摆放意图规划结果。 */
struct ShoesIntentPlanResponse {
  int code{0};
  std::string msg;
  std::vector<ShoesIntentStep> steps;
  std::vector<ShoesIntentFinalBbox> final_bboxes;
};

/** 桌面预感知输入图：图片与拍摄时头部摄像机角度。 */
struct DeskIntentPrePerceptionImage {
  std::string image_base64; /**< PNG base64 或 ``data:image/png;base64,...`` */
  double angle{0.0};
};

/** 桌面观测角度推荐请求：POST ``/v038/test/desk_intent/pre_perception/v1``。 */
struct DeskIntentPrePerceptionRequest {
  std::vector<DeskIntentPrePerceptionImage> images;
};

/** 桌面观测角度推荐结果。 */
struct DeskIntentPrePerceptionResponse {
  int code{0};
  std::string msg;
  double angle{0.0};
};

/** 桌面感知请求：POST ``/v038/test/desk_intent/perception/v1``。 */
struct DeskIntentPerceptionRequest {
  std::string image; /**< PNG base64 或 ``data:image/png;base64,...`` */
};

/** 桌面感知物品：名称与像素框 ``[x1, y1, x2, y2]``。 */
struct DeskIntentPerceptionObject {
  std::string name;
  std::vector<int> bbox;
};

/** 桌面感知结果。 */
struct DeskIntentPerceptionResponse {
  int code{0};
  std::string msg;
  std::vector<DeskIntentPerceptionObject> objects;
};

/** 桌面/鞋子摆放规划单步：源框与目标框为像素坐标 ``[x1, y1, x2, y2]``。 */
struct MovePlanStep {
  int step_id{0};
  std::vector<int> from_bbox; /**< 4 元 [x1,y1,x2,y2] */
  std::vector<int> to_bbox;
};

/** 桌面物品摆放意图请求：POST ``/v038/test/desk_intent/plan/v2``（与 shoe_plan 同机 ``8555``）。 */
struct DeskIntentPlanRequest {
  std::string image; // base64
  std::string user_input; // 用户的需求, "帮我将玩偶放到日历前面"
  std::vector<DeskIntentPerceptionObject> perception;
  std::optional<std::string> judge_input; // 桌面整理评估结果
  std::optional<std::string> memory_input; // 记忆输入，可以根据用户的历史偏好等优化意图生成
};

/** 桌面物品摆放规划单步：源框与目标框为像素坐标 ``[x1, y1, x2, y2]``，并包含语义说明。 */
struct DeskIntentPlanStep {
  int step_id{0};
  std::vector<int> from_bbox; /**< 4 元 [x1,y1,x2,y2] */
  std::vector<int> to_bbox;
  bool is_container{false}; // 是否是容器放置
  std::string reason;
  std::string placement_type; // 'desktop'
  std::optional<std::string> container_name; // 容器名字
};

/** 桌面意图规划结果。 */
struct DeskIntentPlanResponse {
  int code{0};
  std::string msg;
  std::string user_input;
  std::string plan; // 整体规划
  std::vector<DeskIntentPlanStep> steps;
};

/** 桌面整理结果判定请求：POST ``/v038/test/judge/action/v2``。 */
struct DeskIntentJudgeActionRequest {
  std::string tidy_image; /**< 整理后的桌面图片，PNG base64 或 ``data:image/png;base64,...`` */
  std::string user_input; /**< 用户原始整理需求 */
};

/** 桌面整理结果判定结果。 */
struct DeskIntentJudgeActionResponse {
  int code{0};
  std::string msg;
  bool is_tidy{false};
  std::string reason;
  double score{0.0};
};

/** 桌面意图 bbox 匹配请求：POST ``/v038/test/desk_intent/match``。 */
struct DeskIntentMatchRequest {
  std::string arm_image;
  std::string base_image;
  std::vector<int> bbox; /**< 头部相机图中待匹配框 ``[x1,y1,x2,y2]``，须 4 元 */
};

/** 桌面意图匹配结果。 */
struct DeskIntentMatchResponse {
  int code{0};
  std::string msg;
  std::vector<int> bbox; /**< 手部相机图中对应框，4 元 */
};

/** 图像特征提取请求（/image_embedding_extern）。 */
struct ImageEmbeddingRequest {
  std::vector<std::string> image_list;
};

/** 图像特征提取结果。 */
struct ImageEmbeddingResponse {
  int code{0};
  std::string msg;
  std::vector<std::vector<float>> embeddings;
  bool IsSuccess() const { return code == 0; }
  bool IsError() const { return code != 0; }
};

using ImageEmbeddingResult = ImageEmbeddingResponse;

/** 混合相似度请求：同侧 image 与 embedding 二选一（/fused_similarity_hybrid_extern）。 */
struct HybridFusedSimRequest {
  std::optional<std::string> image_a;
  std::optional<std::vector<float>> embedding_a;
  std::optional<std::string> image_b;
  std::optional<std::vector<float>> embedding_b;
};

/** 混合相似度结果。 */
struct FusedSimilarityResponse {
  int code{0};
  std::string msg;
  double similarity{0.0};
  bool IsSuccess() const { return code == 0; }
  bool IsError() const { return code != 0; }
};

using FusedSimilarityResult = FusedSimilarityResponse;

/** pairwise：单图上的检测框项。 */
struct PairwiseItemBBox {
  std::vector<int> bbox; /**< [xmin, ymin, xmax, ymax] */
  std::string id;
  std::string item;
  std::string color;
  std::string shape;
  std::string person;
};

/** pairwise：单张大图及其物体列表。 */
struct PairwiseSceneImage {
  std::string image;
  std::vector<PairwiseItemBBox> items;
};

/** pairwise 请求。 */
struct PairwiseRequest {
  std::optional<std::string> user_id;
  std::vector<PairwiseSceneImage> images;
};

/** pairwise 返回的单个物体配对信息。 */
struct PairwiseMatchedItem {
  std::string id;
  std::string item;
  std::string color;
  std::string shape;
  std::string person;
  std::string pair_id;
};

/** pairwise 结果。 */
struct PairwiseResponse {
  int code{0};
  std::string msg;
  std::vector<PairwiseMatchedItem> data;
  bool IsSuccess() const { return code == 0; }
  bool IsError() const { return code != 0; }
};

using PairwiseResult = PairwiseResponse;

}  // namespace robot_sdk

#endif
