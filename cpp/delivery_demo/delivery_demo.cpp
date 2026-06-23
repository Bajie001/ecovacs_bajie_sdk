/**
 * 玩具配送场景演示：将地面上的玩具抓取后放到桌子上
 *
 * 流程：原地寻玩具 → 手部观测 → 手部拍照 → OVD（开放词表）视觉定位 → 抓取
 *      → 语义导航至桌子 → 头部拍照 → PutWhere 推荐 → 视图放置
 *
 * 实现方式与 toy_storage_demo 一致：仅包含 <robot_sdk/robot_sdk.h>，链接 librobot_sdk.so。
 */
#include <robot_sdk/robot_sdk.h>
#include "../common/sdk_response_helpers.h"
#include <chrono>
#include <csignal>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <regex>
#include <thread>
#include <unistd.h>

namespace {

std::atomic<bool> g_interrupted{false};
volatile std::sig_atomic_t g_connecting = 0;

void signal_handler(int /*signal*/) {
  g_interrupted = true;
  static const char kMsg[] = "\n接收到中断信号，正在退出...\n";
  [[maybe_unused]] ssize_t n = write(STDERR_FILENO, kMsg, sizeof(kMsg) - 1);
  (void)n;
  if (g_connecting) {
    static const char kConnectingMsg[] = "连接尚未完成，强制退出程序。\n";
    [[maybe_unused]] ssize_t n2 =
        write(STDERR_FILENO, kConnectingMsg, sizeof(kConnectingMsg) - 1);
    (void)n2;
    _exit(130);
  }
}

std::string ExtractHostFromLinkUrl(const std::string& link_url) {
  std::string::size_type host_begin = 0;
  const std::string::size_type scheme_pos = link_url.find("://");
  if (scheme_pos != std::string::npos) {
    host_begin = scheme_pos + 3;
  }

  if (host_begin >= link_url.size()) {
    return {};
  }

  if (link_url[host_begin] == '[') {
    const std::string::size_type host_end = link_url.find(']', host_begin + 1);
    if (host_end == std::string::npos) {
      return {};
    }
    return link_url.substr(host_begin + 1, host_end - host_begin - 1);
  }

  const std::string::size_type host_end = link_url.find_first_of(":/?", host_begin);
  return link_url.substr(host_begin, host_end == std::string::npos ? std::string::npos
                                                                  : host_end - host_begin);
}

bool SetEcoRobotHostFromLinkUrl(const std::string& link_url) {
  const std::string host = ExtractHostFromLinkUrl(link_url);
  if (host.empty()) {
    std::cerr << "无法从连接地址中解析 IP，未设置 ECO_ROBOT_HOST: " << link_url << std::endl;
    return false;
  }

  if (setenv("ECO_ROBOT_HOST", host.c_str(), 1) != 0) {
    std::cerr << "设置 ECO_ROBOT_HOST 失败: " << host << std::endl;
    return false;
  }

  std::cout << "SDK默认主机环境变量 ECO_ROBOT_HOST=" << host << std::endl;
  return true;
}

std::vector<float> ExtractJsonNumberArray(const std::string& text,
                                          const std::string& key,
                                          std::size_t size) {
  const std::regex array_regex("\"" + key + "\"\\s*:\\s*\\[([^\\]]*)\\]");
  std::smatch array_match;
  if (!std::regex_search(text, array_match, array_regex)) {
    return {};
  }

  std::vector<float> values;
  const std::string body = array_match[1].str();
  const std::regex number_regex(R"([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?)");
  for (auto it = std::sregex_iterator(body.begin(), body.end(), number_regex);
       it != std::sregex_iterator(); ++it) {
    values.push_back(std::stof(it->str()));
  }

  if (values.size() != size) {
    return {};
  }
  return values;
}

std::optional<std::vector<float>> PlaceTargetFromMetaDataStr(const std::string& meta_data) {
  const std::string::size_type first_non_space = meta_data.find_first_not_of(" \t\r\n");
  if (first_non_space == std::string::npos) {
    return std::nullopt;
  }

  std::vector<float> carrier_bbox = ExtractJsonNumberArray(meta_data, "carrier_bbox", 4);
  if (!carrier_bbox.empty()) {
    return carrier_bbox;
  }

  std::vector<float> point_2ds = ExtractJsonNumberArray(meta_data, "point_2ds", 2);
  if (!point_2ds.empty()) {
    return point_2ds;
  }

  return std::nullopt;
}

void SpeakDeliveryResult(const std::string& text) {
  std::cout << "语音播报: " << text << std::endl;
  const robot_sdk::ErrorCode speech_error = robot_sdk::eco_Speech(text);
  if (speech_error.code != 0) {
    std::cerr << "语音播报失败: code=" << speech_error.code
              << ", msg=" << speech_error.msg << std::endl;
  }
}

std::string FormatSpeechResult(const std::string& summary,
                               const robot_sdk::ErrorCode& error_code) {
  const std::string msg = error_code.msg.empty() ? "无" : error_code.msg;
  return summary + "，错误码 " + std::to_string(error_code.code) + "，错误信息 " + msg;
}

robot_sdk::ErrorCode MakeResultCode(uint32_t code, const std::string& msg) {
  robot_sdk::ErrorCode error_code;
  error_code.code = code;
  error_code.msg = msg;
  return error_code;
}

void UpdateLastError(robot_sdk::ErrorCode* last_error,
                     const robot_sdk::ErrorCode& error_code) {
  if (last_error != nullptr) {
    *last_error = error_code;
  }
}

}  // namespace

bool ObserveAndGrab(int retry, const robot_sdk::BoxData& toy_pose,
                    robot_sdk::ErrorCode* last_error) {
  std::cout << "观测前手臂归位..." << std::endl;
  robot_sdk::RobotArmCtrlData arm_ctrl_data;
  arm_ctrl_data.mode = 2;
  robot_sdk::ErrorCode error_code = robot_sdk::eco_RobotArmCtrl(arm_ctrl_data);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << retry << "次尝试手臂归位失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::cout << "步骤2: 手部观测玩具..." << std::endl;
  error_code = robot_sdk::eco_LookTo(robot_sdk::LookToTarget::HAND, toy_pose);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << retry << "次尝试手部观测失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "步骤3: 手部拍照..." << std::endl;
  robot_sdk::ImageQueryData hand_image_query;
  hand_image_query.camera_type = robot_sdk::CameraType::ARM;

  robot_sdk::ImageQueryNotifyData hand_image_result;
  error_code = robot_sdk::eco_ImageQuery(hand_image_query, hand_image_result);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << retry << "次尝试手部拍照失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::cout << "手部拍照成功: RGB " << demo_sdk::FormatImagePayloadSize(hand_image_result.rgb_image)
            << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "步骤4: OVD 识别（开放词表视觉定位）..." << std::endl;

  robot_sdk::DetectObjectsRequest detect_req;
  detect_req.labels = {"玩具"};
  detect_req.rgb_image = hand_image_result.rgb_image.data;

  std::vector<robot_sdk::ObjectDetection> items;
  std::string location_err;
  if (!robot_sdk::eco_DetectObjectsSimple(detect_req, items, location_err)) {
    UpdateLastError(last_error, MakeResultCode(1, location_err));
    std::cerr << "第" << retry << "次尝试OVD/视觉定位失败: " << location_err << std::endl;
    return false;
  }

  if (items.empty()) {
    UpdateLastError(last_error, MakeResultCode(1, "OVD未检测到可操作的玩具目标"));
    std::cerr << "第" << retry << "次尝试OVD未检测到可操作的玩具目标" << std::endl;
    return false;
  }

  const robot_sdk::ObjectDetection& target_item = items.front();
  if (target_item.bbox.size() < 4) {
    UpdateLastError(last_error, MakeResultCode(1, "OVD返回无效检测框"));
    std::cerr << "第" << retry << "次尝试OVD返回无效检测框" << std::endl;
    return false;
  }

  std::cout << "OVD 识别成功: '" << target_item.name << "'" << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "步骤5: 抓取玩具..." << std::endl;
  robot_sdk::BoundingBox bbox;
  bbox.name = target_item.name;
  bbox.left_up_x = static_cast<float>(target_item.bbox[0]);
  bbox.left_up_y = static_cast<float>(target_item.bbox[1]);
  bbox.right_down_x = static_cast<float>(target_item.bbox[2]);
  bbox.right_down_y = static_cast<float>(target_item.bbox[3]);

  robot_sdk::AccurateGrabData grab_data;
  grab_data.rgb_image = hand_image_result.rgb_image;
  grab_data.depth_image = hand_image_result.depth_image;
  grab_data.tf_goal = hand_image_result.tf_goal;
  grab_data.bbox = bbox;

  error_code = robot_sdk::eco_AccurateGrab(grab_data);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << retry << "次尝试抓取失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::cout << "抓取成功" << std::endl;
  return true;
}

bool ObserveAndPut(int retry, robot_sdk::ErrorCode* last_error) {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "步骤7: 头部拍照..." << std::endl;
  robot_sdk::ImageQueryData head_image_query;
  head_image_query.camera_type = robot_sdk::CameraType::HEAD;

  robot_sdk::ImageQueryNotifyData head_image_result;
  robot_sdk::ErrorCode error_code =
      robot_sdk::eco_ImageQuery(head_image_query, head_image_result);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << retry << "次尝试头部拍照失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::cout << "头部拍照成功: RGB " << demo_sdk::FormatImagePayloadSize(head_image_result.rgb_image)
            << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "步骤8: PutWhere 推荐放置区域..." << std::endl;
  robot_sdk::PutWhereRequest put_where_req;
  put_where_req.image = head_image_result.rgb_image.data;
  put_where_req.placeholder = "玩具";
  put_where_req.summary = "将玩具放到桌面上";

  std::string final_answer;
  std::string put_where_error;
  if (!demo_sdk::PutWhereWithError(put_where_req, final_answer, put_where_error)) {
    UpdateLastError(last_error, MakeResultCode(1, put_where_error));
    std::cerr << "第" << retry << "次尝试PutWhere失败: " << put_where_error << std::endl;
    return false;
  }
  std::cout << "PutWhere 成功: final_answer=" << final_answer << std::endl;

  const std::optional<std::vector<float>> place_ref =
      PlaceTargetFromMetaDataStr(final_answer);
  if (!place_ref) {
    UpdateLastError(last_error, MakeResultCode(1, "无法从PutWhere结果解析放置目标"));
    std::cerr << "第" << retry << "次尝试无法从PutWhere结果解析放置目标: "
              << final_answer << std::endl;
    return false;
  }

  const std::vector<float>& place_target = *place_ref;
  std::cout << "PutWhere放置目标: [";
  for (std::size_t i = 0; i < place_target.size(); ++i) {
    if (i > 0) std::cout << ", ";
    std::cout << place_target[i];
  }
  std::cout << "]" << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "步骤9: eco_PlaceWithView..." << std::endl;

  error_code = robot_sdk::eco_PlaceWithView(head_image_result, place_target);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << retry << "次尝试视图放置失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::cout << "第" << retry << "次放置流程成功完成" << std::endl;
  return true;
}

/**
 * @param link_url 机器人技能服务连接地址
 * @param table_area_name 桌子语义区域名
 */
bool DeliveryDemo(const std::string& link_url, const std::string& table_area_name) {
  robot_sdk::Client& client = robot_sdk::Client::GetInstance();

  auto cleanup = [&]() {
    robot_sdk::eco_SetCancellationFlag(nullptr);
    client.Disconnect();
    robot_sdk::Client::CleanupInstance();
  };
  auto finish = [&](bool success, const std::string& summary,
                    const robot_sdk::ErrorCode& result_code) {
    SpeakDeliveryResult(FormatSpeechResult(summary, result_code));
    cleanup();
    return success;
  };

  // 须在 Connect 之前注册：握手阻塞期间库内才能响应 Ctrl+C 取消。
  robot_sdk::eco_SetCancellationFlag(&g_interrupted);

  g_connecting = 1;
  bool connected = client.Connect(link_url);
  g_connecting = 0;
  if (!connected) {
    if (g_interrupted.load()) {
      std::cerr << "连接已取消或被中断" << std::endl;
    } else {
      std::cerr << "连接服务端失败" << std::endl;
    }
    cleanup();
    return false;
  }

  std::cout << "已连接到服务端" << std::endl;

  std::cout << "机器准备动作..." << std::endl;

  robot_sdk::ErrorCode error_code = robot_sdk::eco_RobotPreparePose();
  if (error_code.code != 0) {
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return finish(true, "玩具配送演示已被中断", error_code);
    }
    std::cerr << "准备动作失败: code=" << error_code.code << ", msg=" << error_code.msg
              << std::endl;
    return finish(false, "玩具配送演示失败，机器准备动作失败", error_code);
  }

  std::cout << "进行机器定位更新..." << std::endl;
  error_code = robot_sdk::eco_MapRelocation();
  if (error_code.code != 0) {
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return finish(true, "玩具配送演示已被中断", error_code);
    }
    std::cerr << "定位失败: code=" << error_code.code << ", msg=" << error_code.msg
              << std::endl;
    return finish(false, "玩具配送演示失败，机器定位更新失败", error_code);
  }

  std::cout << "步骤1: 原地寻找玩具..." << std::endl;
  robot_sdk::SearchData toy_search_data;
  toy_search_data.object.item = "玩具";
  // 不设置 filter_boxes / area：在当前位置附近地面搜寻

  robot_sdk::BoxData toy_pose;
  error_code = robot_sdk::eco_Search(toy_search_data, toy_pose);
  if (error_code.code != 0) {
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return finish(true, "玩具配送演示已被中断", error_code);
    }
    std::cerr << "搜索玩具失败: code=" << error_code.code << ", msg=" << error_code.msg << std::endl;
    return finish(false, "玩具配送演示失败，没有找到需要配送的玩具", error_code);
  }

  std::cout << "找到玩具: 位置=(" << toy_pose.position.x << ", " << toy_pose.position.y << ", "
            << toy_pose.position.z << "), 坐标系=" << toy_pose.frame_id << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  bool grab_success = false;
  robot_sdk::ErrorCode grab_error;
  for (int retry = 1; retry <= 3 && !g_interrupted; ++retry) {
    grab_success = ObserveAndGrab(retry, toy_pose, &grab_error);
    if (grab_success) {
      break;
    }

    std::cout << "第" << retry << "次抓取失败" << std::endl;
  }

  if (g_interrupted) {
    std::cout << "演示已被用户中断" << std::endl;
    const robot_sdk::ErrorCode interrupted_error =
        grab_error.code == 0 ? MakeResultCode(4, "用户中断") : grab_error;
    return finish(true, "玩具配送演示已被中断", interrupted_error);
  }

  if (!grab_success) {
    std::cout << "连续三次抓取失败,任务结束" << std::endl;
    return finish(false, "玩具配送演示失败，连续三次抓取玩具失败", grab_error);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "步骤6: 语义导航到桌子（区域名: " << table_area_name << "）..." << std::endl;
  robot_sdk::SemanticNavigationData nav_data;
  nav_data.area_name = table_area_name;

  error_code = robot_sdk::eco_SemanticNavigation(nav_data);
  if (error_code.code != 0) {
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return finish(true, "玩具配送演示已被中断", error_code);
    }
    std::cerr << "语义导航失败: code=" << error_code.code << ", msg=" << error_code.msg << std::endl;
    return finish(false, "玩具配送演示失败，语义导航到桌子失败", error_code);
  }
  std::cout << "已到达桌子区域，开始升高机身" << std::endl;

  robot_sdk::RobotHeightCtrlData height_data;
  height_data.value = 0.44f;
  error_code = robot_sdk::eco_RobotHeightCtrl(height_data);
  if (error_code.code != 0) {
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return finish(true, "玩具配送演示已被中断", error_code);
    }
    std::cerr << "升高机身失败: code=" << error_code.code << ", msg=" << error_code.msg
              << std::endl;
    return finish(false, "玩具配送演示失败，升高机身失败", error_code);
  }

  bool put_success = false;
  robot_sdk::ErrorCode put_error;
  for (int retry = 1; retry <= 3 && !g_interrupted; ++retry) {
    put_success = ObserveAndPut(retry, &put_error);
    if (put_success) {
      break;
    }

    if (retry == 3) {
      break;
    }

    std::cout << "第" << retry << "次放置失败，重新导航至桌子" << std::endl;
    error_code = robot_sdk::eco_SemanticNavigation(nav_data);
    if (error_code.code != 0) {
      if (error_code.code == 4 || g_interrupted) {
        std::cout << "演示已被用户中断" << std::endl;
        return finish(true, "玩具配送演示已被中断", error_code);
      }
      std::cerr << "语义导航失败: code=" << error_code.code << ", msg=" << error_code.msg << std::endl;
      return finish(false, "玩具配送演示失败，语义导航到桌子失败", error_code);
    }
  }

  if (g_interrupted) {
    std::cout << "演示已被用户中断" << std::endl;
    const robot_sdk::ErrorCode interrupted_error =
        put_error.code == 0 ? MakeResultCode(4, "用户中断") : put_error;
    return finish(true, "玩具配送演示已被中断", interrupted_error);
  }

  if (!put_success) {
    std::cout << "连续三次放置失败,任务结束" << std::endl;
    return finish(false, "玩具配送演示失败，连续三次放置玩具失败", put_error);
  }

  std::cout << "玩具已放置到桌面，配送演示完成，收臂后结束任务" << std::endl;

  error_code = robot_sdk::eco_RobotEndingPose();
  if (error_code.code != 0) {
    std::cerr << "收臂失败: code=" << error_code.code << ", msg=" << error_code.msg
              << std::endl;
  }

  return finish(true, "玩具配送演示完成，玩具已经配送到桌面",
                MakeResultCode(0, "success"));
}

int main(int argc, char* argv[]) {
  std::string link_url = robot_sdk::eco_GetDefaultLinkUrl();
  std::string table_area_name = "桌子";
  if (argc > 1) {
    std::string arg = argv[1];
    if (arg == "-h" || arg == "--help") {
      std::cout << "用法: " << argv[0] << " [连接地址] [桌子区域名]" << std::endl;
      std::cout << "  连接地址默认: robot_sdk::eco_GetDefaultLinkUrl()（ECO_ROBOT_HOST + :9900）" << std::endl;
      std::cout << "  桌子区域名默认: 桌子（须与语义地图中区域名一致）" << std::endl;
      return 0;
    }
    link_url = arg;
  }
  if (argc > 2) {
    table_area_name = argv[2];
  }

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  std::cout << "=== 玩具配送演示（地面 → 桌面，robot_sdk） ===" << std::endl;
  std::cout << "连接地址: " << link_url << std::endl;
  std::cout << "桌子区域名: " << table_area_name << std::endl;
  SetEcoRobotHostFromLinkUrl(link_url);
  std::cout << "按 Ctrl+C 可中断" << std::endl;

  if (DeliveryDemo(link_url, table_area_name)) {
    std::cout << "程序正常结束" << std::endl;
    return 0;
  }
  std::cerr << "程序失败" << std::endl;
  return 1;
}
