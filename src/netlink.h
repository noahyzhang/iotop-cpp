// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <stdint.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>

namespace iotop_cpp {

// 发送消息的模版
struct MsgTemplate {
    struct nlmsghdr nl_msg;
    struct genlmsghdr genl_msg;
};

// 封装 netlink socket 的相关操作
class Netlink {
 public:
    Netlink() = default;
    ~Netlink() {
       if (sock_fd_ > -1) {
          close(sock_fd_);
       }
    }
    Netlink(const Netlink&) = delete;
    Netlink& operator=(const Netlink&) = delete;
    Netlink(Netlink&&) = delete;
    Netlink& operator=(Netlink&&) = delete;

    // 使用此类，先调用 init 方法进行初始化
    int init();
    int send_cmd(uint16_t nl_msg_type, uint32_t nl_msg_pid,
      uint8_t genl_cmd, uint16_t nla_type, void* nla_data, int nla_len);
    inline int get_sock_fd() {
       return sock_fd_;
    }
    inline int get_netlink_family_id() {
       return netlink_family_id_;
    }

 private:
    int init_sock();
    int init_netlink_family_id();

 private:
    int sock_fd_ = -1;
    int netlink_family_id_ = -1;
};

}  // namespace iotop_cpp
