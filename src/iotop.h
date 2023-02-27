// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include "collect/io_monitor.h"
#include "view/view.h"

namespace iotop_cpp {

class IoRunner {
public:
    IoRunner() = default;
    ~IoRunner() = default;

public:
    int init();
    void run();

private:
    // 数据来源
    IoMonitor io_monitor_;
    // 展示
    ViewCurses view_;
};

}  // namespace iotop_cpp
