// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <sys/types.h>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <memory>

// 任务名字最大的长度
#define MAX_COMMAND_LENGTH 128
// 用户名字最大的长度
#define MAX_USER_NAME_LENGTH 128

struct TaskIoInfo {
    /* ---------- 任务的基本信息 -------------- */
    // 进程/线程的 ID
    pid_t task_id{-1};
    // 如果是进程，则为 true，如果是线程，则为 false
    bool is_pid{false};
    // io 的优先级
    int io_priority{-1};
    // 用户 Id
    int user_id{0};
    // 用户名
    char user_name[MAX_USER_NAME_LENGTH+1]{0};
    // 短的 cmdline
    char short_cmdline[MAX_COMMAND_LENGTH+1]{0};
    // 完整的 cmdline
    char full_cmdline[MAX_COMMAND_LENGTH+1]{0};

    /* ---------- 任务的 IO 相关统计 -------------- */
    // 当前从系统中获取到的 IO 相关指标
    // 读 IO 的字节数，包含 pagecache
    uint64_t io_read_char_b{0};
    // 写 IO 的字节数，包含 pagecache
    uint64_t io_write_char_b{0};
    // 实际读磁盘的字节数
    uint64_t io_real_read_b{0};
    // 实际写磁盘的字节数
    uint64_t io_real_write_b{0};
    // Delay waiting for page fault I/O (swap in only)
    uint64_t swapin_delay_total_ns{0};
    // Delay waiting for synchronous block I/O to complete
    // does not account for delays in I/O submission
    uint64_t blkio_delay_total_ns{0};

    /**
     * 一定时间间隔，不同指标的带宽值
     * 这里使用整数，而不是浮点数
     * 因为我们不需要小数点后的值，对于我们来说，微乎其微可以忽略。
     * 而且浮点数存储、计算都是很耗费 CPU 的，展示出来也不直观
     */

    // 读 IO 的速率（字节每秒）
    double io_read_char_period_b_s{0.0};
    // 写 IO 的速率（字节每秒）
    double io_write_char_period_b_s{0.0};
    // 实际读磁盘的速率（字节每秒）
    double io_real_read_period_b_s{0.0};
    // 实际写磁盘的速率（字节每秒）
    double io_real_write_period_b_s{0.0};
    double swapin_delay_period_percent{0.0};
    double blkio_delay_period_percent{0.0};

    /* ---------- 任务的相关逻辑字段 -------------- */
    // 在当前周期是否被更新
    bool is_updated{false};
};

struct SystemIoInfo {
    double curr_disk_read_b_s{0};
    double curr_disk_write_b_s{0};
    // 任务对应的 IO 信息，如果是进程，则 key 为 pid；如果是线程，则 key 为 tid
    std::unordered_map<uint64_t, std::shared_ptr<TaskIoInfo>> task_io_stats_table;
};
