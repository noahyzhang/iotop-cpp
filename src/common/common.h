// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <sys/types.h>
#include <stdint.h>
#include <string>

// 任务名字最大的长度
#define MAX_COMMAND_LENGTH 128
// 用户名字最大的长度
#define MAX_USER_NAME_LENGTH 128

struct TaskIoInfo {
    /* ---------- 任务的基本信息 -------------- */
    // 进程/线程的 ID
    pid_t task_id;
    // 如果是进程，则为 true，如果是线程，则为 false
    bool is_pid;
    // io 的优先级
    int io_priority;
    // 用户 Id
    int user_id;
    // 用户名
    char use_name[MAX_USER_NAME_LENGTH+1];
    // 短的 cmdline
    char short_cmdline[MAX_COMMAND_LENGTH+1];
    // 完整的 cmdline
    char full_cmdline[MAX_COMMAND_LENGTH+1];

    /* ---------- 任务的 IO 相关统计 -------------- */
    // 当前从系统中获取到的 IO 相关指标
    // 读 IO 的字节数，包含 pagecache
    uint64_t io_read_char_b;
    // 写 IO 的字节数，包含 pagecache
    uint64_t io_write_char_b;
    // 实际读磁盘的字节数
    uint64_t io_real_read_b;
    // 实际写磁盘的字节数
    uint64_t io_real_write_b;
    // 读 IO 操作的次数，例如：read、pread
    uint64_t io_read_syscalls;
    // 写 IO 操作的次数，例如：write、pwrite
    uint64_t io_write_syscalls;
    // 取消写的字节数
    uint64_t io_cancelled_write_b;

    uint64_t swapin_delay_total;  // ns
    uint64_t blkio_delay_total;  // ns

    /**
     * 一定时间间隔，不同指标的带宽值
     * 这里使用整数，而不是浮点数
     * 因为我们不需要小数点后的值，对于我们来说，微乎其微可以忽略。
     * 而且浮点数存储、计算都是很耗费 CPU 的，展示出来也不直观
     */

    // 读 IO 的速率（字节每秒）
    uint64_t io_read_char_period_b_s;
    // 写 IO 的速率（字节每秒）
    uint64_t io_write_char_period_b_s;
    // 实际读磁盘的速率（字节每秒）
    uint64_t io_real_read_period_b_s;
    // 实际写磁盘的速率（字节每秒）
    uint64_t io_real_write_period_b_s;
    // 读 IO 操作的速率（次每秒）
    uint64_t io_read_syscalls_period_s;
    // 写 IO 操作的速率（次每秒）
    uint64_t io_write_syscalls_period_s;
    // 取消写的速率（字节每秒）
    uint64_t io_cancelled_write_period_b_s;

    /* ---------- 任务的相关逻辑字段 -------------- */
    // 在当前周期是否被更新
    bool is_updated;
};

