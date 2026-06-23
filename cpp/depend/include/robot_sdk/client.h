#ifndef ROBOT_SDK_CLIENT_H_
#define ROBOT_SDK_CLIENT_H_

#include "export.h"
#include <string>

namespace robot_sdk {

/**
 * 客户端单例：仅管理连接生命周期与连接状态。
 * 底层读写、协议解析与任务通道在库内实现，使用机器人技能前须先 Connect 成功。
 */
class ROBOT_SDK_PUBLIC Client {
 public:
  /** 获取全局唯一 Client 实例。 */
  static Client& GetInstance();

  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;
  Client(Client&&) = delete;
  Client& operator=(Client&&) = delete;

  /**
   * 连接服务端。
   * @param url 连接端点地址（格式与实现约定一致）。
   * @return 是否握手成功。
   */
  bool Connect(const std::string& url);

  /** 断开连接并停止读线程。 */
  void Disconnect();

  /** 当前是否处于已连接状态。 */
  bool IsConnected() const;

  /**
   * 释放内部全局单例（可选）。用于单测或进程退出前显式析构，一般业务可不调。
   */
  static void CleanupInstance();

 private:
  Client();
  ~Client();
};

/**
 * 默认连接 URL，供未显式传入地址时使用。
 * 由环境变量 ECO_ROBOT_HOST（默认 10.10.10.11）与固定端口 9900 拼接。
 */
ROBOT_SDK_PUBLIC std::string eco_GetDefaultLinkUrl();

}  // namespace robot_sdk

#endif
