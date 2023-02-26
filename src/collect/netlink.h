// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <map>
#include <list>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <string>
#include <mutex>
#include "common/common.h"

namespace iotop_cpp {

// netlink 消息报文长度
#define MAX_NETLINK_MSG_SIZE 1024
// 以下定义是 netlink 相关 payload 的转换操作
#define GENLMSG_DATA(glh)  (reinterpret_cast<void*>(reinterpret_cast<char*>(NLMSG_DATA(glh)) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh)  (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na)  (reinterpret_cast<void*>(reinterpret_cast<char*>(na) + NLA_HDRLEN))
#define NLA_PAYLOAD(len)  (len - NLA_HDRLEN)
// 计算时间差值，单位转换为秒
#define TIME_DIFF_S(start, end) ((((start) == (end)) || (start) == 0)?0.0001:(((end)-(start))/1000.0))

// 通过 netlink 发送消息的模版
struct NetlinkMsgTemplate {
    struct nlmsghdr nl_msg;
    struct genlmsghdr genl_msg;
    char buf[MAX_NETLINK_MSG_SIZE];
};

// 收集系统的 IO 信息
class NetlinkIoStats {
public:
     // 构造函数私有化
    NetlinkIoStats() = default;
    ~NetlinkIoStats() = default;
    NetlinkIoStats(const NetlinkIoStats&) = delete;
    NetlinkIoStats& operator=(const NetlinkIoStats&) = delete;
    NetlinkIoStats(const NetlinkIoStats&&) = delete;
    NetlinkIoStats& operator=(NetlinkIoStats&&) = delete;

    // 初始化函数
    int init();
    // 获取任务的 IO 信息
    int get_task_io_info(uint64_t task_id, std::shared_ptr<TaskIoInfo> io_stats);

private:
    // 初始化 sock fd
    int init_sock();
    // 初始化 netlink 的 family id
    int init_netlink_family_id();
    // 发送 netlink 的命令字给内核
    int send_netlink_cmd(uint16_t nl_msg_type, uint32_t nl_msg_pid, uint8_t genl_cmd,
            uint16_t nla_type, void *nla_data, int nla_len);

private:
    int sock_fd_ = -1;
    int netlink_family_id_ = -1;
};

}  // namespace iotop_cpp
