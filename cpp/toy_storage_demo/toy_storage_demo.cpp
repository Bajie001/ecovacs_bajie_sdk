/**
 * 玩具收纳场景演示
 *
 * 实现方式：仅包含对外头文件 <robot_sdk/...>，通过链接共享库 librobot_sdk.so（CMake 目标 robot_sdk）
 * 使用 robot_sdk::Client、ros_skill、OVD / PutWhere 等公开 API，不依赖 SDK 内部源码或 json.h。
 */
#include <robot_sdk/robot_sdk.h>
#include "../common/sdk_response_helpers.h"
#include <cstdlib>
#include <iostream>
#include <optional>
#include <regex>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
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

std::optional<std::vector<float>> CarrierBboxFromMetaDataStr(
    const std::string& meta_data) {
  const std::string::size_type first_non_space = meta_data.find_first_not_of(" \t\r\n");
  if (first_non_space == std::string::npos) {
    return std::nullopt;
  }

  std::vector<float> carrier_bbox = ExtractJsonNumberArray(meta_data, "carrier_bbox", 4);
  if (!carrier_bbox.empty()) {
    return carrier_bbox;
  }

  return std::nullopt;
}

void SpeakToyStorageResult(const std::string& text) {
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

bool ObserveAndGrab(int attempt, const robot_sdk::BoxData& toy_pose,
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
    std::cerr << "第" << attempt << "次尝试手臂归位失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }
  std::cout << "步骤3: 手部观测玩具..." << std::endl;

  error_code = robot_sdk::eco_LookTo(robot_sdk::LookToTarget::HAND, toy_pose);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << attempt << "次尝试手部观测失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "步骤4: 手部拍照..." << std::endl;

  robot_sdk::ImageQueryData image_query_data;
  image_query_data.camera_type = robot_sdk::CameraType::ARM;

  robot_sdk::ImageQueryNotifyData image_result;
  error_code = robot_sdk::eco_ImageQuery(image_query_data, image_result);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << attempt << "次尝试手部拍照失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::cout << "手部拍照成功: RGB " << demo_sdk::FormatImagePayloadSize(image_result.rgb_image)
            << ", 深度图像 " << demo_sdk::FormatImagePayloadSize(image_result.depth_image)
            << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "步骤5: 调用OVD检测玩具..." << std::endl;

  robot_sdk::DetectObjectsRequest detect_req;
  detect_req.labels = {"玩具"};
  detect_req.rgb_image = image_result.rgb_image.data;

  std::vector<robot_sdk::ObjectDetection> items;
  std::string error_msg;
  if (!robot_sdk::eco_DetectObjectsSimple(detect_req, items, error_msg)) {
    UpdateLastError(last_error, MakeResultCode(1, error_msg));
    std::cerr << "第" << attempt << "次尝试OVD检测失败: " << error_msg << std::endl;
    return false;
  }

  if (items.empty()) {
    UpdateLastError(last_error, MakeResultCode(1, "OVD未找到玩具"));
    std::cerr << "第" << attempt << "次尝试未找到玩具" << std::endl;
    return false;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  const robot_sdk::ObjectDetection& target_item = items.front();
  if (target_item.bbox.size() < 4) {
    UpdateLastError(last_error, MakeResultCode(1, "OVD返回无效检测框"));
    std::cerr << "第" << attempt << "次尝试OVD返回无效检测框" << std::endl;
    return false;
  }

  std::cout << "OVD检测成功: 找到玩具'" << target_item.name << "', bbox=("
            << target_item.bbox[0] << ", " << target_item.bbox[1] << ", "
            << target_item.bbox[2] << ", " << target_item.bbox[3]
            << ")" << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "步骤6: 准确抓取玩具..." << std::endl;

  robot_sdk::BoundingBox bbox;
  bbox.name = target_item.name;
  bbox.left_up_x = static_cast<float>(target_item.bbox[0]);
  bbox.left_up_y = static_cast<float>(target_item.bbox[1]);
  bbox.right_down_x = static_cast<float>(target_item.bbox[2]);
  bbox.right_down_y = static_cast<float>(target_item.bbox[3]);

  robot_sdk::AccurateGrabData accurate_grab_data;
  accurate_grab_data.rgb_image = image_result.rgb_image;
  accurate_grab_data.depth_image = image_result.depth_image;
  accurate_grab_data.tf_goal = image_result.tf_goal;
  accurate_grab_data.bbox = bbox;

  error_code = robot_sdk::eco_AccurateGrab(accurate_grab_data);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << attempt << "次尝试准确抓取失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::cout << "抓取成功!" << std::endl;
  return true;
}

bool ObserveAndPut(int retry, const robot_sdk::BoxData& basket_pose,
                   robot_sdk::ErrorCode* last_error) {
  std::cout << "步骤7: 头部观测收纳筐..." << std::endl;

  robot_sdk::ErrorCode error_code =
      robot_sdk::eco_LookTo(robot_sdk::LookToTarget::HEAD, basket_pose);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << retry << "次尝试头部观测收纳筐失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "步骤8: 头部拍照..." << std::endl;

  robot_sdk::ImageQueryData head_image_query_data;
  head_image_query_data.camera_type = robot_sdk::CameraType::HEAD;

  robot_sdk::ImageQueryNotifyData head_image_result;
  error_code = robot_sdk::eco_ImageQuery(head_image_query_data, head_image_result);
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

  std::cout << "步骤9: 调用putwhere接口确定放置位置..." << std::endl;

  robot_sdk::PutWhereRequest put_where_req;
  put_where_req.image = head_image_result.rgb_image.data;
  put_where_req.placeholder = "玩具";
  put_where_req.summary = "玩具收纳";

  std::string final_answer;
  std::string put_where_error;

  bool put_where_success =
      demo_sdk::PutWhereWithError(put_where_req, final_answer, put_where_error);

  if (!put_where_success) {
    UpdateLastError(last_error, MakeResultCode(1, put_where_error));
    std::cerr << "第" << retry << "次尝试putwhere调用失败: " << put_where_error << std::endl;
    return false;
  }

  std::cout << "putwhere成功: 区域=" << final_answer << std::endl;

  const std::optional<std::vector<float>> place_ref =
      CarrierBboxFromMetaDataStr(final_answer);
  if (!place_ref || place_ref->size() < 4) {
    UpdateLastError(last_error, MakeResultCode(1, "无法从putwhere结果解析carrier_bbox"));
    std::cerr << "第" << retry << "次尝试无法从putwhere结果解析carrier_bbox: "
              << final_answer << std::endl;
    return false;
  }

  const std::vector<float>& carrier_bbox = *place_ref;
  std::cout << "putwhere放置框: bbox=[" << carrier_bbox[0] << ", " << carrier_bbox[1]
            << ", " << carrier_bbox[2] << ", " << carrier_bbox[3] << "]" << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "步骤10: 放入收纳筐..." << std::endl;

  error_code = robot_sdk::eco_PlaceIn(head_image_result, carrier_bbox);
  if (error_code.code != 0) {
    UpdateLastError(last_error, error_code);
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return false;
    }
    std::cerr << "第" << retry << "次尝试放入收纳筐失败: code=" << error_code.code
              << ", msg=" << error_code.msg << std::endl;
    return false;
  }

  std::cout << "第" << retry << "次放置流程成功完成！" << std::endl;
  return true;
}

/**
 * @brief 玩具收纳业务场景演示
 *
 * @param link_url 服务端连接地址
 * @return bool 成功返回true，失败返回false
 */
bool ToyStorageDemo(const std::string& link_url) {
  robot_sdk::Client& client = robot_sdk::Client::GetInstance();

  auto cleanup = [&]() {
    robot_sdk::eco_SetCancellationFlag(nullptr);
    client.Disconnect();
    robot_sdk::Client::CleanupInstance();
  };
  auto finish = [&](bool success, const std::string& summary,
                    const robot_sdk::ErrorCode& result_code) {
    SpeakToyStorageResult(FormatSpeechResult(summary, result_code));
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
      return finish(true, "玩具收纳演示已被中断", error_code);
    }
    std::cerr << "准备动作失败: code=" << error_code.code << ", msg=" << error_code.msg
              << std::endl;
    return finish(false, "玩具收纳演示失败，机器准备动作失败", error_code);
  }

  std::cout << "进行机器定位更新..." << std::endl;
  error_code = robot_sdk::eco_MapRelocation();
  if (error_code.code != 0) {
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return finish(true, "玩具收纳演示已被中断", error_code);
    }
    std::cerr << "定位失败: code=" << error_code.code << ", msg=" << error_code.msg
              << std::endl;
    return finish(false, "玩具收纳演示失败，机器定位更新失败", error_code);
  }

  std::cout << "步骤1: 搜索收纳筐..." << std::endl;
  robot_sdk::SearchData basket_search_data;
  basket_search_data.object.item = "收纳筐";
  //basket_search_data.area.area_id = "1";

  robot_sdk::BoxData basket_pose;
  error_code = robot_sdk::eco_Search(basket_search_data, basket_pose);
  if (error_code.code != 0) {
    if (error_code.code == 4 || g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      return finish(true, "玩具收纳演示已被中断", error_code);
    }
    std::cerr << "搜索收纳筐失败: code=" << error_code.code << ", msg=" << error_code.msg
              << std::endl;
    return finish(false, "玩具收纳演示失败，没有找到收纳筐", error_code);
  }

  std::cout << "找到收纳筐: 位置=(" << basket_pose.position.x << ", " << basket_pose.position.y
            << ", " << basket_pose.position.z << "), 坐标系=" << basket_pose.frame_id
            << std::endl;

  robot_sdk::BoxData basket_pose_6d = basket_pose;
  bool has_found_toy = false;

  for (int attempt = 1; !g_interrupted; ++attempt) {
    std::cout << "\n=== 尝试第 " << attempt << " 次收纳玩具 ===" << std::endl;

    if (g_interrupted) break;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << "步骤2: 搜索玩具..." << std::endl;

    robot_sdk::SearchData toy_search_data;
    toy_search_data.object.item = "玩具";
    toy_search_data.filter_boxes.push_back(basket_pose_6d);
    //toy_search_data.area.area_id = "1";

    robot_sdk::BoxData toy_pose;
    error_code = robot_sdk::eco_Search(toy_search_data, toy_pose);
    if (error_code.code != 0) {
      if (error_code.code == 4 || g_interrupted) {
        std::cout << "演示已被用户中断" << std::endl;
        return finish(true, "玩具收纳演示已被中断", error_code);
      }
      std::cerr << "第" << attempt << "次尝试搜索玩具失败: code=" << error_code.code
                << ", msg=" << error_code.msg << std::endl;
      std::cout << "任务结束，开始收臂" << std::endl;
      const robot_sdk::ErrorCode search_error = error_code;
      error_code = robot_sdk::eco_RobotEndingPose();
      if (error_code.code != 0) {
        std::cerr << "收臂失败: code=" << error_code.code << ", msg=" << error_code.msg
                  << std::endl;
      }
      if (has_found_toy) {
        std::cout << "已经找到并处理过玩具，本次未找到新玩具，任务成功结束" << std::endl;
        return finish(true, "玩具收纳演示完成，玩具已经收纳好",
                      MakeResultCode(0, "success"));
      }
      return finish(false, "玩具收纳演示失败，没有找到需要收纳的玩具", search_error);
    }

    has_found_toy = true;
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
      return finish(true, "玩具收纳演示已被中断", interrupted_error);
    }

    if (!grab_success) {
      std::cout << "连续三次抓取失败,任务结束" << std::endl;
      return finish(false, "玩具收纳演示失败，连续三次抓取玩具失败", grab_error);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool put_success = false;
    robot_sdk::ErrorCode put_error;
    for (int retry = 1; retry <= 3 && !g_interrupted; ++retry) {
      put_success = ObserveAndPut(retry, basket_pose_6d, &put_error);
      if (put_success) {
        break;
      }

      std::cout << "第" << retry << "次放置失败" << std::endl;
    }

    if (g_interrupted) {
      std::cout << "演示已被用户中断" << std::endl;
      const robot_sdk::ErrorCode interrupted_error =
          put_error.code == 0 ? MakeResultCode(4, "用户中断") : put_error;
      return finish(true, "玩具收纳演示已被中断", interrupted_error);
    }

    if (!put_success) {
      std::cout << "连续三次放置失败,任务结束" << std::endl;
      return finish(false, "玩具收纳演示失败，连续三次放置玩具失败", put_error);
    }

    std::cout << "第" << attempt << "次收纳成功完成！" << std::endl;
    std::cout << "准备开始下一次收纳..." << std::endl;

    for (int i = 0; i < 10 && !g_interrupted; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (g_interrupted) {
      std::cout << "接收到中断信号，停止循环" << std::endl;
      break;
    }
  }

  if (g_interrupted) {
    return finish(true, "玩具收纳演示已被中断", MakeResultCode(4, "用户中断"));
  }
  return finish(true, "玩具收纳演示完成，玩具已经收纳好", MakeResultCode(0, "success"));
}

int main(int argc, char* argv[]) {
  std::string link_url = robot_sdk::eco_GetDefaultLinkUrl();

  if (argc > 1) {
    std::string arg = argv[1];
    if (arg == "-h" || arg == "--help") {
      std::cout << "用法: " << argv[0] << " [连接地址]" << std::endl;
      std::cout << "默认地址: 由 ECO_ROBOT_HOST（默认 10.10.10.11）与固定端口 9900 决定，"
                   "见 robot_sdk::eco_GetDefaultLinkUrl()" << std::endl;
      std::cout << "本程序仅链接 librobot_sdk.so，通过 robot_sdk 公开头文件调用。" << std::endl;
      return 0;
    }
    link_url = arg;
  }

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  std::cout << "=== 玩具收纳C++演示程序（robot_sdk 共享库） ===" << std::endl;
  std::cout << "连接地址: " << link_url << std::endl;
  SetEcoRobotHostFromLinkUrl(link_url);
  std::cout << "按 Ctrl+C 退出程序" << std::endl;

  bool success = ToyStorageDemo(link_url);

  if (success) {
    std::cout << "玩具收纳演示程序完成!" << std::endl;
    return 0;
  }
  std::cerr << "玩具收纳演示程序失败!" << std::endl;
  return 1;
}
