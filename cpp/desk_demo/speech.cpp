#include "speech.hpp"

#include <robot_sdk/robot_sdk.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <thread>

namespace speech {
namespace {

constexpr bool kSpeechEnabled = false;

std::string trim_copy(std::string s) {
  auto not_space = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
  s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
  return s;
}

}  // namespace

void Speak(const std::string& text) {
  if (!kSpeechEnabled) {
    return;
  }

  const std::string t = trim_copy(text);
  if (t.empty()) {
    return;
  }

  std::cout << "语音播报: " << t << std::endl;
  std::thread([t]() {
    const robot_sdk::ErrorCode ec = robot_sdk::eco_Speech(t);
    if (ec.code != 0) {
      std::cerr << "语音播报失败 (" << t << "): code=" << ec.code << ", msg=" << ec.msg
                << std::endl;
    }
  }).detach();
}

}  // namespace speech
