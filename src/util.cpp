// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <unistd.h>
#include <syscall.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <fcntl.h>
#include <easylogging++.h>
#include "src/util.h"

namespace iotop_cpp {

// copy from ioprio.h
#define IOPRIO_CLASS_SHIFT  (13)
#define IOPRIO_PRIO_VALUE(class, data)  (((class) << IOPRIO_CLASS_SHIFT) | data)

// copy from <sys/syscalls.h>
enum {
    IOPRIO_WHO_PROCESS = 1,
    IOPRIO_WHO_PGRP,
    IOPRIO_WHO_USER,
};

// io priority
// reference from https://linux.die.net/man/2/ioprio_set
enum {
    IOPRIO_CLASS_NONE,
    IOPRIO_CLASS_RT,
    IOPRIO_CLASS_BE,
    IOPRIO_CLASS_IDLE,
    IOPRIO_CLASS_MAX,
    IOPRIO_CLASS_MIN = IOPRIO_CLASS_RT,
};

int Util::get_task_io_priority_from_sched(pid_t pid, std::shared_ptr<int> io_priority) {
    int scheduler = sched_getscheduler(pid);
    int nice = getpriority(PRIO_PROCESS, pid);
    int io_prio_nice = (nice + 20) / 5;
    if (scheduler == SCHED_FIFO || scheduler == SCHED_RR) {
        io_priority.reset((IOPRIO_CLASS_RT << IOPRIO_CLASS_SHIFT) + io_prio_nice);
        return 0;
    }
    if (scheduler == SCHED_IDLE) {
        io_priority.reset(IOPRIO_CLASS_IDLE << IOPRIO_CLASS_SHIFT);
        return 0;
    }
    io_priority.reset((IOPRIO_CLASS_BE << IOPRIO_CLASS_SHIFT) + io_prio_nice);
    return 0;
}

int Util::get_task_io_priority(pid_t task, std::shared_ptr<int> io_priority) {
    // 获取进程的 IO 调度的优先级
    int io_prio = syscall(SYS_ioprio_get, IOPRIO_WHO_PROCESS, task);
    if (io_prio < 0) {
        LOG(ERROR) << "syscall SYS_ioprio_get failed";
        return -1;
    }
    int io_class = io_prio >> IOPRIO_CLASS_SHIFT;
    if (!io_class) {
        return get_io_priority_from_sched(task, io_priority);
    }
    return 0;
}

int Util::get_task_cmdline(pid_t pid, bool is_short, std::shared_ptr<std::string> cmdline) {
    // 从 /proc/xx/cmdline 获取 cmdline
    char path[30];
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    auto res = get_file_first_line(path, cmdline);
    if (res < 0) {
        std::cerr << "get file: " << path << " first line failed, errno: " << res << std::endl;
        return -1;
    }
    // 处理需要短的 cmdline 的情况
    size_t line_size = cmdline->size();
    if (line_size > 0) {
        // 如果需要一个短的 cmdline
        if (is_short && ( (cmdline[0] == '/') || (line_size > 1 && cmdline[0] == '.' && cmdline[1] == '/')
            || (line_size > 2 && cmdline[0] == '.' && cmdline[1] == '.' && cmdline[2] == '/'))) {
            size_t found = cmdline->find_first_of('/');
            if (found != std::string::npos && found+1 < line_size) {
                cmdline = cmdline->substr(found+1);
            }
        }
        return 0;
    }
    // 如果从 /proc/xx/cmdline 中未能获取，还可以从 /proc/xx/status 中获取
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    res = get_file_first_line(path, cmdline);
    if (res < 0) {
        std::cerr << "get file: " << path << " first line failed, errno: " << res << std::endl;
        return -2;
    }
    line_size = cmdline->size();
    if (line_size > 0) {
        size_t found = cmdline->find_first_of('\t');
        if (found != std::string::npos && found+1 < line_size) {
            cmdline = cmdline->substr(found+1);
        }
        return 0;
    }
    return -3;
}

int Util::read_task_delayacct(std::shared_ptr<int> delayacct) {
    const char* path = "/proc/sys/kernel/task_delayacct";
    auto line = std::make_shared<std::string>();
    auto res = get_file_first_line(path, line);
    if (res < 0) {
        std::cerr << "get file: " << path << " first line failed, errno: " << res << std::endl;
        return -1;
    }
    if (line->size() > 0) {
        delayacct.reset(std::atoi(line->c_str()));
        return 0;
    }
    return -2;
}

int Util::write_task_delayacct(int delayacct) {
    char* real_delayacct = delayacct ? "1\n" : "0\n";
    int fd = open("/proc/sys/kernel/task_delayacct", O_WRONLY);
    if (fd < 0) {
        std::cerr << "open /proc/sys/kernel/task_delayacct failed, err: " << strerror(errno) << std::endl;
        return -1;
    }
    size_t real_delayacct_size = strlen(real_delayacct);
    ssize_t write_size = write(fd, real_delayacct, real_delayacct_size);
    if (write_size < 0) {
        close(fd);
        std::cerr << "write /proc/sys/kernel/task_delayacct failed, err: " << strerror(errno) << std::endl;
        return -2;
    }
    if (write_size != real_delayacct_size) {
        close(fd);
        std::cerr << "write /proc/sys/kernel/task_delayacct failed, expected write: "
                  << real_delayacct << ", real write size: " << real_delayacct_size << std::endl;
        return -3;
    }
    close(fd);
    return 0;
}

bool Util::is_process(pid_t task) {
    char path[30];
    snprintf(path, sizeof(path), "/proc/%d", task);
    struct stat st;
    if (stat(path, &st) < 0) {
        return false;
    }
    return (st.st_mode & S_IFMT) == S_IFDIR;
}

int Util::get_file_first_line(const std::string& path, std::shared_ptr<std::string> line) {
    FILE* fp = fopen(path.c_str(), "r");
    if (fp == nullptr) {
        std::cerr << "fopen err: " << strerror(errno) << std::endl;
        return -1;
    }
    char* tmp_line = NULL;
    size_t len = 0;
    if ((getline(&tmp_line, &len, fp)) < 0) {
        std::cerr << "getline err: " << strerror(errno) << std::endl;
        if (tmp_line) {
            free(tmp_line);
        }
        fclose(fp);
        return -2;
    }
    fclose(fp);
    if (tmp_line && len > 0) {
        line.reset(tmp_line);
        return 0;
    }
    return -3;
}

}  // namespace iotop_cpp
