/**
 * 桌面整理场景演示 (C++ 版)
 *
 * 对齐 Python desk_demo：
 *   重定位 → 准备姿态 → 感知1(loop_0) → [规划 → 执行 → 感知(loop_k) → 评判]×max_loops → 结束姿态
 * VLM：通过 eco_DeskIntent*（HTTP）等价于 Python 侧 eco_vlm_*（WS）。
 */
#include <robot_sdk/robot_sdk.h>

#include "grab_items.hpp"
#include "speech.hpp"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <tuple>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

std::atomic<bool> g_interrupted{false};

void signal_handler(int /*signal*/) {
  g_interrupted = true;
  static const char kMsg[] = "\n接收到中断信号，正在退出...\n";
  [[maybe_unused]] ssize_t n = write(STDERR_FILENO, kMsg, sizeof(kMsg) - 1);
  (void)n;
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

}  // namespace

// ============================================================
// 配置结构
// ============================================================
struct DeskConfig {
  static constexpr int kFixedImgW = 640;
  static constexpr int kFixedImgH = 400;

  std::string area_name = "书桌";
  std::string area_id;
  std::string user_input;
  bool stop_before_grab = false;
  const int img_w = kFixedImgW;
  const int img_h = kFixedImgH;
  float connect_timeout_sec = 60.0f;
  int max_loops = 3;
  bool enable_vlm_head_angle_suggest = true;
  std::vector<float> head_sweep_angles_rad = {0.15f, 0.2f,  0.25f, 0.3f, 0.35f, 0.4f};
  float head_sweep_sleep_sec = 0.3f;
  float vlm_suggest_angle_timeout_sec = 150.0f;
  int sdk_max_attempts = 3;
  float sdk_retry_delay_sec = 0.35f;
  /** 用户侧仅记忆经验；可抓清单由 grab_items.md 自动前缀进 memory_input */
  std::string memory_input;
  std::string grab_items_path;
};

// ============================================================
// 工具函数
// ============================================================

namespace utils {

std::string new_task_id() {
  // 使用 uuidv4 风格的简单实现：8-4-4-4-12
  static const char hex[] = "0123456789abcdef";
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);
  auto r = [&]() { return hex[dis(gen)]; };
  std::string id;
  id.reserve(36);
  for (int i : {8, 4, 4, 4, 12}) {
    if (!id.empty()) id.push_back('-');
    for (int j = 0; j < i; ++j) id.push_back(r());
  }
  return id;
}

void ensure_dir(const std::filesystem::path& p) {
  std::filesystem::create_directories(p);
}

std::string json_escape(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 2);
  out.push_back('"');
  for (char c : s) {
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          char buf[8];
          std::snprintf(buf, sizeof(buf), "\\u%04x", c);
          out += buf;
        } else {
          out.push_back(c);
        }
    }
  }
  out.push_back('"');
  return out;
}

void write_json(const std::filesystem::path& path, const std::string& body) {
  ensure_dir(path.parent_path());
  std::ofstream ofs(path);
  if (ofs) ofs << body;
}

std::string image_to_data_url(const robot_sdk::ImageMsg& img) {
  if (img.data.empty()) return "";
  if (img.data.rfind("data:image", 0) == 0) return img.data;
  return "data:image/png;base64," + img.data;
}

/** 供 HTTP 请求的纯 base64（无 data: 前缀）；若已是 data URL 则去掉前缀。 */
std::string image_to_base64_payload(const robot_sdk::ImageMsg& img) {
  std::string u = image_to_data_url(img);
  const std::string prefix = "data:image/png;base64,";
  if (u.size() > prefix.size() && u.compare(0, prefix.size(), prefix) == 0)
    return u.substr(prefix.size());
  const std::string jpeg = "data:image/jpeg;base64,";
  if (u.size() > jpeg.size() && u.compare(0, jpeg.size(), jpeg) == 0)
    return u.substr(jpeg.size());
  if (u.rfind("base64,", 0) == 0) return u.substr(7);
  return img.data.empty() ? u : img.data;
}

std::string trim_ws(const std::string& s) {
  size_t beg = 0;
  while (beg < s.size() && std::isspace(static_cast<unsigned char>(s[beg]))) ++beg;
  size_t end = s.size();
  while (end > beg && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
  return s.substr(beg, end - beg);
}

std::string perception_objects_to_json(const std::vector<robot_sdk::DeskIntentPerceptionObject>& objs) {
  std::ostringstream oss;
  oss << '[';
  for (size_t i = 0; i < objs.size(); ++i) {
    if (i > 0) oss << ", ";
    const auto& o = objs[i];
    oss << "{\"name\": " << json_escape(o.name) << ", \"bbox\": [";
    for (size_t j = 0; j < o.bbox.size(); ++j) {
      if (j > 0) oss << ", ";
      oss << o.bbox[j];
    }
    oss << "]}";
  }
  oss << ']';
  return oss.str();
}

std::string bbox_id_map_to_json(const std::map<std::string, int>& mp) {
  std::ostringstream oss;
  oss << '{';
  bool first = true;
  for (const auto& kv : mp) {
    if (!first) oss << ", ";
    first = false;
    oss << json_escape(kv.first) << ": " << kv.second;
  }
  oss << '}';
  return oss.str();
}

}  // namespace utils

// ============================================================
// 错误处理
// ============================================================

class DeskDemoError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};


constexpr int kErrConnectFailed = 300101;
constexpr int kErrRelocateFailed = 300103;
constexpr int kErrPreparePoseFailed = 300104;
constexpr int kErrFinishPoseFailed = 300105;
constexpr int kErrArmHomeFailed = 300106;
constexpr int kErrSemanticNavFailed = 300201;
constexpr int kErrHeightCtrlFailed = 300202;
constexpr int kErrHeadCtrlFailed = 300203;
constexpr int kErrHeadAngleFailed = 300204;
constexpr int kErrImageQueryFailed = 300205;
constexpr int kErrPerceptionFailed = 300206;
constexpr int kErrModelTimeout = 300301;
constexpr int kErrModelGateway = 300303;
constexpr int kErrModelAuth = 300304;
constexpr int kErrPlanFailed = 300401;
constexpr int kErrPoseFailed = 300403;
constexpr int kErrFromPoseMissing = 300404;
constexpr int kErrBboxMatchFailed = 300406;
constexpr int kErrExecutionFailed = 300501;
constexpr int kErrGrabFailed = 300502;
constexpr int kErrPlaceFailed = 300503;
constexpr int kErrPlaceInFailed = 300504;
constexpr int kErrFallbackPlaceFailed = 300505;
constexpr int kErrJudgeFailed = 300601;

std::string ErrorHex(int code) {
  std::ostringstream oss;
  oss << "0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << code;
  return oss.str();
}

std::string ErrorTag(int code) {
  return "error_code=" + std::to_string(code) + " error_hex=" + ErrorHex(code);
}

std::string LowerCopy(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  return s;
}

bool ContainsAny(const std::string& s, std::initializer_list<const char*> parts) {
  for (const char* part : parts) {
    if (s.find(part) != std::string::npos) return true;
  }
  return false;
}

int ClassifyErrorCode(const std::string& name, const std::string& detail) {
  const std::string s = LowerCopy(name + " " + detail);
  if (ContainsAny(s, {"401", "403", "api_key", "api key", "api-key", "apikey",
                      "invalid token", "token invalid", "unauthorized", "forbidden", "鉴权",
                      "认证", "令牌无效"}))
    return kErrModelAuth;
  if (ContainsAny(s, {"502", "503", "504", "gateway", "service unavailable"}))
    return kErrModelGateway;
  if (ContainsAny(s, {"timed out", "timeout", "超时"})) return kErrModelTimeout;
  if (ContainsAny(s, {"websocket", "connect"})) return kErrConnectFailed;
  if (ContainsAny(s, {"maprelocation", "relocation"})) return kErrRelocateFailed;
  if (ContainsAny(s, {"preparepose"})) return kErrPreparePoseFailed;
  if (ContainsAny(s, {"endingpose", "ending_pose"})) return kErrFinishPoseFailed;
  if (ContainsAny(s, {"robotarmctrl", "arm_home"})) return kErrArmHomeFailed;
  if (ContainsAny(s, {"semanticnavigation"})) return kErrSemanticNavFailed;
  if (ContainsAny(s, {"heightctrl"})) return kErrHeightCtrlFailed;
  if (ContainsAny(s, {"headctrl"})) return kErrHeadCtrlFailed;
  if (ContainsAny(s, {"preperception", "suggest"})) return kErrHeadAngleFailed;
  if (ContainsAny(s, {"imagequery"})) return kErrImageQueryFailed;
  if (ContainsAny(s, {"deskintentperception"})) return kErrPerceptionFailed;
  if (ContainsAny(s, {"deskintentplan", "plan"})) return kErrPlanFailed;
  if (ContainsAny(s, {"getpose", "computeobjectpose"})) return kErrPoseFailed;
  if (ContainsAny(s, {"deskintentmatch", "match"})) return kErrBboxMatchFailed;
  if (ContainsAny(s, {"execution", "执行工作流"})) return kErrExecutionFailed;
  if (ContainsAny(s, {"grab"})) return kErrGrabFailed;
  if (ContainsAny(s, {"placein", "place_in"})) return kErrPlaceInFailed;
  if (ContainsAny(s, {"place"})) return kErrPlaceFailed;
  if (ContainsAny(s, {"judge"})) return kErrJudgeFailed;
  return kErrPerceptionFailed;
}

bool is_cancelled(const robot_sdk::ErrorCode& ec) {
  return ec.code == 4 || g_interrupted;
}

void check_or_throw(const robot_sdk::ErrorCode& ec, const std::string& op) {
  if (ec.code != 0) {
    throw DeskDemoError(op + " failed: code=" + std::to_string(ec.code) +
                        ", msg=" + ec.msg);
  }
}

/** 对齐 Python retry_utils.retry_run：异常退避重试，默认 3 次 / 0.35s。 */
template <typename Fn>
auto retry_run(Fn&& fn, const std::string& name, int max_attempts, double delay_sec) ->
    typename std::invoke_result<Fn>::type {
  using Ret = typename std::invoke_result<Fn>::type;
  for (int attempt = 1; attempt <= max_attempts; ++attempt) {
    if (g_interrupted.load()) throw DeskDemoError(name + " cancelled");
    try {
      Ret out = fn();
      if (attempt > 1)
        std::cout << name << " 成功（第 " << attempt << "/" << max_attempts << " 次尝试）"
                  << std::endl;
      return out;
    } catch (const std::exception& e) {
      if (attempt >= max_attempts) {
        std::cerr << ErrorTag(ClassifyErrorCode(name, e.what())) << " " << name << " 仍失败 ["
                  << attempt << "/" << max_attempts << "]: " << e.what() << std::endl;
        throw;
      }
      std::cerr << ErrorTag(ClassifyErrorCode(name, e.what())) << " " << name << " 失败 ["
                << attempt << "/" << max_attempts << "] 将重试: " << e.what() << std::endl;
      if (delay_sec > 0)
        std::this_thread::sleep_for(std::chrono::duration<double>(delay_sec));
    }
  }
  throw DeskDemoError(name + ": unreachable");
}

template <typename Fn>
void retry_run_void(Fn&& fn, const std::string& name, int max_attempts, double delay_sec) {
  retry_run(
      [&]() -> int {
        fn();
        return 0;
      },
      name, max_attempts, delay_sec);
}

void arm_home_after_execution(int max_attempts, double delay_sec) {
  std::cout << "==================================================" << std::endl;
  std::cout << "执行后收回机械臂" << std::endl;
  std::cout << "==================================================" << std::endl;
  retry_run_void(
      [&]() {
        robot_sdk::RobotArmCtrlData data;
        data.mode = 2;
        robot_sdk::ErrorCode ec = robot_sdk::eco_RobotArmCtrl(data, 30000);
        if (is_cancelled(ec)) throw DeskDemoError("eco_RobotArmCtrl cancelled");
        check_or_throw(ec, "eco_RobotArmCtrl");
      },
      "eco_RobotArmCtrl(ARM_HOME)", max_attempts, delay_sec);
}

bool robot_head_ctrl_retry(float rad, int max_attempts, double delay_sec) {
  try {
    retry_run_void(
        [&]() {
          robot_sdk::RobotHeadCtrlData hd;
          hd.value = rad;
          robot_sdk::ErrorCode ec = robot_sdk::eco_RobotHeadCtrl(hd, 120000);
          if (is_cancelled(ec)) throw DeskDemoError("eco_RobotHeadCtrl cancelled");
          check_or_throw(ec, "eco_RobotHeadCtrl");
        },
        "eco_RobotHeadCtrl", max_attempts, delay_sec);
    return true;
  } catch (const std::exception& e) {
    std::cerr << "eco_RobotHeadCtrl(" << rad << ") 失败: " << e.what() << std::endl;
    return false;
  }
}

bool image_query_retry(robot_sdk::CameraType camera_type, robot_sdk::ImageQueryNotifyData& out_view,
                       const std::string& name, int max_attempts, double delay_sec) {
  try {
    out_view = retry_run(
        [&]() -> robot_sdk::ImageQueryNotifyData {
          robot_sdk::ImageQueryData img_data;
          img_data.camera_type = camera_type;
          robot_sdk::ImageQueryNotifyData view;
          robot_sdk::ErrorCode ec = robot_sdk::eco_ImageQuery(img_data, view, 120000);
          if (is_cancelled(ec)) throw DeskDemoError(name + " cancelled");
          check_or_throw(ec, name.c_str());
          return view;
        },
        name, max_attempts, delay_sec);
    return true;
  } catch (const std::exception& e) {
    std::cerr << name << " 失败: " << e.what() << std::endl;
    return false;
  }
}

// ============================================================
// 意图规划 (eco_DeskIntentPlanSimple，含 perception + judge_input)
// ============================================================

struct IntentStep {
  int step_id = 0;
  std::vector<int> from_bbox;
  std::vector<int> to_bbox;
  bool is_container = false;
  std::string reason;
  std::string placement_type;
  std::optional<std::string> container_name;
};

bool desk_intent_plan(const robot_sdk::ImageMsg& rgb_image,
                      const std::string& user_input,
                      const std::vector<robot_sdk::DeskIntentPerceptionObject>& perception_objects,
                      const std::string& judge_input,
                      const std::string& user_memory_experience,
                      const std::filesystem::path& grab_items_path,
                      std::vector<IntentStep>& out_steps,
                      const std::filesystem::path& output_dir,
                      int max_attempts,
                      double retry_delay_sec) {
  const std::string memory_sent =
      grab_items::build_memory_input_for_plan(user_memory_experience, nullptr, grab_items_path);
  if (!utils::trim_ws(user_memory_experience).empty()) {
    std::cout << "  用户 memory_input(经验): "
              << utils::trim_ws(user_memory_experience).substr(0, 200) << std::endl;
  }
  const size_t log_len = std::min(memory_sent.size(), size_t{300});
  std::cout << "  实际 memory_input(len=" << memory_sent.size()
            << "): " << memory_sent.substr(0, log_len) << std::endl;

  robot_sdk::DeskIntentPlanRequest req;
  req.image = utils::image_to_data_url(rgb_image);
  req.user_input = user_input;
  req.perception = perception_objects;
  if (!judge_input.empty()) req.judge_input = judge_input;
  req.memory_input = memory_sent;

  std::vector<robot_sdk::DeskIntentPlanStep> steps;
  std::string error_msg;

  constexpr int kPlanTimeoutMs = 360000;
  bool ok = false;
  try {
    ok = retry_run(
        [&]() -> bool {
          std::string err;
          if (!robot_sdk::eco_DeskIntentPlanSimple(req, steps, err, kPlanTimeoutMs))
            throw DeskDemoError(err);
          error_msg = std::move(err);
          return true;
        },
        "eco_DeskIntentPlan", max_attempts, retry_delay_sec);
  } catch (const DeskDemoError& e) {
    error_msg = e.what();
    const int code = ClassifyErrorCode("eco_DeskIntentPlan", error_msg);
    if (code == kErrModelAuth)
      throw DeskDemoError(ErrorTag(code) + " desk_intent_plan failed: " + error_msg);
    ok = false;
  }
  if (!ok) {
    std::cerr << ErrorTag(kErrPlanFailed) << " desk_intent_plan failed: " << error_msg << std::endl;
    std::string task_id = utils::new_task_id();
    std::string json_body =
        "{\"task_id\": " + utils::json_escape(task_id) + ", \"status\": \"error\", \"error_msg\": " +
        utils::json_escape(error_msg) +
        ", \"plan_backend\": \"eco_DeskIntentPlan\", \"plan_response_format\": \"desk_intent_http\"}";
    utils::write_json(output_dir / "plan_result.json", json_body);
    return false;
  }

  out_steps.clear();
  for (const auto& s : steps) {
    IntentStep step;
    step.step_id = s.step_id;
    step.from_bbox = s.from_bbox;
    step.to_bbox = s.to_bbox;
    step.is_container = s.is_container;
    step.reason = s.reason;
    step.placement_type = s.placement_type;
    step.container_name = s.container_name;
    out_steps.push_back(std::move(step));
  }

  std::string task_id = utils::new_task_id();
  std::ostringstream oss;
  oss << "{\"task_id\": " << utils::json_escape(task_id) << ", \"status\": \"success\", "
      << "\"user_input\": " << utils::json_escape(user_input)
      << ", \"plan_backend\": \"eco_DeskIntentPlan\", \"plan_response_format\": \"desk_intent_http\", "
      << "\"intent_steps\": [";
  for (size_t i = 0; i < steps.size(); ++i) {
    const auto& s = steps[i];
    if (i > 0) oss << ", ";
    oss << "{\"step_id\": " << s.step_id << ", \"from_bbox\": [";
    for (size_t j = 0; j < s.from_bbox.size(); ++j) {
      if (j > 0) oss << ", ";
      oss << s.from_bbox[j];
    }
    oss << "], \"to_bbox\": [";
    for (size_t j = 0; j < s.to_bbox.size(); ++j) {
      if (j > 0) oss << ", ";
      oss << s.to_bbox[j];
    }
    oss << "], \"is_container\": " << (s.is_container ? "true" : "false");
    if (!s.reason.empty()) oss << ", \"reason\": " << utils::json_escape(s.reason);
    if (!s.placement_type.empty())
      oss << ", \"placement_type\": " << utils::json_escape(s.placement_type);
    if (s.container_name.has_value() && !s.container_name->empty())
      oss << ", \"container_name\": " << utils::json_escape(*s.container_name);
    oss << "}";
  }
  oss << "], \"steps_count\": " << steps.size() << "}";
  utils::write_json(output_dir / "plan_result.json", oss.str());

  return true;
}

// ============================================================
// 物品 bbox 匹配 (eco_DeskIntentMatchSimple)
// ============================================================

bool desk_intent_match(const robot_sdk::ImageMsg& arm_image,
                       const robot_sdk::ImageMsg& base_image,
                       const std::vector<int>& from_bbox,
                       std::vector<int>& out_bbox,
                       const std::filesystem::path& output_dir) {
  std::cout << "--- calling desk_intent_match ---" << std::endl;

  if (from_bbox.size() != 4) {
    std::cerr << ErrorTag(kErrBboxMatchFailed) << " desk_intent_match: from_bbox must be 4 elements" << std::endl;
    return false;
  }

  robot_sdk::DeskIntentMatchRequest req;
  req.arm_image = utils::image_to_data_url(arm_image);
  req.base_image = utils::image_to_data_url(base_image);
  req.bbox = from_bbox;

  std::vector<int> matched_bbox;
  std::string error_msg;

  bool ok = robot_sdk::eco_DeskIntentMatchSimple(req, matched_bbox, error_msg, 180000);
  if (!ok) {
    std::cerr << ErrorTag(kErrBboxMatchFailed) << " desk_intent_match failed: " << error_msg << std::endl;

    std::string task_id = utils::new_task_id();
    std::string json_body = "{\"task_id\": " + utils::json_escape(task_id) +
                            ", \"status\": \"error\", \"error_msg\": " +
                            utils::json_escape(error_msg) + "}";
    utils::write_json(output_dir / "desk_intent_match_result.json", json_body);
    return false;
  }

  if (matched_bbox.size() != 4) {
    std::cerr << ErrorTag(kErrBboxMatchFailed) << " desk_intent_match: returned bbox size is not 4" << std::endl;
    return false;
  }

  out_bbox = matched_bbox;

  std::string task_id = utils::new_task_id();
  std::string bbox_json = "{\"task_id\": " + utils::json_escape(task_id) +
                          ", \"status\": \"success\", \"bbox\": [" +
                          std::to_string(out_bbox[0]) + ", " +
                          std::to_string(out_bbox[1]) + ", " +
                          std::to_string(out_bbox[2]) + ", " +
                          std::to_string(out_bbox[3]) + "]}";
  utils::write_json(output_dir / "desk_intent_match_result.json", bbox_json);

  std::cout << "desk_intent_match ok, bbox=[" << out_bbox[0] << "," << out_bbox[1]
            << "," << out_bbox[2] << "," << out_bbox[3] << "]" << std::endl;
  return true;
}

// ============================================================
// 感知工作流：导航 → 升高度 → 固定头部俯仰 0.4 rad → 拍照 → DeskIntentPerception
// ============================================================

struct PerceptionOutcome {
  robot_sdk::ImageQueryNotifyData head_view;
  std::vector<robot_sdk::DeskIntentPerceptionObject> perception_objects;
  std::optional<float> applied_head_angle_rad;
};

// ---------------------------------------------------------------------------
// 多角度采样 + eco_DeskIntentPrePerception（与 Python head_sweep_angles_rad 一致）
// ---------------------------------------------------------------------------
bool run_head_angle_suggest_pipeline(const std::vector<float>& angles_rad,
                                     float sleep_sec,
                                     int suggest_timeout_ms,
                                     const std::filesystem::path& output_dir,
                                     int max_attempts,
                                     double retry_delay_sec,
                                     float* out_applied_rad) {
  std::vector<robot_sdk::DeskIntentPrePerceptionImage> images;
  robot_sdk::DeskIntentPrePerceptionRequest pre_req;
  const auto rec_path = output_dir / "step_head_angle_vlm_suggest.json";
  std::optional<float> last_set_rad;
  const float fallback_rad = !angles_rad.empty() ? angles_rad.back() : 0.4f;

  for (size_t idx = 0; idx < angles_rad.size(); ++idx) {
    float rad = angles_rad[idx];

    if (!robot_head_ctrl_retry(rad, max_attempts, retry_delay_sec)) {
      std::cerr << "采样角 " << rad << "：设置头部失败，跳过" << std::endl;
      continue;
    }
    last_set_rad = rad;

    if (sleep_sec > 0)
      std::this_thread::sleep_for(std::chrono::duration<double>(sleep_sec));

    robot_sdk::ImageQueryNotifyData cap;
    {
      std::ostringstream cap_name;
      cap_name << "eco_ImageQuery(head_sample rad=" << std::fixed << std::setprecision(4) << rad
               << ")";
      if (!image_query_retry(robot_sdk::CameraType::HEAD, cap, cap_name.str(), max_attempts,
                             retry_delay_sec)) {
        std::cerr << "采样角 " << rad << "：拍照失败，跳过" << std::endl;
        continue;
      }
    }

    robot_sdk::DeskIntentPrePerceptionImage pi;
    pi.image_base64 = utils::image_to_data_url(cap.rgb_image);
    pi.angle = static_cast<double>(rad);
    images.push_back(std::move(pi));
  }

  auto apply_fallback = [&](const std::string& reason, const std::string& error = std::string()) {
    const float applied_fallback = last_set_rad.has_value() ? *last_set_rad : fallback_rad;
    const bool applied_ok = robot_head_ctrl_retry(applied_fallback, max_attempts, retry_delay_sec);
    if (sleep_sec > 0 && applied_ok)
      std::this_thread::sleep_for(std::chrono::duration<double>(sleep_sec));
    std::cerr << "头部选角未完成，回退到 " << std::fixed << std::setprecision(4) << applied_fallback
              << " rad（原因: " << reason << "）" << std::endl;
    std::ostringstream skip;
    skip << "{\"status\": \"fallback\", \"reason\": " << utils::json_escape(reason)
         << ", \"sample_count\": " << images.size()
         << ", \"fallback_rad\": " << applied_fallback
         << ", \"fallback_applied\": " << (applied_ok ? "true" : "false");
    if (!error.empty()) skip << ", \"error\": " << utils::json_escape(error);
    skip << "}";
    utils::write_json(rec_path, skip.str());
    if (out_applied_rad) *out_applied_rad = applied_fallback;
    return false;
  };

  if (images.size() < 2) {
    std::cerr << "有效采样不足 " << images.size()
              << " 组（需要≥2），跳过 eco_DeskIntentPrePerception" << std::endl;
    return apply_fallback("not_enough_samples");
  }

  speech::Speak(speech::kHeadAngle);
  pre_req.images = std::move(images);
  double suggested = 0;
  std::string err;
  try {
    suggested = retry_run(
        [&]() -> double {
          double s = 0;
          std::string e;
          if (!robot_sdk::eco_DeskIntentPrePerceptionSimple(pre_req, s, e, suggest_timeout_ms))
            throw DeskDemoError(e);
          return s;
        },
        "eco_DeskIntentPrePerceptionSimple", max_attempts, retry_delay_sec);
  } catch (const std::exception& e) {
    err = e.what();
    std::cerr << ErrorTag(ClassifyErrorCode("eco_DeskIntentPrePerceptionSimple", err)) << " eco_DeskIntentPrePerceptionSimple failed: " << err << std::endl;
    return apply_fallback("suggest_failed", err);
  }

  double applied = std::max(0.0, std::min(1.3, suggested));
  if (!robot_head_ctrl_retry(static_cast<float>(applied), max_attempts, retry_delay_sec)) {
    std::cerr << ErrorTag(kErrHeadAngleFailed) << " 应用推荐头部角度失败" << std::endl;
    return apply_fallback("apply_suggested_failed");
  }

  {
    std::ostringstream okj;
    okj << "{\"status\": \"success\", \"suggested_rad_raw\": " << suggested
        << ", \"applied_rad\": " << applied << "}";
    utils::write_json(rec_path, okj.str());
  }

  if (sleep_sec > 0)
    std::this_thread::sleep_for(std::chrono::duration<double>(sleep_sec));
  if (out_applied_rad) *out_applied_rad = static_cast<float>(applied);
  std::cout << "头部选角完成，当前应用角度: " << std::fixed << std::setprecision(4) << applied
            << " rad（推荐 raw=" << suggested << "）" << std::endl;
  return true;
}

bool run_perception_workflow(PerceptionOutcome& out,
                             const std::string& area_name,
                             const std::string& area_id,
                             int img_w,
                             int img_h,
                             const DeskConfig& cfg,
                             const std::filesystem::path& output_dir,
                             const std::optional<float>& reuse_head_angle_rad = std::nullopt) {
  (void)img_w;
  (void)img_h;

  const int max_attempts = cfg.sdk_max_attempts;
  const double retry_delay_sec = static_cast<double>(cfg.sdk_retry_delay_sec);

  // 语义导航
  {
    speech::Speak(speech::kNavigating);
    retry_run_void(
        [&]() {
          robot_sdk::SemanticNavigationData nav_data;
          nav_data.area_name = area_name;
          nav_data.area_id = area_id;
          robot_sdk::ErrorCode ec = robot_sdk::eco_SemanticNavigation(nav_data, 180000);
          if (is_cancelled(ec)) throw DeskDemoError("semantic_navigation cancelled");
          check_or_throw(ec, "eco_SemanticNavigation");
        },
        "eco_SemanticNavigation", max_attempts, retry_delay_sec);

    std::string task_id = utils::new_task_id();
    std::string json_body = "{\"task_id\": " + utils::json_escape(task_id) +
                            ", \"area_name\": " + utils::json_escape(area_name) +
                            ", \"area_id\": " + utils::json_escape(area_id) +
                            ", \"status\": \"success\"}";
    utils::write_json(output_dir / "step2_navigate.json", json_body);
  }

  // 机身高度
  {
    speech::Speak(speech::kRaising);
    retry_run_void(
        [&]() {
          robot_sdk::RobotHeightCtrlData height_data;
          height_data.value = 0.44f;
          robot_sdk::ErrorCode ec = robot_sdk::eco_RobotHeightCtrl(height_data, 120000);
          if (is_cancelled(ec)) throw DeskDemoError("robot_height_ctrl cancelled");
          check_or_throw(ec, "eco_RobotHeightCtrl");
        },
        "eco_RobotHeightCtrl", max_attempts, retry_delay_sec);

    std::string task_id = utils::new_task_id();
    std::string json_body = "{\"task_id\": " + utils::json_escape(task_id) +
                            ", \"mode\": 1, \"value_m\": 0.44, \"status\": \"success\"}";
    utils::write_json(output_dir / "step3_height_ctrl.json", json_body);
  }

  out.applied_head_angle_rad.reset();

  // 头部选角：首次执行选角；后续感知优先复用首次确定的角度
  if (reuse_head_angle_rad.has_value()) {
    const float reused = *reuse_head_angle_rad;
    std::cout << "复用已缓存头部角度: " << std::fixed << std::setprecision(4) << reused
              << " rad" << std::endl;
    const bool applied_ok = robot_head_ctrl_retry(reused, max_attempts, retry_delay_sec);
    if (!applied_ok)
      std::cerr << ErrorTag(kErrHeadCtrlFailed) << " 复用头部角度失败，使用当前头部姿态继续拍照" << std::endl;
    if (cfg.head_sweep_sleep_sec > 0 && applied_ok)
      std::this_thread::sleep_for(
          std::chrono::duration<double>(static_cast<double>(cfg.head_sweep_sleep_sec)));
    std::ostringstream reusej;
    reusej << "{\"status\": \"reused\", \"applied_rad\": " << reused
           << ", \"applied_ok\": " << (applied_ok ? "true" : "false") << "}";
    utils::write_json(output_dir / "step_head_angle_vlm_suggest.json", reusej.str());
    out.applied_head_angle_rad = reused;
  } else if (cfg.enable_vlm_head_angle_suggest) {
    const int suggest_timeout_ms =
        static_cast<int>(cfg.vlm_suggest_angle_timeout_sec * 1000.0f);
    float applied = 0.0f;
    const bool ok = run_head_angle_suggest_pipeline(
        cfg.head_sweep_angles_rad, cfg.head_sweep_sleep_sec, suggest_timeout_ms, output_dir,
        max_attempts, retry_delay_sec, &applied);
    out.applied_head_angle_rad = applied;
    if (!ok)
      std::cerr << "头部选角未完成，已回退并应用头部角度: " << applied << " rad" << std::endl;
  } else {
    constexpr float kFormalHeadPitchRad = 0.4f;
    std::cout << "头部俯仰: 固定 " << std::fixed << std::setprecision(4) << kFormalHeadPitchRad
              << " rad（未启用选角）" << std::endl;
    if (!robot_head_ctrl_retry(kFormalHeadPitchRad, max_attempts, retry_delay_sec))
      std::cerr << "eco_RobotHeadCtrl 固定角警告，仍继续拍照" << std::endl;
    if (cfg.head_sweep_sleep_sec > 0)
      std::this_thread::sleep_for(
          std::chrono::duration<double>(static_cast<double>(cfg.head_sweep_sleep_sec)));
    std::ostringstream fixj;
    fixj << "{\"status\": \"fixed_angle\", \"mode\": \"skip_vlm_head_sweep\", "
         << "\"applied_rad\": " << kFormalHeadPitchRad << "}";
    utils::write_json(output_dir / "step_head_angle_vlm_suggest.json", fixj.str());
    out.applied_head_angle_rad = kFormalHeadPitchRad;
  }

  {
    robot_sdk::ImageQueryNotifyData head_view;
    if (!image_query_retry(robot_sdk::CameraType::HEAD, head_view, "eco_ImageQuery(head)", max_attempts, retry_delay_sec))
      throw DeskDemoError("image_query (head) failed");

    std::string task_id = utils::new_task_id();
    std::string json_body = "{\"task_id\": " + utils::json_escape(task_id) +
                            ", \"camera_type\": 2, \"status\": \"success\"}";
    utils::write_json(output_dir / "step4_image_query_head.json", json_body);
    out.head_view = std::move(head_view);
  }

  // 物体感知（HTTP）
  out.perception_objects.clear();
  {
    speech::Speak(speech::kPerceiving);
    robot_sdk::DeskIntentPerceptionRequest pr;
    pr.image = utils::image_to_data_url(out.head_view.rgb_image);

    std::vector<robot_sdk::DeskIntentPerceptionObject> objects;
    constexpr int kPercTimeoutMs = 150000;
    try {
      objects = retry_run(
          [&]() -> std::vector<robot_sdk::DeskIntentPerceptionObject> {
            std::vector<robot_sdk::DeskIntentPerceptionObject> objs;
            std::string e;
            if (!robot_sdk::eco_DeskIntentPerceptionSimple(pr, objs, e, kPercTimeoutMs))
              throw DeskDemoError(e);
            return objs;
          },
          "eco_DeskIntentPerception", max_attempts, retry_delay_sec);
    } catch (const std::exception& e) {
      const int code = ClassifyErrorCode("eco_DeskIntentPerception", e.what());
      std::cerr << ErrorTag(code) << " eco_DeskIntentPerception 失败: " << e.what()
                << std::endl;
      if (code == kErrModelAuth)
        throw DeskDemoError(ErrorTag(code) + " eco_DeskIntentPerception 失败: " + e.what());
      return false;
    }

    out.perception_objects = std::move(objects);
    int pid = 0;
    for (auto& o : out.perception_objects) {
      if (o.name.empty()) o.name = "obj_" + std::to_string(pid);
      ++pid;
    }

    std::ostringstream percf;
    percf << "{\"perception\": " << utils::perception_objects_to_json(out.perception_objects) << "}";
    utils::write_json(output_dir / "step4_perception.json", percf.str());
    std::cout << "感知完成，共 " << out.perception_objects.size() << " 个物体" << std::endl;
  }

  std::ostringstream sum;
  sum << "{\"workflow\": \"perception_workflow\", \"status\": \"success\", "
      << "\"outputs\": {\"perception\": " << utils::json_escape((output_dir / "step4_perception.json").string())
      << "}}";
  utils::write_json(output_dir / "perception_workflow_summary.json", sum.str());

  std::cout << "感知工作流执行完毕" << std::endl;
  return true;
}

// ============================================================
// 执行工作流
// ============================================================

bool get_pose_from_bboxes(const robot_sdk::ImageQueryNotifyData& head_view,
                          const std::vector<robot_sdk::BboxItem>& bboxes,
                          std::vector<robot_sdk::PoseResultItem>& out_poses,
                          const std::filesystem::path& output_dir,
                          int max_attempts,
                          double retry_delay_sec) {
  if (bboxes.empty()) {
    std::cerr << ErrorTag(kErrPoseFailed) << " bbox 列表为空" << std::endl;
    return false;
  }

  std::cout << "6D姿态计算入参: bbox数量=" << bboxes.size() << std::endl;

  robot_sdk::GetPoseData pose_data;
  pose_data.bbox = bboxes;
  pose_data.rgb_image = head_view.rgb_image;
  pose_data.depth_image = head_view.depth_image;
  pose_data.tf_map = head_view.tf_goal;

  robot_sdk::GetPoseNotifyData notify;
  try {
    notify = retry_run(
        [&]() -> robot_sdk::GetPoseNotifyData {
          robot_sdk::GetPoseNotifyData n;
          robot_sdk::ErrorCode ec = robot_sdk::eco_GetPose(pose_data, n, 180000);
          if (is_cancelled(ec)) throw DeskDemoError("eco_GetPose cancelled");
          check_or_throw(ec, "eco_GetPose");
          if (n.pose_results.empty()) throw DeskDemoError("eco_GetPose: no pose results");
          return n;
        },
        "eco_GetPose", max_attempts, retry_delay_sec);
  } catch (const std::exception& e) {
    std::cerr << ErrorTag(ClassifyErrorCode("eco_GetPose", e.what())) << " get_pose failed: " << e.what() << std::endl;
    return false;
  }

  out_poses = std::move(notify.pose_results);

  if (out_poses.empty()) {
    std::cerr << ErrorTag(kErrPoseFailed) << " 未获取到姿态结果" << std::endl;
    return false;
  }

  // 写 JSON 结果
  std::string task_id = utils::new_task_id();
  std::ostringstream oss;
  oss << "{\"task_id\": " + utils::json_escape(task_id) + ", \"status\": \"success\", ";
  oss << "\"pose_results\": [";
  for (size_t i = 0; i < out_poses.size(); ++i) {
    const auto& p = out_poses[i];
    if (i > 0) oss << ", ";
    oss << "{\"id\": " << p.id
        << ", \"position\": {\"x\": " << p.position.x << ", \"y\": " << p.position.y
        << ", \"z\": " << p.position.z << "}"
        << ", \"orientation\": {\"x\": " << p.orientation.x << ", \"y\": " << p.orientation.y
        << ", \"z\": " << p.orientation.z << ", \"w\": " << p.orientation.w << "}"
        << ", \"box_length\": {\"x\": " << p.box_length.x << ", \"y\": " << p.box_length.y
        << ", \"z\": " << p.box_length.z << "}"
        << ", \"frame_id\": " << utils::json_escape(p.frame_id) << "}";
  }
  oss << "]}";
  utils::write_json(output_dir / "step5_get_pose.json", oss.str());

  std::cout << "6D姿态计算完成，获取到 " << out_poses.size() << " 个姿态" << std::endl;
  return true;
}

bool pose_adaptive(const robot_sdk::PoseResultItem& pose_item,
                   const std::filesystem::path& output_dir,
                   int max_attempts,
                   double retry_delay_sec) {
  robot_sdk::BoxData box;
  box.frame_id = pose_item.frame_id;
  box.position = pose_item.position;
  box.orientation = pose_item.orientation;
  box.box_length = pose_item.box_length;

  try {
    retry_run_void(
        [&]() {
          robot_sdk::ErrorCode ec =
              robot_sdk::eco_LookTo(robot_sdk::LookToTarget::HAND, box, 180000);
          if (is_cancelled(ec)) throw DeskDemoError("eco_LookTo cancelled");
          check_or_throw(ec, "eco_LookTo");
        },
        "eco_LookTo", max_attempts, retry_delay_sec);
  } catch (const std::exception& e) {
    std::cerr << ErrorTag(ClassifyErrorCode("eco_LookTo", e.what())) << " 位置自适应失败: " << e.what() << std::endl;
    return false;
  }

  std::string task_id = utils::new_task_id();
  std::ostringstream oss;
  oss << "{\"task_id\": " << utils::json_escape(task_id)
      << ", \"frame_id\": " << utils::json_escape(pose_item.frame_id)
      << ", \"position\": {\"x\": " << pose_item.position.x << ", \"y\": " << pose_item.position.y
      << ", \"z\": " << pose_item.position.z << "}"
      << ", \"orientation\": {\"x\": " << pose_item.orientation.x << ", \"y\": " << pose_item.orientation.y
      << ", \"z\": " << pose_item.orientation.z << ", \"w\": " << pose_item.orientation.w << "}"
      << ", \"box_length\": {\"x\": " << pose_item.box_length.x
      << ", \"y\": " << pose_item.box_length.y << ", \"z\": " << pose_item.box_length.z << "}"
      << ", \"status\": \"success\"}";
  utils::write_json(output_dir / "step6_pose_adaptive.json", oss.str());
  std::cout << "位置自适应完成" << std::endl;
  return true;
}

bool arm_image_query(robot_sdk::ImageQueryNotifyData& out_arm_view,
                     const std::filesystem::path& output_dir,
                     int max_attempts,
                     double retry_delay_sec) {
  std::cout << "============================================================" << std::endl;
  std::cout << "手臂相机拍照" << std::endl;
  std::cout << "============================================================" << std::endl;

  if (!image_query_retry(robot_sdk::CameraType::ARM, out_arm_view, "eco_ImageQuery(ARM)", max_attempts, retry_delay_sec))
    return false;

  std::string task_id = utils::new_task_id();
  std::string json_body = "{\"task_id\": " + utils::json_escape(task_id) +
                          ", \"camera_type\": 1, \"status\": \"success\"}";
  utils::write_json(output_dir / "step4_image_query_arm.json", json_body);

  std::cout << "手臂相机拍照成功" << std::endl;
  return true;
}

bool process_item(const IntentStep& step,
                  const robot_sdk::ImageQueryNotifyData& head_view_frozen,
                  const std::vector<robot_sdk::PoseResultItem>& pose_results,
                  const std::map<std::string, int>& bbox_id_map,
                  const std::filesystem::path& output_dir,
                  bool stop_before_grab,
                  int max_attempts,
                  double retry_delay_sec) {
  int step_id = step.step_id;
  std::cout << "=== 处理物品 step_id=" << step_id << " ===" << std::endl;

  std::string k_from = std::to_string(step_id) + "_from";
  std::string k_to = std::to_string(step_id) + "_to";

  auto it_from = bbox_id_map.find(k_from);
  auto it_to = bbox_id_map.find(k_to);
  int from_bbox_id = it_from != bbox_id_map.end() ? it_from->second : -1;
  int to_bbox_id = it_to != bbox_id_map.end() ? it_to->second : -1;

  const robot_sdk::PoseResultItem* from_pose = nullptr;
  const robot_sdk::PoseResultItem* to_pose = nullptr;

  for (const auto& p : pose_results) {
    if (from_bbox_id >= 0 && p.id == from_bbox_id) from_pose = &p;
    if (to_bbox_id >= 0 && p.id == to_bbox_id) to_pose = &p;
  }

  if (!from_pose) {
    std::cerr << ErrorTag(kErrFromPoseMissing) << " 未获取到 step_id=" << step_id << " 的 from_pose (bbox_id=" << from_bbox_id << ")"
              << std::endl;
    return false;
  }
  if (!to_pose) {
    std::cerr << "未获取到 step_id=" << step_id << " 的 to_pose，使用 from_pose 代替" << std::endl;
    to_pose = from_pose;
  }

  std::cout << "--- 位置自适应 ---" << std::endl;
  bool ok = pose_adaptive(*from_pose, output_dir, max_attempts, retry_delay_sec);
  if (!ok) std::cerr << "位置自适应失败，继续执行后续任务" << std::endl;

  std::cout << "--- 手臂相机拍照 ---" << std::endl;
  robot_sdk::ImageQueryNotifyData arm_view;
  ok = arm_image_query(arm_view, output_dir, max_attempts, retry_delay_sec);
  if (!ok) return false;

  std::cout << "--- 获取物品bbox ---" << std::endl;
  std::vector<int> item_bbox;
  ok = desk_intent_match(arm_view.rgb_image, head_view_frozen.rgb_image, step.from_bbox,
                         item_bbox, output_dir);
  if (!ok) return false;

  std::string bbox_json = "{\"status\": \"success\", \"bbox\": [" +
                          std::to_string(item_bbox[0]) + ", " +
                          std::to_string(item_bbox[1]) + ", " +
                          std::to_string(item_bbox[2]) + ", " +
                          std::to_string(item_bbox[3]) + "]}";
  utils::write_json(output_dir / "step11_item_bbox.json", bbox_json);

  std::string item_name = utils::trim_ws(step.reason);
  if (item_name.empty()) item_name = "item_" + std::to_string(step_id);

  if (stop_before_grab) {
    std::cout << "已按配置在抓取前停止（仅生成到 bbox 输出）" << std::endl;
    return true;
  }

  std::cout << "--- 物品抓取 ---" << std::endl;
  speech::Speak(speech::kGrabbing);
  {
    robot_sdk::AccurateGrabData grab_data;
    grab_data.rgb_image = arm_view.rgb_image;
    grab_data.depth_image = arm_view.depth_image;
    grab_data.tf_goal = arm_view.tf_goal;
    grab_data.bbox.name = item_name;
    if (item_bbox.size() == 4) {
      grab_data.bbox.left_up_x = static_cast<float>(item_bbox[0]);
      grab_data.bbox.left_up_y = static_cast<float>(item_bbox[1]);
      grab_data.bbox.right_down_x = static_cast<float>(item_bbox[2]);
      grab_data.bbox.right_down_y = static_cast<float>(item_bbox[3]);
    }

    bool grab_ok = false;
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
      if (g_interrupted.load()) return false;
      robot_sdk::ErrorCode ec = robot_sdk::eco_AccurateGrab(grab_data, 300000);
      if (is_cancelled(ec)) return false;
      if (ec.code == 0) {
        grab_ok = true;
        if (attempt > 1)
          std::cout << "eco_AccurateGrab 成功（第 " << attempt << "/" << max_attempts
                    << " 次尝试）" << std::endl;
        break;
      }
      std::cerr << ErrorTag(kErrGrabFailed) << " 物品抓取失败 [" << attempt << "/" << max_attempts
                << "]: code=" << ec.code << ", msg=" << ec.msg << std::endl;
      if (attempt < max_attempts && retry_delay_sec > 0)
        std::this_thread::sleep_for(std::chrono::duration<double>(retry_delay_sec));
    }
    if (!grab_ok) return false;

    std::string task_id = utils::new_task_id();
    std::string json_body =
        "{\"task_id\": " + utils::json_escape(task_id) +
        ", \"item_name\": " + utils::json_escape(item_name) +
        ", \"bbox\": [" + std::to_string(item_bbox[0]) + ", " +
        std::to_string(item_bbox[1]) + ", " + std::to_string(item_bbox[2]) + ", " +
        std::to_string(item_bbox[3]) + "], \"status\": \"success\"}";
    utils::write_json(output_dir / "step7_accurate_grab.json", json_body);
  }

  speech::Speak(speech::kPlacing);
  if (step.is_container) {
    std::cout << "--- 容器放置 ---" << std::endl;
  } else {
    std::cout << "--- 精准放置 ---" << std::endl;
  }

  {
    robot_sdk::AccuratePlaceData place_data;
    place_data.frame_id = to_pose->frame_id;
    place_data.position = to_pose->position;
    place_data.orientation = to_pose->orientation;
    place_data.box_length = to_pose->box_length;

    auto try_place = [&](const robot_sdk::AccuratePlaceData& data) -> bool {
      for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        if (g_interrupted.load()) return false;
        robot_sdk::ErrorCode ec = robot_sdk::eco_AccuratePlace(data, 300000);
        if (is_cancelled(ec)) return false;
        if (ec.code == 0) {
          if (attempt > 1)
            std::cout << "eco_AccuratePlace 成功（第 " << attempt << "/" << max_attempts
                      << " 次尝试）" << std::endl;
          return true;
        }
        std::cerr << ErrorTag(kErrPlaceFailed) << " 精准放置失败 [" << attempt << "/" << max_attempts
                  << "]: code=" << ec.code << ", msg=" << ec.msg << std::endl;
        if (attempt < max_attempts && retry_delay_sec > 0)
          std::this_thread::sleep_for(std::chrono::duration<double>(retry_delay_sec));
      }
      return false;
    };

    bool place_success = false;
    if (step.is_container) {
      if (step.to_bbox.size() < 4) {
        std::cerr << ErrorTag(kErrPlaceInFailed) << " 容器放置失败：to_bbox 无效，尝试使用精准放置回退" << std::endl;
      } else {
        std::vector<float> carrier_bbox;
        carrier_bbox.reserve(4);
        for (int i = 0; i < 4; ++i) {
          carrier_bbox.push_back(static_cast<float>(step.to_bbox[i]));
        }
        for (int attempt = 1; attempt <= max_attempts; ++attempt) {
          if (g_interrupted.load()) return false;
          robot_sdk::ErrorCode ec = robot_sdk::eco_PlaceIn(head_view_frozen, carrier_bbox, 300000);
          if (is_cancelled(ec)) return false;
          if (ec.code == 0) {
            place_success = true;
            if (attempt > 1) {
              std::cout << "eco_PlaceIn 成功（第 " << attempt << "/" << max_attempts
                        << " 次尝试）" << std::endl;
            }
            break;
          }
          std::cerr << ErrorTag(kErrPlaceInFailed) << " 容器放置失败 [" << attempt << "/" << max_attempts
                    << "]: code=" << ec.code << ", msg=" << ec.msg << std::endl;
          if (attempt < max_attempts && retry_delay_sec > 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(retry_delay_sec));
          }
        }

        if (place_success) {
          std::string task_id = utils::new_task_id();
          std::ostringstream oss;
          oss << "{\"task_id\": " << utils::json_escape(task_id)
              << ", \"step_id\": " << step_id
              << ", \"to_bbox\": [" << step.to_bbox[0] << ", " << step.to_bbox[1] << ", "
              << step.to_bbox[2] << ", " << step.to_bbox[3] << "]"
              << ", \"status\": \"success\"}";
          utils::write_json(
              output_dir / ("step8_container_place_" + std::to_string(step_id) + ".json"),
              oss.str());
        } else {
          std::cerr << "容器放置失败，尝试精准放置回退" << std::endl;
        }
      }
    }

    if (!place_success) {
      if (try_place(place_data)) {
        place_success = true;
        std::string task_id = utils::new_task_id();
        std::ostringstream oss;
        oss << "{\"task_id\": " << utils::json_escape(task_id)
            << ", \"frame_id\": " << utils::json_escape(to_pose->frame_id)
            << ", \"position\": {\"x\": " << to_pose->position.x
            << ", \"y\": " << to_pose->position.y << ", \"z\": " << to_pose->position.z << "}"
            << ", \"orientation\": {\"x\": " << to_pose->orientation.x
            << ", \"y\": " << to_pose->orientation.y << ", \"z\": " << to_pose->orientation.z
            << ", \"w\": " << to_pose->orientation.w << "}"
            << ", \"box_length\": {\"x\": " << to_pose->box_length.x
            << ", \"y\": " << to_pose->box_length.y << ", \"z\": " << to_pose->box_length.z << "}"
            << ", \"status\": \"success\"}";
        std::filesystem::path place_path =
            step.is_container ? (output_dir / ("step8_container_place_" + std::to_string(step_id) + ".json"))
                              : (output_dir / "step8_accurate_place.json");
        utils::write_json(place_path, oss.str());
      } else {
        std::cerr << "精准放置失败，尝试放回原位" << std::endl;
        robot_sdk::AccuratePlaceData restore = place_data;
        restore.position = from_pose->position;
        restore.orientation = from_pose->orientation;
        restore.box_length = from_pose->box_length;
        if (!try_place(restore)) {
          std::cerr << ErrorTag(kErrFallbackPlaceFailed) << " 放回原位失败" << std::endl;
          return false;
        }
      }
    }
  }

  std::cout << "物品 " << step_id << " 处理完成" << std::endl;
  return true;
}

bool run_execution_workflow(const robot_sdk::ImageQueryNotifyData& head_view_frozen,
                              const std::vector<IntentStep>& intent_steps,
                              const std::filesystem::path& output_dir,
                              bool stop_before_grab,
                              int max_attempts,
                              double retry_delay_sec) {
  std::cout << "============================================================" << std::endl;
  std::cout << "开始执行工作流 (Execution Workflow)" << std::endl;
  std::cout << "============================================================" << std::endl;

  std::vector<robot_sdk::BboxItem> bboxes;
  std::map<std::string, int> bbox_id_map;
  int bbox_id = 1;

  for (const auto& step : intent_steps) {
    int sid = step.step_id;
    if (!step.from_bbox.empty() && step.from_bbox.size() == 4) {
      robot_sdk::BboxItem bb;
      bb.id = bbox_id;
      bb.left_up_x = static_cast<float>(step.from_bbox[0]);
      bb.left_up_y = static_cast<float>(step.from_bbox[1]);
      bb.right_down_x = static_cast<float>(step.from_bbox[2]);
      bb.right_down_y = static_cast<float>(step.from_bbox[3]);
      bboxes.push_back(bb);
      bbox_id_map[std::to_string(sid) + "_from"] = bbox_id;
      ++bbox_id;
    }
    if (!step.to_bbox.empty() && step.to_bbox.size() == 4) {
      robot_sdk::BboxItem bb;
      bb.id = bbox_id;
      bb.left_up_x = static_cast<float>(step.to_bbox[0]);
      bb.left_up_y = static_cast<float>(step.to_bbox[1]);
      bb.right_down_x = static_cast<float>(step.to_bbox[2]);
      bb.right_down_y = static_cast<float>(step.to_bbox[3]);
      bboxes.push_back(bb);
      bbox_id_map[std::to_string(sid) + "_to"] = bbox_id;
      ++bbox_id;
    }
  }

  std::ostringstream map_debug;
  map_debug << "bbox_id映射: " << utils::bbox_id_map_to_json(bbox_id_map);
  std::cout << map_debug.str() << std::endl;

  utils::write_json(output_dir / "step5_bbox_id_map.json", utils::bbox_id_map_to_json(bbox_id_map));

  std::cout << "共构建 " << bboxes.size() << " 个 bbox" << std::endl;

  std::cout << "--- 计算6D姿态 ---" << std::endl;
  std::vector<robot_sdk::PoseResultItem> pose_results;
  bool ok = get_pose_from_bboxes(head_view_frozen, bboxes, pose_results, output_dir, max_attempts,
                                retry_delay_sec);
  if (!ok || pose_results.empty()) {
    std::cerr << ErrorTag(kErrPoseFailed) << " 6D姿态计算失败" << std::endl;
    return false;
  }

  std::cout << "--- 循环处理每个物品 ---" << std::endl;
  for (size_t i = 0; i < intent_steps.size(); ++i) {
    std::cout << "处理物品 " << (i + 1) << "/" << intent_steps.size() << std::endl;
    ok = process_item(intent_steps[i], head_view_frozen, pose_results, bbox_id_map, output_dir,
                      stop_before_grab, max_attempts, retry_delay_sec);
    if (!ok) std::cerr << "物品 " << (i + 1) << " 处理失败，继续下一个" << std::endl;
    if (stop_before_grab) return true;
  }

  std::cout << "执行工作流完成" << std::endl;
  return true;
}

// ============================================================
// 评判 eco_DeskIntentJudgeActionSimple
// ============================================================

bool run_judge_workflow(const robot_sdk::ImageMsg& tidy_rgb,
                        const std::string& user_input,
                        const std::filesystem::path& output_dir,
                        bool* out_is_tidy,
                        std::string* out_reason,
                        double* out_score,
                        int max_attempts,
                        double retry_delay_sec) {
  std::cout << "==================================================" << std::endl;
  std::cout << "评判工作流 (eco_DeskIntentJudgeAction)" << std::endl;
  std::cout << "==================================================" << std::endl;

  std::string ui = utils::trim_ws(user_input);
  if (ui.empty()) ui = "桌面整理";

  robot_sdk::DeskIntentJudgeActionRequest req;
  req.tidy_image = utils::image_to_data_url(tidy_rgb);
  req.user_input = ui;

  bool is_tidy = false;
  std::string reason;
  double score = 0;
  constexpr int kJudgeMs = 150000;
  std::cout << "调用 eco_DeskIntentJudgeAction，user_input=" << ui
            << " timeout_ms=" << kJudgeMs << std::endl;
  try {
    std::tie(is_tidy, reason, score) = retry_run(
        [&]() -> std::tuple<bool, std::string, double> {
          bool tidy = false;
          std::string r;
          double sc = 0;
          std::string e;
          if (!robot_sdk::eco_DeskIntentJudgeActionSimple(req, tidy, r, sc, e, kJudgeMs))
            throw DeskDemoError(e);
          return {tidy, r, sc};
        },
        "eco_DeskIntentJudgeAction", max_attempts, retry_delay_sec);
  } catch (const std::exception& e) {
    std::cerr << ErrorTag(ClassifyErrorCode("eco_DeskIntentJudgeAction", e.what())) << " judge failed: " << e.what() << std::endl;
    return false;
  }

  std::string task_id = utils::new_task_id();
  std::ostringstream oss;
  oss << "{\"task_id\": " << utils::json_escape(task_id) << ", \"status\": \"success\", "
      << "\"is_tidy\": " << (is_tidy ? "true" : "false") << ", \"reason\": "
      << utils::json_escape(reason) << ", \"score\": " << score
      << ", \"judge_backend\": \"eco_DeskIntentJudgeAction\", \"vlm_user_input\": "
      << utils::json_escape(ui) << "}";
  const std::filesystem::path judge_json = output_dir / "judge_result.json";
  utils::write_json(judge_json, oss.str());

  std::cout << "评判完成，is_tidy=" << (is_tidy ? "true" : "false") << ", score=" << score
            << std::endl;
  std::cout << "评判原因: " << reason << std::endl;
  std::cout << "评判结果已保存到: " << judge_json << std::endl;

  if (out_is_tidy) *out_is_tidy = is_tidy;
  if (out_reason) *out_reason = reason;
  if (out_score) *out_score = score;
  return true;
}

// ============================================================
// 主工作流
// ============================================================

bool desk_workflow(const std::string& ws_url, const DeskConfig& cfg,
                   const std::filesystem::path& output_dir) {
  utils::ensure_dir(output_dir);

  std::cout << "============================================================" << std::endl;
  std::cout << "开始执行桌面整理工作流" << std::endl;
  std::cout << "  目标区域: " << cfg.area_name << std::endl;
  std::cout << "  用户输入: " << cfg.user_input << std::endl;
  std::cout << "  最大循环: " << cfg.max_loops << std::endl;
  std::cout << "  输出目录: " << output_dir << std::endl;
  {
    const char* host = std::getenv("ECO_ROBOT_HOST");
    std::cout << "  SDK默认主机: ECO_ROBOT_HOST="
              << (host ? host : "(unset，使用 SDK 默认 10.10.10.11)") << std::endl;
  }
  std::cout << "============================================================" << std::endl;

  robot_sdk::Client& client = robot_sdk::Client::GetInstance();

  // 须在 Connect 之前注册：握手阻塞期间库内才能响应取消（否则 Ctrl+C 仅打印提示仍会卡很久）。
  robot_sdk::eco_SetCancellationFlag(&g_interrupted);

  auto cleanup = [&]() {
    robot_sdk::eco_SetCancellationFlag(nullptr);
    client.Disconnect();
    robot_sdk::Client::CleanupInstance();
  };

  const int max_attempts = cfg.sdk_max_attempts;
  const double retry_delay_sec = static_cast<double>(cfg.sdk_retry_delay_sec);

  try {
    retry_run_void(
        [&]() {
          if (!client.Connect(ws_url)) throw DeskDemoError("websocket_connect failed");
        },
        "websocket_connect", max_attempts, retry_delay_sec);
  } catch (const std::exception& e) {
    if (g_interrupted.load())
      std::cerr << "连接已取消或被中断" << std::endl;
    else
      std::cerr << ErrorTag(kErrConnectFailed) << " connect failed: " << e.what() << std::endl;
    cleanup();
    return false;
  }

  try {
    std::cout << "步骤1: 重定位" << std::endl;
    speech::Speak(speech::kRelocating);
    retry_run_void(
        [&]() {
          robot_sdk::ErrorCode ec = robot_sdk::eco_MapRelocation(120000);
          if (is_cancelled(ec)) throw DeskDemoError("eco_MapRelocation cancelled");
          check_or_throw(ec, "eco_MapRelocation");
        },
        "eco_MapRelocation", max_attempts, retry_delay_sec);

    std::cout << "步骤2: 准备姿态" << std::endl;
    speech::Speak(speech::kPreparePose);
    try {
      retry_run_void(
          [&]() {
            robot_sdk::ErrorCode ec = robot_sdk::eco_RobotPreparePose(120000);
            if (is_cancelled(ec)) throw DeskDemoError("eco_RobotPreparePose cancelled");
            check_or_throw(ec, "eco_RobotPreparePose");
          },
          "eco_RobotPreparePose", max_attempts, retry_delay_sec);
    } catch (const std::exception& e) {
      std::cerr << ErrorTag(kErrPreparePoseFailed) << " 准备姿态失败（已重试）: " << e.what() << std::endl;
    }

    std::filesystem::path loop0 = output_dir / "loop_0";
    utils::ensure_dir(loop0);
    std::cout << "步骤3: 感知1 (初始) -> " << loop0 << std::endl;

    PerceptionOutcome perc0;
    run_perception_workflow(perc0, cfg.area_name, cfg.area_id, cfg.img_w, cfg.img_h, cfg, loop0);

    std::optional<float> cached_head_angle_rad = perc0.applied_head_angle_rad;
    robot_sdk::ImageQueryNotifyData head_view_frozen = perc0.head_view;
    robot_sdk::ImageMsg rgb_plan = perc0.head_view.rgb_image;
    std::vector<robot_sdk::DeskIntentPerceptionObject> perception = perc0.perception_objects;

    std::string judge_input;
    bool is_tidy = false;
    int current_loop = 0;

    while (current_loop < cfg.max_loops) {
      ++current_loop;
      std::filesystem::path loop_dir = output_dir / ("loop_" + std::to_string(current_loop - 1));
      utils::ensure_dir(loop_dir);

      std::cout << "======== 第 " << current_loop << "/" << cfg.max_loops << " 轮 -> " << loop_dir
                << " ========" << std::endl;

      speech::Speak(speech::kPlanning);
      std::vector<IntentStep> intent_steps;
      if (!desk_intent_plan(rgb_plan, cfg.user_input, perception, judge_input, cfg.memory_input,
                            cfg.grab_items_path.empty()
                                ? std::filesystem::path{}
                                : std::filesystem::path(cfg.grab_items_path),
                            intent_steps, loop_dir, max_attempts, retry_delay_sec))
        throw DeskDemoError(ErrorTag(kErrPlanFailed) + " 意图规划失败");
      if (intent_steps.empty()) throw DeskDemoError(ErrorTag(kErrPlanFailed) + " 没有需要处理的物品");

      speech::Speak(speech::kExecution);
      if (!run_execution_workflow(head_view_frozen, intent_steps, loop_dir, cfg.stop_before_grab,
                                  max_attempts, retry_delay_sec))
        throw DeskDemoError(ErrorTag(kErrExecutionFailed) + " 执行工作流失败");

      if (cfg.stop_before_grab) {
        cleanup();
        return true;
      }

      arm_home_after_execution(max_attempts, retry_delay_sec);

      std::filesystem::path next_loop = output_dir / ("loop_" + std::to_string(current_loop));
      utils::ensure_dir(next_loop);
      std::cout << "步骤6: 感知2 -> " << next_loop << std::endl;
      PerceptionOutcome perc_after;
      run_perception_workflow(perc_after, cfg.area_name, cfg.area_id, cfg.img_w, cfg.img_h, cfg,
                              next_loop, cached_head_angle_rad);

      std::cout << "==================================================" << std::endl;
      std::cout << "步骤7: 评判 (第 " << current_loop << " 轮)" << std::endl;
      std::cout << "==================================================" << std::endl;
      speech::Speak(speech::kJudging);
      std::string reason;
      double score = 0;
      if (!run_judge_workflow(perc_after.head_view.rgb_image, cfg.user_input, loop_dir, &is_tidy,
                              &reason, &score, max_attempts, retry_delay_sec))
        throw DeskDemoError(ErrorTag(kErrJudgeFailed) + " 评判失败");

      if (is_tidy) {
        std::cout << "==================================================" << std::endl;
        std::cout << "评判通过 (第 " << current_loop << " 轮)" << std::endl;
        std::cout << "  score: " << score << std::endl;
        std::cout << "  reason: " << reason << std::endl;
        std::cout << "==================================================" << std::endl;
        break;
      }

      judge_input = reason;
      std::cout << "==================================================" << std::endl;
      std::cout << "评判未通过 (第 " << current_loop << " 轮)" << std::endl;
      std::cout << "  score: " << score << std::endl;
      std::cout << "  reason: " << reason << std::endl;
      std::cout << "==================================================" << std::endl;

      if (current_loop >= cfg.max_loops) {
        std::cout << "已达到最大循环次数，退出循环" << std::endl;
        break;
      }

      rgb_plan = perc_after.head_view.rgb_image;
      perception = perc_after.perception_objects;
      head_view_frozen = perc_after.head_view;
      std::cout << "下一轮将使用感知2的 " << perception.size()
                << " 个物体结果重新规划，并同步更新 head_view_frozen" << std::endl;
    }

    std::cout << "步骤8: 结束姿态" << std::endl;
    try {
      retry_run_void(
          [&]() {
            robot_sdk::ErrorCode ec = robot_sdk::eco_RobotEndingPose(120000);
            if (is_cancelled(ec)) throw DeskDemoError("eco_RobotEndingPose cancelled");
            check_or_throw(ec, "eco_RobotEndingPose");
          },
          "eco_RobotEndingPose", max_attempts, retry_delay_sec);
    } catch (const std::exception& e) {
      std::cerr << ErrorTag(kErrFinishPoseFailed) << " 结束姿态失败（已重试）: " << e.what() << std::endl;
    }

    speech::Speak(speech::kDone);

    {
      const std::filesystem::path summary_json = output_dir / "workflow_summary.json";
      std::ostringstream w;
      w << "{\"status\": \"success\", \"area_name\": " << utils::json_escape(cfg.area_name)
        << ", \"user_input\": " << utils::json_escape(cfg.user_input)
        << ", \"final_is_tidy\": " << (is_tidy ? "true" : "false") << ", \"loops_run\": "
        << current_loop << "}";
      utils::write_json(summary_json, w.str());

      std::cout << "============================================================" << std::endl;
      std::cout << "桌面整理工作流执行完毕" << std::endl;
      std::cout << "  总轮数: " << current_loop << std::endl;
      std::cout << "  评判结果: " << (is_tidy ? "通过" : "未通过 (已达最大重试)") << std::endl;
      std::cout << "  汇总文件: " << summary_json << std::endl;
      std::cout << "============================================================" << std::endl;
    }

    cleanup();
    return true;

  } catch (const DeskDemoError& e) {
    std::cerr << "流程失败: " << e.what() << std::endl;
    cleanup();
    return false;
  }
}

// ============================================================
// CLI 入口
// ============================================================

void print_usage(const char* prog) {
  std::cout << "用法: " << prog
            << " [--ws-url URL] [--area-name NAME] [--area-id ID] [--user-input TEXT] "
            << "[--stop-before-grab] [--connect-timeout-sec T]\n"
            << "         [--max-loops N] [--no-head-angle-suggest] [--head-sweep-rad R1,R2,...]\n"
            << "         [--head-sweep-sleep SEC] [--memory-input TEXT] [--grab-items-path PATH]\n"
            << "         [--output-dir DIR]\n";
  std::cout << "  --ws-url                 WebSocket URL\n";
  std::cout << "  --area-name             区域名 (默认: 书桌)\n";
  std::cout << "  --area-id               区域 ID\n";
  std::cout << "  --user-input            意图描述\n";
  std::cout << "  --stop-before-grab      抓取前停止\n";
  std::cout << "  --connect-timeout-sec   连接超时 (默认 60)\n";
  std::cout << "  --max-loops             规划-执行-评判循环 (默认: 3)" << std::endl;
  std::cout << "  --no-head-angle-suggest 跳过多角度选角 (DeskIntentPrePerception)" << std::endl;
  std::cout << "  --head-sweep-rad        选角采样弧度，逗号分隔，至少 2 个数" << std::endl;
  std::cout << "  --head-sweep-sleep      转头/拍照间隔秒 (默认: 0.3)" << std::endl;
  std::cout << "  --memory-input          记忆经验（可抓清单由 grab_items.md 自动拼接）" << std::endl;
  std::cout << "  --grab-items-path       可抓清单 md（默认 desk_demo/grab_items.md）" << std::endl;
  std::cout << "  --output-dir            输出根目录 (默认: outputs)" << std::endl;
  std::cout << "  -h, --help              显示帮助" << std::endl;
}

int main(int argc, char* argv[]) {
  DeskConfig cfg;
  std::string ws_url;
  std::filesystem::path output_root = "outputs";

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      print_usage(argv[0]);
      return 0;
    } else if (arg == "--ws-url" && i + 1 < argc) {
      ws_url = argv[++i];
    } else if (arg == "--area-name" && i + 1 < argc) {
      cfg.area_name = argv[++i];
    } else if (arg == "--area-id" && i + 1 < argc) {
      cfg.area_id = argv[++i];
    } else if (arg == "--user-input" && i + 1 < argc) {
      cfg.user_input = argv[++i];
    } else if (arg == "--stop-before-grab") {
      cfg.stop_before_grab = true;
    } else if (arg == "--connect-timeout-sec" && i + 1 < argc) {
      cfg.connect_timeout_sec = static_cast<float>(std::atof(argv[++i]));
    } else if (arg == "--max-loops" && i + 1 < argc) {
      cfg.max_loops = std::max(1, std::atoi(argv[++i]));
    } else if (arg == "--no-head-angle-suggest") {
      cfg.enable_vlm_head_angle_suggest = false;
    } else if (arg == "--head-sweep-sleep" && i + 1 < argc) {
      cfg.head_sweep_sleep_sec = static_cast<float>(std::atof(argv[++i]));
    } else if (arg == "--memory-input" && i + 1 < argc) {
      cfg.memory_input = argv[++i];
    } else if (arg == "--grab-items-path" && i + 1 < argc) {
      cfg.grab_items_path = argv[++i];
    } else if (arg == "--head-sweep-rad" && i + 1 < argc) {
      std::string raw = argv[++i];
      cfg.head_sweep_angles_rad.clear();
      std::istringstream rs(raw);
      std::string part;
      while (std::getline(rs, part, ',')) {
        part = utils::trim_ws(part);
        if (part.empty()) continue;
        cfg.head_sweep_angles_rad.push_back(static_cast<float>(std::strtod(part.c_str(), nullptr)));
      }
      if (cfg.head_sweep_angles_rad.size() < 2) {
        std::cerr << "--head-sweep-rad 至少需要 2 个弧度值" << std::endl;
        return 1;
      }
    } else if (arg == "--output-dir" && i + 1 < argc) {
      output_root = argv[++i];
    } else {
      std::cerr << "未知参数: " << arg << std::endl;
      return 1;
    }
  }

  if (ws_url.empty()) {
    ws_url = robot_sdk::eco_GetDefaultLinkUrl();
  }

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  std::cout << "=== 桌面整理C++演示程序（robot_sdk 共享库） ===" << std::endl;
  std::cout << "连接地址: " << ws_url << std::endl;
  std::cout << "目标区域: " << cfg.area_name << std::endl;
  std::cout << "按 Ctrl+C 退出程序" << std::endl;

  SetEcoRobotHostFromLinkUrl(ws_url);

  bool success = desk_workflow(ws_url, cfg, output_root);

  if (success) {
    std::cout << "桌面整理演示程序完成!" << std::endl;
    return 0;
  }
  std::cerr << "桌面整理演示程序失败!" << std::endl;
  return 1;
}
