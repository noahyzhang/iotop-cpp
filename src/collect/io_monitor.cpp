// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <string.h>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include "common/util.h"
#include "collect/io_monitor.h"

namespace iotop_cpp {

int IoMonitor::init() {
    if (netlink_io_obj_.init() < 0) {
        std::cerr << "IoMonitor::init netlink_io_obj init failed";
        return -1;
    }
    return 0;
}

int IoMonitor::collect_timed() {
    // 先设置所有任务是过期的
    set_task_state_overdue();
    io_stats_.curr_disk_read_b_s = 0;
    io_stats_.curr_disk_write_b_s = 0;
    // 获取所有任务的 IO 信息
    uint64_t collect_tm_ns = Util::get_steady_now_time_ns();
    get_task_io_info(collect_tm_ns);
    origin_collect_timestamp_ns_ = collect_tm_ns;
    // 移除过期的任务
    // remove_overdue_task();
    return 0;
}

void IoMonitor::get_task_io_info(uint64_t collect_tm_ns) {
    auto task_ids = Util::get_all_task_id();
    for (const auto& task_id : task_ids) {
        auto process_io_info = std::make_shared<TaskIoInfo>();
        process_io_info->task_id = task_id.first;
        process_io_info->is_pid = true;
        handle_every_task_io_info(process_io_info, collect_tm_ns);
        io_stats_.task_io_stats_table.emplace(task_id.first, process_io_info);
        io_stats_.curr_disk_read_b_s += process_io_info->io_real_read_period_b_s;
        io_stats_.curr_disk_write_b_s += process_io_info->io_real_write_period_b_s;
        // 获取线程的 IO 信息
        for (const auto& tid : task_id.second) {
            auto thread_io_info = std::make_shared<TaskIoInfo>();
            thread_io_info->task_id = tid;
            thread_io_info->is_pid = false;
            handle_every_task_io_info(thread_io_info, collect_tm_ns);
            io_stats_.task_io_stats_table.emplace(tid, thread_io_info);
        }
    }
}

void IoMonitor::handle_every_task_io_info(std::shared_ptr<TaskIoInfo> task_io_info, uint64_t collect_tm_ns) {
    pid_t task_id = task_io_info->task_id;
    // 填充任务的优先级
    auto prio = Util::get_task_io_priority(task_io_info->task_id);
    if (prio < 0) {
        task_io_info->io_priority = 0;
    } else {
        task_io_info->io_priority = prio;
    }
    // 填充任务的 IO 信息和 user_id
    uint64_t origin_read_char_b = task_io_info->io_read_char_b;
    uint64_t origin_write_char_b = task_io_info->io_write_char_b;
    uint64_t origin_real_read_b = task_io_info->io_real_read_b;
    uint64_t origin_real_write_b = task_io_info->io_real_write_b;
    uint64_t origin_swapin_delay_ns = task_io_info->swapin_delay_total_ns;
    uint64_t origin_blkio_delay_ns = task_io_info->blkio_delay_total_ns;
    auto res = netlink_io_obj_.get_task_io_info(task_id, task_io_info);
    if (__glibc_unlikely(res < 0)) {
        // std::cerr << "IoMonitor::get_task_io_info get process: " << task_id << " io info failed";
        return;
    }
    double tm_diff_s = static_cast<double>(collect_tm_ns - origin_collect_timestamp_ns_) / 1E9;
    // 对于 tm_diff_s 明显错误的情况，直接过滤。我们认为 diff 时间大于0，至少是正数
    if (tm_diff_s > 1E-3) {
        #define ULL_VALUE_DIFF(to, from) ( ((to) < (from)) ? (0UL) : ((to)-(from)) )
        task_io_info->io_read_char_period_b_s =
            static_cast<double>(ULL_VALUE_DIFF(task_io_info->io_read_char_b, origin_read_char_b)) / tm_diff_s;
        task_io_info->io_write_char_period_b_s =
            static_cast<double>(ULL_VALUE_DIFF(task_io_info->io_write_char_b, origin_write_char_b)) / tm_diff_s;
        task_io_info->io_real_read_period_b_s =
            static_cast<double>(ULL_VALUE_DIFF(task_io_info->io_real_read_b, origin_real_read_b)) / tm_diff_s;
        task_io_info->io_real_write_period_b_s =
            static_cast<double>(ULL_VALUE_DIFF(task_io_info->io_real_write_b, origin_real_write_b)) / tm_diff_s;
        task_io_info->io_read_char_period_b_s =
            static_cast<double>(ULL_VALUE_DIFF(task_io_info->io_read_char_b, origin_read_char_b)) / tm_diff_s;
        task_io_info->swapin_delay_period_percent =
            static_cast<double>(ULL_VALUE_DIFF(task_io_info->swapin_delay_total_ns, origin_swapin_delay_ns))
            / static_cast<double>(collect_tm_ns - origin_collect_timestamp_ns_) * 1E2;
        task_io_info->blkio_delay_period_percent =
            static_cast<double>(ULL_VALUE_DIFF(task_io_info->blkio_delay_total_ns, origin_blkio_delay_ns))
            / static_cast<double>(collect_tm_ns - origin_collect_timestamp_ns_) * 1E2;
        #undef ULL_VALUE_DIFF
    }
    // 填充任务的 user_name
    auto user_name = Util::get_user_name(task_io_info->user_id);
    strncpy(task_io_info->user_name, user_name.c_str(), user_name.size());
    // 填充任务的 cmdline
    auto cmd = std::make_shared<std::string>();
    if (Util::get_task_cmdline(task_id, true, cmd) >= 0) {
        strncpy(task_io_info->short_cmdline, cmd->c_str(), cmd->size());
    }
    cmd->clear();
    if (Util::get_task_cmdline(task_id, false, cmd) >= 0) {
        strncpy(task_io_info->full_cmdline, cmd->c_str(), cmd->size());
    }
    task_io_info->is_updated = true;
}

void IoMonitor::set_task_state_overdue() {
    for (const auto& x : io_stats_.task_io_stats_table) {
        x.second->is_updated = false;
    }
}

void IoMonitor::remove_overdue_task() {
    auto iter = io_stats_.task_io_stats_table.begin();
    for (; iter != io_stats_.task_io_stats_table.end(); ) {
        if (iter->second) {
            if (iter->second->is_updated == false) {
                iter->second.reset();
                io_stats_.task_io_stats_table.erase(iter);
            }
        } else {
            ++iter;
        }
    }
}

}  // namespace iotop_cpp
