## iotop-cpp

因为工作需要统计线程级别的 IO 使用情况，经过调研，发现 iotop 工具是一款较为理想的统计 IO 的工具。

iotop 是一款开源、免费的用来监控磁盘 IO 使用状况的类似 top 命令的工具，iotop 可以监控线程级别的 IO 信息。它是 Python 语言编写的，与 iostat 工具比较，iostat 是系统级别的 IO 监控，而 iotop 是线程级别 IO 监控。

值得一提的是 iotop 内部使用 netlink 方式获取相关的 IO 信息。因此打算使用 cpp 实现一下 iotop，用于学习
