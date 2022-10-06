// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <stdint.h>
#include <pthread.h>
#include <memory>
#include <string>
#include "src/netlink.h"

namespace iotop_cpp {

// 存储 IO 状态的结构体
struct IoStats {
    pid_t pid;
    pid_t tid;
    uint64_t swapin_delay_total;  // ns
    uint64_t blkio_delay_total;  // ns
    uint64_t read_bytes;
    uint64_t write_bytes;
    int io_priority;  // io 的优先级
    int user_id;  // 用户 Id
    std::string short_cmdline;  // 短的 cmdline
    std::string full_cmdline;  // 完整的 cmdline

   //  bool is_valid_value;  // 此结构体是否为合法值
};

class IoInfo {
 public:
    IoInfo() : netlink_() {}
    ~IoInfo() = default;
    IoInfo(const IoInfo&) = delete;
    IoInfo& operator=(const IoInfo&) = delete;
    IoInfo(IoInfo&&) = delete;
    IoInfo& operator=(IoInfo&&) = delete;

    int init();
    int get_io_stat(pid_t pid, pid_t tid);
 private:
    int set_io_basic_stat(pid_t task, std::shared_ptr<struct IoStats> stats);
    int set_io_rw_stat(pid_t pid, pid_t tid, std::shared_ptr<struct IoStats> stats);

 private:
    Netlink netlink_;
};

}  // namespace iotop_cpp
