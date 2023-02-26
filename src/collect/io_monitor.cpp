// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <string.h>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include "common/util.h"
#include "collect/io_monitor.h"

namespace iotop_cpp {

int IoMonitor::init() {
    if (netlink_io_obj_.init() < 0) {
        LOG(ERROR) << "IoMonitor::init netlink_io_obj init failed";
        return -1;
    }
    return 0;
}

int IoMonitor::collect_timed() {
    // 先设置所有任务是过期的
    set_task_state_overdue();
    // 获取所有任务的 IO 信息

    // 移除过期的任务
    remove_overdue_task();
    return 0;
}

void IoMonitor::get_task_io_info() {
    auto task_ids = Util::get_all_task_id();
    for (const auto& task_id : task_ids) {
        auto task_io_info = std::make_shared<TaskIoInfo>();
        task_io_info->task_id = task_id.first;
        task_io_info->is_pid = true;
        auto prio = Util::get_task_io_priority(task_id.first);
        task_io_info->io_priority = prio < 0 ? 0 : prio;

        auto res = netlink_io_obj_.get_task_io_info(task_id.first, task_io_info);
        if (__glibc_unlikely(res < 0)) {
            LOG(ERROR) << "IoMonitor::get_task_io_info get process: " << task_id.first << " io info failed";
            continue;
        }
        snprintf(task_io_info->use_name, MAX_USER_NAME_LENGTH, Util::get_user_name(task_io_info->user_id).c_str());
        auto cmd = std::make_shared<std::string>();
        if (Util::get_task_cmdline(task_id.first, true, cmd) >= 0) {
            snprintf(task_io_info->short_cmdline, MAX_COMMAND_LENGTH, cmd->c_str());
        }
        cmd->clear();
        if (Util::get_task_cmdline(task_id.first, false, cmd) >= 0) {
            snprintf(task_io_info->full_cmdline, MAX_COMMAND_LENGTH, cmd->c_str());
        }

        io_stats_table_.emplace(task_id.first, task_io_info);
        // 获取线程的 IO 信息
        for (const auto& tid : task_id.second) {

        }
    }
}

void 

}  // namespace iotop_cpp
