// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

namespace iotop_cpp {

class View {
 public:
    virtual void view_init();
    virtual void view_loop();
    virtual void view_finish();
};

class ViewBatch : public View {
 public:
    void view_init() override;
    void view_loop() override;
    void view_finish() override;
};

class ViewCurses : public View {
 public:
    void view_init() override;
    void view_loop() override;
    void view_finish() override;
};

}  // namespace iotop_cpp
