// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <unistd.h>
#include <syscall.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <vector>
#include <sstream>
#include <iostream>
#include "common/util.h"

namespace iotop_cpp {

// copy from ioprio.h
#define IOPRIO_CLASS_SHIFT  (13)
#define IOPRIO_STR_MAXSIZ  (10)
#define IOPRIO_STR_FORMAT "%2s/%1i"

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

int Util::get_task_io_priority(pid_t task_id) {
    // 获取进程的 IO 调度的优先级
    int io_prio = syscall(SYS_ioprio_get, IOPRIO_WHO_PROCESS, task_id);
    if (io_prio < 0) {
        std::cerr << "syscall SYS_ioprio_get failed";
        return -1;
    }
    int io_class = io_prio >> IOPRIO_CLASS_SHIFT;
    if (!io_class) {
        return get_task_io_priority_from_sched(task_id);
    }
    return io_prio;
}

int Util::get_task_io_priority_from_sched(pid_t task_id) {
    int scheduler = sched_getscheduler(task_id);
    int nice = getpriority(PRIO_PROCESS, task_id);
    int io_prio_nice = (nice + 20) / 5;
    if (scheduler == SCHED_FIFO || scheduler == SCHED_RR) {
        return (IOPRIO_CLASS_RT << IOPRIO_CLASS_SHIFT) + io_prio_nice;
    }
    if (scheduler == SCHED_IDLE) {
        return IOPRIO_CLASS_IDLE << IOPRIO_CLASS_SHIFT;
    }
    return (IOPRIO_CLASS_BE << IOPRIO_CLASS_SHIFT) + io_prio_nice;
}

const char* Util::convert_io_prio_to_str(int io_prio) {
    static const char corrupted[]="xx/x";
    static char buf[IOPRIO_STR_MAXSIZ];
    int io_class = io_prio >> IOPRIO_CLASS_SHIFT;

    io_prio &= ((1 << IOPRIO_CLASS_SHIFT) - 1);
    if (io_class >= IOPRIO_CLASS_MAX) {
        return corrupted;
    }
    static const char* str_ioprio_class[] = {"-", "rt", "be", "id"};
    snprintf(buf, sizeof(buf), IOPRIO_STR_FORMAT, str_ioprio_class[io_class], io_prio);
    return (const char *)buf;
}

int Util::set_io_prio(int which, int who, int io_prio_class, int io_prio) {
    return syscall(SYS_ioprio_set, which, who, ((io_prio_class << IOPRIO_CLASS_SHIFT) | io_prio));
}

int Util::read_task_delayacct() {
    const char* path = "/proc/sys/kernel/task_delayacct";
    auto line = std::make_shared<std::string>();
    auto res = get_file_first_line(path, line);
    if (res < 0) {
        std::cerr << "get file: " << path << " first line failed, errno: " << res << std::endl;
        return 0;
    }
    if (line->size() > 0) {
        return std::atoi(line->c_str());
    }
    return 0;
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
        // std::cerr << "getline err: " << strerror(errno) << std::endl;
        if (tmp_line) {
            free(tmp_line);
        }
        fclose(fp);
        return -2;
    }
    fclose(fp);
    if (tmp_line && len > 0) {
        *line = tmp_line;
        return 0;
    }
    return -3;
}

// int Util::write_task_delayacct(int delayacct) {
//     char* real_delayacct = delayacct ? "1\n" : "0\n";
//     int fd = open("/proc/sys/kernel/task_delayacct", O_WRONLY);
//     if (fd < 0) {
//         std::cerr << "open /proc/sys/kernel/task_delayacct failed, err: " << strerror(errno) << std::endl;
//         return -1;
//     }
//     size_t real_delayacct_size = strlen(real_delayacct);
//     ssize_t write_size = write(fd, real_delayacct, real_delayacct_size);
//     if (write_size < 0) {
//         close(fd);
//         std::cerr << "write /proc/sys/kernel/task_delayacct failed, err: " << strerror(errno) << std::endl;
//         return -2;
//     }
//     if (write_size != real_delayacct_size) {
//         close(fd);
//         std::cerr << "write /proc/sys/kernel/task_delayacct failed, expected write: "
//                   << real_delayacct << ", real write size: " << real_delayacct_size << std::endl;
//         return -3;
//     }
//     close(fd);
//     return 0;
// }

int Util::get_task_cmdline(pid_t pid, bool is_short, std::shared_ptr<std::string> cmdline) {
    // 从 /proc/xx/cmdline 获取 cmdline
    char path[30];
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    auto res = get_file_first_line(path, cmdline);
    if (res >= 0) {
        // 处理需要短的 cmdline 的情况
        size_t line_size = cmdline->size();
        if (line_size > 0) {
            // 如果需要一个短的 cmdline
            if (is_short && ( (cmdline->at(0) == '/')
                || (line_size > 1 && cmdline->at(0) == '.' && cmdline->at(1) == '/')
                || (line_size > 2 && cmdline->at(0) == '.' && cmdline->at(1) == '.' && cmdline->at(2) == '/'))) {
                size_t found = cmdline->find_first_of('/');
                if (found != std::string::npos && found+1 < line_size) {
                    *cmdline = cmdline->substr(found+1);
                }
            }
            return 0;
        }
    }
    // 如果从 /proc/xx/cmdline 中未能获取，还可以从 /proc/xx/status 中获取
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    res = get_file_first_line(path, cmdline);
    if (res >= 0) {
        size_t line_size = cmdline->size();
        if (line_size > 0) {
            size_t found = cmdline->find_first_of('\t');
            if (found != std::string::npos && found+1 < line_size) {
                *cmdline = cmdline->substr(found+1);
            }
            return 0;
        }
    }
    return -1;
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

std::vector<std::pair<uint64_t, std::vector<uint64_t>>> Util::get_all_task_id(bool is_only_pid) {
    static std::vector<std::pair<uint64_t, std::vector<uint64_t>>> task_ids;
    task_ids.clear();
    // 遍历 /proc 文件系统
    int dir_fd = openat(AT_FDCWD, "/proc", O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    if (dir_fd < 0) {
        std::cerr << "openat dir: " << "/proc" << " failed, err: " << strerror(errno);
        return task_ids;
    }
    DIR* dir = fdopendir(dir_fd);
    if (!dir) {
        close(dir_fd);
        std::cerr << "fdopendir: " << dir_fd << " failed, err: " << strerror(errno);
        return task_ids;
    }
    // 读取 proc 文件系统中进程或线程的目录中的文件信息
    const struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过非目录
        if (entry->d_type != DT_DIR && entry->d_type != DT_UNKNOWN) {
            continue;
        }
        const char* name = entry->d_name;
        // RedHat 会使用点来隐藏线程，这里进行兼容
        if (name[0] == '.') {
            name++;
        }
        // 跳过名字为非数字的目录，任务的目录一定以数字（pid）命名
        if (name[0] < '0' || name[0] > '9') {
            continue;
        }
        // 文件的名字应该为一个数字，表明是进程的目录
        char* end_ptr = nullptr;
        uint64_t pid = strtoul(name, &end_ptr, 10);
        if (pid == 0 || pid == UINT64_MAX || *end_ptr != '\0') {
            continue;
        }
        if (!is_only_pid) {
            auto tids = get_all_thread_id(pid);
            task_ids.emplace_back(pid, tids);
        } else {
            task_ids.emplace_back(pid, std::vector<uint64_t>());
        }
    }
    closedir(dir);
    close(dir_fd);
    return task_ids;
}

std::vector<uint64_t> Util::get_all_thread_id(uint64_t pid) {
    std::vector<uint64_t> tids;
    std::stringstream oss;
    oss << "/proc/" << pid << "/task";
    std::string pavaro_thread_path = oss.str();
    int dir_fd = openat(AT_FDCWD, pavaro_thread_path.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    if (dir_fd < 0) {
        std::cerr << "openat pavaro_thread_path: "
            << pavaro_thread_path << " failed, err: " << strerror(errno);
        return tids;
    }
    DIR* dir = fdopendir(dir_fd);
    if (!dir) {
        close(dir_fd);
        std::cerr << "fdopendir: " << dir_fd << " failed, err: " << strerror(errno);
        return tids;
    }
    // 读取 proc 文件系统中进程目录下所有线程的文件信息
    const struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过非目录
        if (entry->d_type != DT_DIR && entry->d_type != DT_UNKNOWN) {
            continue;
        }
        const char* name = entry->d_name;
        // RedHat 会使用点来隐藏线程，这里进行兼容
        if (name[0] == '.') {
            name++;
        }
        // 跳过名字为非数字的目录，任务的目录一定以数字（pid）命名
        if (name[0] < '0' || name[0] > '9') {
            continue;
        }
        // 文件的名字应该为一个数字，表明是线程的目录
        char* end_ptr = nullptr;
        uint64_t thread_id = strtoul(name, &end_ptr, 10);
        if (thread_id == 0 || thread_id == UINT64_MAX || *end_ptr != '\0') {
            continue;
        }
        tids.emplace_back(thread_id);
    }
    closedir(dir);
    close(dir_fd);
    return tids;
}

std::string Util::get_user_name(int user_id) {
    struct passwd* pwd = getpwuid(user_id);
    if (pwd && pwd->pw_name) {
        return std::string(pwd->pw_name);
    } else {
        return "<unknown>";
    }
}

uint64_t Util::get_steady_now_time_ns() {
    struct timespec tm;
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_nsec + tm.tv_sec * static_cast<uint64_t>(1E9);
}

}  // namespace iotop_cpp
