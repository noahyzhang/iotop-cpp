// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <iostream>
#include "src/util.h"
#include "src/view.h"

namespace iotop_cpp {

// -------------------- class ViewBatch implement --------------------

void ViewBatch::view_init()  {
    if (Util::read_task_delayacct() == 0) {
        std::cout << "Warning: task_delayacct is 0, enable by: echo 1 > /proc/sys/kernel/task_delayacct" << std::endl;
    }
}

void ViewBatch::view_loop()  {
    for (;;) {
        
    }
}

void ViewBatch::view_finish() {}


// -------------------- class ViewCurses implement --------------------

void ViewCurses::view_init() {
    
}

void ViewCurses::view_loop() {

}

void ViewCurses::view_finish() {

}

};  // namespace iotop_cpp
