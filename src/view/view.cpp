// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "common/common.h"
#include "common/util.h"
#include "cxxcurses/cxxcurses.hpp"
#include "view/view.h"

namespace iotop_cpp {

#define MAX_LINE_LENGTH 512

#define HEADER_LINE_FORMAT "Current DISK READ: %7.2lf B/s | Current DISK WRITE: %7.2lf B/s"
#define BODY_FIRST_LINE_FORMAT "%6s %4s %15s %15s %15s %15s   %15s     %s"
#define BODY_OTHER_LINE_FORMAT "%6i %4s %15s %15.2f %15.2f %15.2f %% %15.2f %%   %s"

// -------------------- class ViewCurses implement --------------------

int ViewCurses::init() {
    cxxcurses::terminal init;
    return 0;
}

void ViewCurses::draw_data(const SystemIoInfo& sys_io_info) {
    const auto& main_win = cxxcurses::terminal::main_win;
    print_header(sys_io_info.curr_disk_read_b_s, sys_io_info.curr_disk_write_b_s);
    print_body(sys_io_info.task_io_stats_table);
    main_win.draw();
    clear();
    sleep(1);
}

// Current DISK READ:       0.00 B/s | Current DISK WRITE:       0.00 B/s
// 头部如上例子
void ViewCurses::print_header(double curr_real_read, double curr_real_write) {
    const auto& main_win = cxxcurses::terminal::main_win;
    // 输出头部
    char buff[MAX_LINE_LENGTH];
    snprintf(buff, sizeof(buff), HEADER_LINE_FORMAT, curr_real_read, curr_real_write);
    main_win << cxxcurses::format(0, 0)(buff);
}

// TID  PRIO  USER     DISK READ  DISK WRITE  SWAPIN     IO>    COMMAND
//   1 ?sys root        0.00 B/s    0.00 B/s  0.00 %  0.00 % init
//   2 ?sys root        0.00 B/s    0.00 B/s  0.00 %  0.00 % [kthreadd]
// body 部分如上例子
void ViewCurses::print_body(const std::unordered_map<uint64_t, std::shared_ptr<TaskIoInfo>>& task_io_infos) {
    const auto& main_win = cxxcurses::terminal::main_win;
    size_t y_line = 1;
    // 先输出 body 的头部
    char header_line[MAX_LINE_LENGTH];
    snprintf(header_line, sizeof(header_line), BODY_FIRST_LINE_FORMAT,
        "TID", "PRIO", "USER", "DISK READ", "DISK WRITE", "SWAPIN", "IO", "COMMAND");
    main_win << cxxcurses::format(y_line++, 0)(header_line);
    // 再输出 body 的主体
    auto max_yx = main_win.max_yx();
    char body_line[MAX_LINE_LENGTH]{0};
    for (const auto& task : task_io_infos) {
        if (y_line - 1 >= static_cast<size_t>(max_yx.first)) {
            break;
        }
        snprintf(body_line, sizeof(body_line), BODY_OTHER_LINE_FORMAT,
            task.second->task_id, Util::convert_io_prio_to_str(task.second->io_priority), task.second->user_name,
            task.second->io_real_read_period_b_s, task.second->io_real_write_period_b_s,
            task.second->swapin_delay_period_percent, task.second->blkio_delay_period_percent,
            task.second->full_cmdline);
        main_win << cxxcurses::format(y_line++, 0)(body_line);
    }
}

}  // namespace iotop_cpp
