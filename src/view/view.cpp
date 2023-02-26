// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include "ncurses.h"
#include <iostream>
#include "util.h"
#include "view.h"

namespace iotop_cpp {

#define HEADER_FIRST_LINE_FORMAT "  Total DISK READ: %7.2f %s |   Total DISK WRITE: %7.2f %s"
#define HEADER_SECOND_LINE_FORMAT "Current DISK READ: %7.2f %s | Current DISK WRITE: %7.2f %s"
#define BODY_FIRST_LINE_FORMAT "%6s %4s %8s %11s %11s %6s %6s %s"
#define BODY_OTHER_LINE_FORMAT "%6i %4s %s %7.2f %-3.3s %7.2f %-3.3s %2.2f %% %2.2f %% %s"

// -------------------- class ViewCurses implement --------------------

void ViewCurses::view_init() {
    cxxcurses::terminal init;
}

void ViewCurses::view_loop() {

}

void ViewCurses::view_finish() {

}

// Total DISK READ:         0.00 B/s | Total DISK WRITE:         0.00 B/s
// Current DISK READ:       0.00 B/s | Current DISK WRITE:       0.00 B/s
// 头部如上例子
void ViewCurses::print_header(uint64_t total_read, uint64_t total_write, uint64_t curr_read, uint64_t curr_write) {
    const auto& main_win = cxxcurses::terminal::main_win;

    char buff[1024];
    snprintf(buff, sizeof(buff), HEADER_FIRST_LINE_FORMAT, total_read, total_write);
    main_win << cxxcurses::format(0, 0)(buff);

    char buff2[1024];
    snprintf(buff2, sizeof(buff2), HEADER_SECOND_LINE_FORMAT, curr_read, curr_write);
    main_win << cxxcurses::format(1, 0)(buff2);
}

// TID  PRIO  USER     DISK READ  DISK WRITE  SWAPIN     IO>    COMMAND
//   1 ?sys root        0.00 B/s    0.00 B/s  0.00 %  0.00 % init
//   2 ?sys root        0.00 B/s    0.00 B/s  0.00 %  0.00 % [kthreadd]
// body 部分如上例子
void ViewCurses::print_body() {

}

}  // namespace iotop_cpp
