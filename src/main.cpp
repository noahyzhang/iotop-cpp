// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <iostream>
#include "iotop.h"

int main() {
    // // 解析命令行参数
    // iotop_cpp::Config::get_instance().parse_args(argc, argv);
    // // 初始化
    iotop_cpp::IoRunner io_runner;
    if (io_runner.init() < 0) {
        std::cerr << "init failed";
        return -1;
    }
    io_runner.run();
    return 0;
}
