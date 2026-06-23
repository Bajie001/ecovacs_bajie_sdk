#ifndef DEMO_SDK_RESPONSE_HELPERS_H_
#define DEMO_SDK_RESPONSE_HELPERS_H_

#include <robot_sdk/robot_sdk.h>

#include <string>
#include <utility>

namespace demo_sdk {

inline std::string FormatImagePayloadSize(const robot_sdk::ImageMsg& image) {
  return "payload_bytes=" + std::to_string(image.data.size());
}

inline std::string FormatSdkError(const std::string& api_name, int code,
                                  const std::string& msg) {
  std::string error = api_name + " failed: code=" + std::to_string(code);
  if (!msg.empty()) {
    error += ", msg=" + msg;
  }
  return error;
}

inline bool PutWhereWithError(const robot_sdk::PutWhereRequest& req,
                              std::string& final_answer,
                              std::string& error_msg,
                              int timeout_ms = robot_sdk::kDefaultHttpRequestTimeoutMs) {
  auto response = robot_sdk::eco_PutWhere(req, timeout_ms);
  if (!response) {
    error_msg = "eco_PutWhere failed: empty response";
    return false;
  }
  if (response->code != 0) {
    error_msg = FormatSdkError("eco_PutWhere", response->code, response->msg);
    return false;
  }

  final_answer = std::move(response->final_answer);
  error_msg.clear();
  return true;
}

}  // namespace demo_sdk

#endif  // DEMO_SDK_RESPONSE_HELPERS_H_
