/**
 * 鞋子整理场景演示（匹配 Python `shoe_sorting_demo.py` 工作流与算法）
 *
 * 工作流：
 *   connect → cancelAllMissions → preparePose
 *   → resolve organize area
 *   → ×3: navigate → capture → detect → plan → execute
 *   → cleanup
 */
#include <robot_sdk/robot_sdk.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unistd.h>
#include <vector>

#include "execution.h"
#include "pairing.h"
#include "planning.h"

namespace {

std::atomic<bool> g_interrupted{false};

void signal_handler(int) { g_interrupted = true; }

static void ThrowDemoOnError(const robot_sdk::ErrorCode& ec, const std::string& desc) {
  if (ec.code == 0) return;
  if (ec.code == 4 || g_interrupted)
    throw std::runtime_error("演示已被用户中断（" + desc + "）");
  throw std::runtime_error(desc + " 失败: code=" + std::to_string(ec.code) + ", msg=" + ec.msg);
}

static void Say(const std::string& text, bool no_voice = false) {
  std::cout << "[voice] " << text << std::endl;
  if (no_voice) return;
  robot_sdk::eco_Speech(text);
}

static std::vector<uint8_t> Base64Decode(const std::string& input) {
  static constexpr char kChars[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::vector<uint8_t> result;
  result.reserve(input.size() * 3 / 4);
  int val = 0, bits = 0;
  for (char c : input) {
    if (c == '=') break;
    const char* p = strchr(kChars, c);
    if (!p) continue;
    val = (val << 6) | static_cast<int>(p - kChars);
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      result.push_back(static_cast<uint8_t>(val >> bits));
      val &= (1 << bits) - 1;
    }
  }
  return result;
}

std::string ExtractHostFromLinkUrl(const std::string& link_url) {
  auto host_begin = link_url.find("://");
  host_begin = (host_begin == std::string::npos) ? 0 : host_begin + 3;
  if (host_begin >= link_url.size()) return {};
  if (link_url[host_begin] == '[')
    return link_url.substr(host_begin + 1, link_url.find(']', host_begin + 1) - host_begin - 1);
  auto host_end = link_url.find_first_of(":/?", host_begin);
  return link_url.substr(host_begin, host_end == std::string::npos ? std::string::npos : host_end - host_begin);
}

bool SetAndroidHostFromLinkUrl(const std::string& link_url) {
  auto host = ExtractHostFromLinkUrl(link_url);
  if (host.empty()) {
    std::cerr << "无法从连接地址中解析 IP，未设置 ECO_ROBOT_HOST: " << link_url << std::endl;
    return false;
  }
  if (setenv("ECO_ROBOT_HOST", host.c_str(), 1) != 0) {
    std::cerr << "设置 ECO_ROBOT_HOST 失败: " << host << std::endl;
    return false;
  }
  return true;
}

// ---- 默认整理区域（机器人正前方，相对坐标）----
const std::vector<std::vector<double>> kDefaultCarrierBox = {
    {1.2, 0.6, 0.0}, {1.2, -0.6, 0.0}, {0.6, -0.6, 0.0}, {0.6, 0.6, 0.0},
};
const std::vector<std::vector<double>> kDefaultFrontEdge = {
    {0.6, -0.6, 0.0}, {0.6, 0.6, 0.0},
};

/// 语义地图 "x,y,z;x,y,z;..."（cm）→ vector（m）。
static std::vector<std::vector<double>> ParseContentString(const std::string& str) {
  std::vector<std::vector<double>> result;
  std::stringstream ss(str);
  std::string point_str;
  while (std::getline(ss, point_str, ';')) {
    if (point_str.empty()) continue;
    std::stringstream ps(point_str);
    std::vector<double> pt;
    std::string coord;
    while (std::getline(ps, coord, ',')) pt.push_back(std::stod(coord) / 100.0);
    if (pt.size() >= 2) {
      if (pt.size() == 2) pt.push_back(0.0);
      result.push_back(std::move(pt));
    }
  }
  return result;
}

/// 机器人相对坐标 → 地图坐标（对应 Python _relative_to_map）。
static std::vector<std::vector<double>> RelativeToMap(
    const std::vector<std::vector<double>>& coords,
    const robot_sdk::RobotPos& robot_pos) {
  double cx = robot_pos.x, cy = robot_pos.y;
  double cos_yaw = std::cos(robot_pos.yaw), sin_yaw = std::sin(robot_pos.yaw);
  std::vector<std::vector<double>> result;
  for (const auto& p : coords)
    result.push_back({cx + cos_yaw * p[0] - sin_yaw * p[1],
                      cy + sin_yaw * p[0] + cos_yaw * p[1],
                      p.size() > 2 ? p[2] : 0.0});
  return result;
}

/// 编码点列为 "x,y;x,y;..." cm 字符串。
static std::string EncodePointsCm(const std::vector<std::vector<double>>& points) {
  std::string out;
  for (const auto& p : points) {
    if (!out.empty()) out += ";";
    out += std::to_string(static_cast<int>(std::round(p[0] * 100))) + "," +
           std::to_string(static_cast<int>(std::round(p[1] * 100)));
  }
  return out;
}

/**
 * 确定整理区域（对应 Python _resolve_organize_area）。
 * @return (temp_area_id, map_carrier, map_front)
 */
static std::tuple<std::string, std::vector<std::vector<double>>, std::vector<std::vector<double>>>
ResolveOrganizeArea(
    const std::vector<std::vector<double>>& carrier_box,
    const std::vector<std::vector<double>>& front_edge,
    const std::string& area_name,
    bool no_voice = false) {
  if (!area_name.empty()) {
    robot_sdk::SemanticMapManagerData query_data;
    query_data.cmd = 1;  // QUERY
    query_data.object_info.name = area_name;
    robot_sdk::SemanticMapManagerNotifyData notify;
    ThrowDemoOnError(robot_sdk::eco_SemanticMapManager(query_data, &notify, 30*1000), "查询语义区域");
    for (const auto& obj : notify.objects_info) {
      if (obj.name == area_name || obj.id == area_name) {
        if (obj.content.empty() || obj.direction.empty()){
          Say("语义区域" + area_name + "缺少 content/direction", no_voice);
          throw std::runtime_error("语义区域 " + area_name + " 缺少 content/direction");
        }
        Say("已找到语义区域" + area_name, no_voice);
        return {"", ParseContentString(obj.content), ParseContentString(obj.direction)};
      }
    }
    Say("语义区域" + area_name + "没找到", no_voice);
    throw std::runtime_error("语义区域 " + area_name + " 未找到");
  }

  const auto& rel_carrier = carrier_box.empty() ? kDefaultCarrierBox : carrier_box;
  const auto& rel_front  = front_edge.empty()  ? kDefaultFrontEdge   : front_edge;

  Say("未指定整理区域，将使用机器正前方默认区域，请确保前方无障碍物", no_voice);

  ThrowDemoOnError(robot_sdk::eco_RefreshRobotInfo({"pos"}), "刷新机器人位置缓存");
  robot_sdk::RobotPos robot_pos;
  ThrowDemoOnError(robot_sdk::eco_RobotPosition(robot_pos), "获取机器人当前位置");

  auto map_carrier = RelativeToMap(rel_carrier, robot_pos);
  auto map_front   = RelativeToMap(rel_front,  robot_pos);

  // 创建临时语义区域
  std::stringstream ss;
  ss << "temp_" << std::hex << std::mt19937(std::random_device{}())();
  std::string temp_area_id = ss.str();

  robot_sdk::SemanticMapManagerData add_data;
  add_data.cmd = 0;  // ADD
  add_data.object_info.id = temp_area_id;
  add_data.object_info.name = temp_area_id;
  add_data.object_info.model_name = "custom_organize_zone";
  add_data.object_info.model_level = 5;  // CUSTOM_ORGANIZE_ZONE
  add_data.object_info.content = EncodePointsCm(map_carrier);
  add_data.object_info.direction = EncodePointsCm(map_front);
  ThrowDemoOnError(robot_sdk::eco_SemanticMapManager(add_data, nullptr, 30*1000), "创建临时语义区域");
  return {temp_area_id, map_carrier, map_front};
}

/// 鞋子整理主编排（对应 Python shoe_sorting_demo.shoe_sort）。
void ShoeSortingDemo(const std::string& link_url,
                     const std::vector<std::vector<double>>& carrier_box,
                     const std::vector<std::vector<double>>& front_edge,
                     const std::string& area_name,
                     const std::string& vis_prefix,
                     bool no_voice = false) {
  auto& client = robot_sdk::Client::GetInstance();
  if (!client.Connect(link_url)) throw std::runtime_error("连接服务端失败");

  std::string temp_area_id;
  auto do_cleanup = [&]() {
    robot_sdk::eco_StopAll(10*1000);
    robot_sdk::eco_RobotPreparePose(10*1000);
    if (!temp_area_id.empty()) {
      robot_sdk::SemanticMapManagerData del_data;
      del_data.cmd = 4; del_data.object_info.id = temp_area_id;
      robot_sdk::eco_SemanticMapManager(del_data, nullptr, 10*1000);
    }
    robot_sdk::eco_SetCancellationFlag(nullptr);
    client.Disconnect();
    robot_sdk::Client::CleanupInstance();
  };

  robot_sdk::eco_SetCancellationFlag(&g_interrupted);
  try {
    robot_sdk::eco_StopAll();
    ThrowDemoOnError(robot_sdk::eco_RobotPreparePose(20*1000), "准备动作");

    Say("开始整理鞋子", no_voice);                                // 节点 1

    auto [tid, mc, mf] = ResolveOrganizeArea(carrier_box, front_edge, area_name, no_voice);
    temp_area_id = tid;
    std::string nav = temp_area_id.empty() ? area_name : temp_area_id;
    if (nav.empty()) throw std::runtime_error("必须有语义区域名或已创建临时区域");
    // ---- 重试循环（对应 Python for attempt in range(1, 4)）----
    for (int attempt = 1; attempt <= 3 && !g_interrupted; ++attempt) {
      Say("第" + std::to_string(attempt) + "/3次规划尝试", no_voice);  // 节点 2
      std::cout << "\n===== 规划尝试 " << attempt << "/3 =====" << std::endl;

      // 每次先导航到目标区域（3 次重试，对应 Python mission_retry）
      {
        robot_sdk::SemanticNavigationData nav_data;
        nav_data.area_name = nav;
        bool nav_ok = false;
        for (int nav_attempt = 1; nav_attempt <= 3 && !g_interrupted; ++nav_attempt) {
          robot_sdk::ErrorCode nav_ec = robot_sdk::eco_SemanticNavigation(nav_data, 20*1000);
          if (nav_ec.code == 0) { nav_ok = true; break; }
          std::cout << "    导航尝试 " << nav_attempt << "/3 失败: code=" << nav_ec.code
                    << ", msg=" << nav_ec.msg << std::endl;
          if (nav_attempt < 3) {
            Say("导航未成功，正在重试，别着急", no_voice);
            robot_sdk::eco_StopAll(10*1000);
          }
        }
        if (!nav_ok)
          throw std::runtime_error("语义导航到 " + nav + " 重试 3 次后失败");
      }

      std::string display_name = area_name.empty() ? "当前区域" : area_name;
      Say("已到达" + display_name + "，开始检测鞋子", no_voice);     // 节点 3

      // 步骤1: 头部拍照
      robot_sdk::ImageQueryData head_query;
      head_query.camera_type = robot_sdk::CameraType::HEAD;
      robot_sdk::ImageQueryNotifyData head_image;
      ThrowDemoOnError(robot_sdk::eco_ImageQuery(head_query, head_image), "头部拍照");

      // 步骤2: 检测鞋子
      std::vector<shoe_sorting::DetectedShoe> shoes;
      try {
        shoes = shoe_sorting::DetectShoes(head_image.rgb_image.data, {"鞋子"});
      } catch (const std::exception&) {
        // DetectShoes 无结果时抛异常，等价 Python 返回空列表
      }
      if (shoes.empty()) {
        Say("未检测到鞋子，任务结束", no_voice);                      // 节点 5
        std::cout << "未检测到鞋子，任务结束" << std::endl; break;
      }

      Say("检测到" + std::to_string(shoes.size()) + "只鞋子，开始配对", no_voice);  // 节点 4

      // 步骤3-5: 方案规划
      std::string attempt_prefix = vis_prefix.empty() ? "" : vis_prefix + "_" + std::to_string(attempt) + "_";
      auto plan_steps = shoe_sorting::GetShoeMoveSteps(
          head_image, shoes, mc, mf, attempt_prefix, /*move_all=*/ attempt == 1 );
      if (plan_steps.empty()) {
        Say("规划为空，任务结束", no_voice);                          // 节点 7
        std::cout << "规划为空，任务结束" << std::endl; break;
      }

      Say("规划完成，共" + std::to_string(plan_steps.size()) + "次搬运", no_voice);  // 节点 6
      
      // 头图参考框链 + 逐步骤执行
      auto head_refs = shoe_sorting::HeadImageRefBboxes(plan_steps);
      size_t total = plan_steps.size();
      for (size_t i = 0; i < plan_steps.size() && !g_interrupted; ++i) {
        Say("正在整理第" + std::to_string(plan_steps[i].step_id) + "/"
            + std::to_string(total) + "只", no_voice);              // 节点 8
        try {
          shoe_sorting::ExecutePickAndPlace(head_image, plan_steps[i], mf, "鞋子", head_refs[i], no_voice);
        } catch (const std::exception& e) {
          Say("第" + std::to_string(plan_steps[i].step_id) + "步搬运失败", no_voice);
          std::cerr << "步骤 " << plan_steps[i].step_id << " 失败: " << e.what() << std::endl;
          robot_sdk::eco_StopAll(10*1000);
          robot_sdk::eco_RobotPreparePose(10*1000);
          break;
        }
      }
    }

    Say("整理完成", no_voice);                                     // 节点 10

    ThrowDemoOnError(robot_sdk::eco_RobotEndingPose(10*1000), "结束姿态");
    do_cleanup();
  } catch (const std::exception& e) {
    Say("整理失败", no_voice);                                   // 节点 11
    std::cerr << "程序异常: " << e.what() << std::endl;
    do_cleanup(); throw; }
}

}  // namespace

int main(int argc, char* argv[]) {
  std::string link_url = robot_sdk::eco_GetDefaultLinkUrl(), vis_prefix, area_name;
  bool no_voice = false;
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "-h" || a == "--help") {
      std::cout << "用法: " << argv[0] << " [连接地址] [--vis-out PREFIX] [--area-name NAME] [--no-voice]\n";
      return 0;
    }
    if (a == "--vis-out" && i + 1 < argc) vis_prefix = argv[++i];
    else if (a == "--area-name" && i + 1 < argc) area_name = argv[++i];
    else if (a == "--no-voice") no_voice = true;
    else if (a[0] != '-') link_url = a;
  }

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);
  SetAndroidHostFromLinkUrl(link_url);

  std::cout << "=== 鞋子整理演示（C++）===" << std::endl;
  std::cout << "连接地址: " << link_url << std::endl;
  if (!area_name.empty()) std::cout << "语义区域: " << area_name << std::endl;

  try { ShoeSortingDemo(link_url, {}, {}, area_name, vis_prefix, no_voice); return 0; }
  catch (const std::exception& e) { std::cerr << "程序失败: " << e.what() << std::endl; return 1; }
}
