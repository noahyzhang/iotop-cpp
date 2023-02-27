// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <getopt.h>

/**
 * 本文件提供 iotop-cpp 的参数
 * 参数提取、参数校验等功能
 */

namespace iotop_cpp {

// // 排序方式的参照
// enum {
//     SORT_BY_TID,
//     SORT_BY_PRIO,
//     SORT_BY_USER,
//     SORT_BY_READ,
//     SORT_BY_WRITE,
//     SORT_BY_SWAPIN,
//     SORT_BY_IO,
//     SORT_BY_GRAPH,
//     SORT_BY_COMMAND,
//     SORT_BY_MAX
// };

// // 排序方法
// enum {
//     SORT_DESC,
//     SORT_ASC
// };

// // 用户设置的参数标识
// typedef union {
//     struct _flags {
//         int only;
//         int batch_mode;
//         int processes;
//         int accumulated;
//         int kilobytes;
//         int timestamp;
//         int quiet;
//         int fullcmdline;
//         int sort_by;
//         int sort_order;
//     } flags;
//     int opts[10];
// } ArgFlags;

// // 默认的必须存在的参数
// typedef struct DefaultExistedParams {
//     int iter;
//     int delay;
//     int pid;
//     int user_id;
// } DefaultExistedParams;

// class Config {
//  public:
//     ~Config() = default;
//     Config(const Config&) = delete;
//     Config& operator=(const Config&) = delete;
//     Config(Config&&) = delete;
//     Config& operator=(Config&&) = delete;

//     static Config& get_instance() {
//         static Config conf;
//         return conf;
//     }
//     void parse_args(int argc, void* argv[]);

//  private:
//     Config() {
//         config_.flags.sort_by = SORT_BY_GRAPH;
//         config_.flags.sort_order = SORT_DESC;
//     }
//     static void print_help();

//  private:
//     ArgFlags config_{0};  // 当前被设置的参数
//     DefaultExistedParams default_existed_params_{-1, 1, -1, -1};  // 默认存在的参数
//     // 用于解析命令行
//     // 尽量保持与官方的 iotop 的参数选项一致
//     // 官方的 iotop 是 python 语言的。地址：http://guichaz.free.fr/iotop/
//     struct option long_options_[] = {
//         {"version", no_argument, nullptr, 'v'},  // 版本号。无参数
//         {"help", no_argument, nullptr, 'h'},  // 帮助手册。无参数
//         {"only", no_argument, nullptr, 'o'},  // 只显示实际执行I/O的进程或线程。无参数
//         {"batch", no_argument, nullptr, 'b'},  // 无交互模式。无参数
//         {"iter", required_argument, nullptr, 'n'},  // 结束前的刷新次数，默认为无限次。一个参数
//         {"delay", required_argument, nullptr, 'd'},  // 刷新的时间间隔，默认为1秒。一个参数
//         {"pid", required_argument, nullptr, 'p'},  // 监控指定的进程或线程，默认为全部。一个参数
//         {"user", required_argument, nullptr, 'u'},  // 监控指定用户下的进程或线程，默认为全部。一个参数
//         {"process", no_argument, nullptr, 'P'},  // 仅仅显示进程，无参数
//         {"accumulated", no_argument, nullptr, 'a'},  // 显示累计 IO 数值，而不是带宽。无参数
//         {"kilobytes", no_argument, nullptr, 'k'},  // 使用 KB 作为单位。无参数
//         {"timestamp", no_argument, nullptr, 't'},  // 在每一行添加时间戳，会进入无交互模式。无参数
//         {"quiet", no_argument, nullptr, 'q'},  // 抑制某些标题行，会进入无交互模式。无参数
//         {"fullcmdline", no_argument, nullptr, 'c'},  // 显示进程或线程的全路径。无参数
//         {nullptr, 0, nullptr, 0}
//     };
// };

}  // namespace iotop_cpp
