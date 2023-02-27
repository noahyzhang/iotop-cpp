// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <iostream>
#include <unistd.h>
#include "iotop.h"

namespace iotop_cpp {

int IoRunner::init() {
    if (io_monitor_.init() < 0) {
        std::cerr << "io_monitor_ init failed";
        return -1;
    }
    if (view_.init() < 0) {
        std::cerr << "view_ init failed";
        return -2;
    }
    return 0;
}

void IoRunner::run() {
    for (;;) {
        if (io_monitor_.collect_timed() < 0) {
            continue;
        }
        auto sys_io_info = io_monitor_.get_io_stats_info();
        view_.draw_data(sys_io_info);
        sleep(1);
    }
}

}  // namespace iotop_cpp
