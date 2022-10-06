// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <pthread.h>
#include <memory>
#include <string>

namespace iotop_cpp {

class Util {
 public:
    // 获取任务（进程/线程）的 IO 优先级
    static int get_task_io_priority(pid_t task, std::shared_ptr<int> io_priority);

    inline static int io_prio_to_class(int io_prio);
    inline static int io_prio_to_prio(int io_prio);

    static int get_task_cmdline(pid_t pid, bool is_short, std::shared_ptr<std::string> cmdline);

    // 关于系统的延迟记账信息，参考：https://www.kernel.org/doc/Documentation/accounting/delay-accounting.rst
    // 获取系统的延迟记账信息
    static int read_task_delayacct(std::shared_ptr<int> delayacct);
    // 设置系统的延迟记账信息
    static int write_task_delayacct(int delayacct);

    // 判断此 task 是否为进程
    static bool is_process(pid_t task);

 private:
    // 通过系统调度器来获取任务（进程/线程） IO 优先级
    static int get_task_io_priority_from_sched(pid_t task, std::shared_ptr<int> io_priority);
    // 获取给定路径文件的第一行
    static int get_file_first_line(const std::string& path, std::shared_ptr<std::string> line);
};

}  // namespace iotop_cpp
