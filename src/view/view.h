// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include "cxxcurses/cxxcurses.hpp"

namespace iotop_cpp {

class ViewCurses {
public:
    void view_init();
    void view_loop();
    void view_finish();

private:
    // 展示头部信息（汇总数据）
    void print_header(uint64_t total_read, uint64_t total_write, uint64_t curr_read, uint64_t curr_write);
    // 展示 body 信息
    void print_body();
};

}  // namespace iotop_cpp
