// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <stdint.h>
#include <memory>
#include <unordered_map>

namespace iotop_cpp {

class ViewCurses {
public:
    ViewCurses() = default;
    ~ViewCurses() = default;

public:
    int init();
    void draw_data(const SystemIoInfo& sys_io_info);

private:
    // 展示头部信息（汇总数据）
    void print_header(double curr_real_read, double curr_real_write);
    // 展示 body 信息
    void print_body(const std::unordered_map<uint64_t, std::shared_ptr<TaskIoInfo>>& task_io_infos);
};

}  // namespace iotop_cpp
