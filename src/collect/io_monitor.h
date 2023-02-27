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
    const SystemIoInfo& get_io_stats_info() {
        return io_stats_;
    }

private:
    void get_task_io_info(uint64_t collect_tm_ns);

    void handle_every_task_io_info(std::shared_ptr<TaskIoInfo> task_io_info, uint64_t collect_tm_ns);

    void set_task_state_overdue();
    void remove_overdue_task();

private:
    // 通过 netlink 获取 IO 的类对象
    NetlinkIoStats netlink_io_obj_;
    // 保存之前收集时间
    uint64_t origin_collect_timestamp_ns_ = 0;
    // 任务对应的 IO 信息，如果是进程，则 key 为 pid；如果是线程，则 key 为 tid
    SystemIoInfo io_stats_;
};

}  // namespace iotop_cpp
