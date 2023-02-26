// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <unordered_map>
#include <memory>
#include "collect/netlink.h"

namespace iotop_cpp {

class IoMonitor {
public:
    IoMonitor() = default;
    ~IoMonitor() = default;

public:
    int init();
    int collect_timed();

private:
    void set_task_state_overdue();
    void remove_overdue_task();

    void get_task_io_info();
    

private:
    // 通过 netlink 获取 IO 的类对象
    NetlinkIoStats netlink_io_obj_;
    // 任务对应的 IO 信息，如果是进程，则 key 为 pid；如果是线程，则 key 为 tid
    std::unordered_map<uint64_t, std::shared_ptr<TaskIoInfo>> io_stats_table_;

};

}  // namespace iotop_cpp
