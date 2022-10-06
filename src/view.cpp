// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <iostream>
#include "src/util.h"
#include "src/view.h"

namespace iotop_cpp {

// -------------------- class ViewBatch implement --------------------

void ViewBatch::view_init() override {
    if (Util::read_task_delayacct() == 0) {
        std::cout << "Warning: task_delayacct is 0, enable by: echo 1 > /proc/sys/kernel/task_delayacct" << std::endl;
    }
}

void ViewBatch::view_loop() override {
    for (;;) {
        
    }
}

void ViewBatch::view_finish() override {}


// -------------------- class ViewCurses implement --------------------

void ViewCurses::view_init() override {
    
}

void ViewCurses::view_loop() override {

}

void ViewCurses::view_finish() override {

}

};  // namespace iotop_cpp
