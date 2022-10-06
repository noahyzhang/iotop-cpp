// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include "src/config.h"

int main(int argc, char* argv[]) {
    // 解析命令行参数
    iotop_cpp::Config::get_instance().parse_args(argc, argv);
    // 初始化
    

}