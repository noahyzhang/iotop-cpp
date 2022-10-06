// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <linux/taskstats.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <easylogging++.h>
#include "src/iotop.h"
#include "src/util.h"

namespace iotop_cpp {

int IoInfo::init() {
    return netlink_.init();
}

int IoInfo::set_io_rw_stat(pid_t pid, pid_t tid, std::shared_ptr<struct IoStats> stats) {
    int sock_fd = netlink_.get_sock_fd();
    uint16_t netlink_family_id = netlink_.get_netlink_family_id();
    if (sock_fd < 0 || netlink_family_id < 0) {
        LOG(ERROR) << "netlink sock_fd or family_id is invalid";
        return -1;
    }
    if (netlink_.send_cmd(netlink_family_id, tid, TASKSTATS_CMD_GET, TASKSTATS_CMD_ATTR_PID,
                          reinterpret_cast<void*>(&tid), sizeof(tid))) {
        LOG(ERROR) << "send_cmd failed";
        return -1;
    }
    stats->pid = pid;
    stats->tid = tid;

    struct MsgTemplate msg;
    ssize_t rv = recv(sock_fd, &msg, sizeof(msg), 0);
    if (rv < 0 || !NLMSG_OK((&msg.nl_msg), (size_t)rv) || msg.nl_msg.nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr* err = (struct nlmsgerr*)(NLMSG_DATA(&msg));
        if (err->error != ESRCH) {
            LOG(ERROR) << "recv failed, errno: " << strerror(errno);
        }
        return -1;
    }
    rv = (NLMSG_PAYLOAD(&msg.nl_msg, 0) - GENL_HDRLEN);
    struct nlattr* nl_attr = (struct nlattr*)( reinterpret_cast<char*>(NLMSG_DATA(&msg)) + GENL_HDRLEN);
    int len = 0;
    while (len < rv) {
        len += NLA_ALIGN(nl_attr->nla_len);
        if (nl_attr->nla_type == TASKSTATS_TYPE_AGGR_TGID || nl_attr->nla_type == TASKSTATS_TYPE_AGGR_PID) {
            int aggr_len = (nl_attr->nla_len) - NLA_HDRLEN;
            int curr_copy_size = 0;
            nl_attr = (struct nlattr*)(reinterpret_cast<char*>(nl_attr) + NLA_HDRLEN);
            while (curr_copy_size < aggr_len) {
                if (nl_attr->nla_type == TASKSTATS_TYPE_STATS)  {
                    struct taskstats* task_stats = (struct taskstats*)(reinterpret_cast<char*>(nl_attr) + NLA_HDRLEN);
                    stats->read_bytes = task_stats->read_bytes;
                    stats->write_bytes = task_stats->write_bytes;
                    stats->swapin_delay_total = task_stats->swapin_delay_total;
                    stats->blkio_delay_total = task_stats->blkio_delay_total;
                    stats->user_id = task_stats->ac_uid;
                }
                curr_copy_size += NLA_ALIGN(nl_attr->nla_len);
                nl_attr = (struct nlattr*)(reinterpret_cast<char*>(nl_attr) + curr_copy_size);
            }
        }
        nl_attr = (struct nlattr*)(reinterpret_cast<char*>(
            (reinterpret_cast<char*>(NLMSG_DATA(&msg)) + GENL_HDRLEN)) + len);
    }
    return 0;
}

int IoInfo::set_io_basic_stat(pid_t task, std::shared_ptr<struct IoStats> stats) {
    // 设置 io 的优先级
    auto io_priority = std::make_shared<int>();
    if (Util::get_task_io_priority(task, io_priority) < 0) {
        // stats->is_valid_value = false;
        stats->io_priority = 0;
    } else {
        stats->io_priority = *io_priority;
    }
    // 设置 cmdline
    auto cmdline = std::make_shared<std::string>();
    // 获取短的 cmdline
    if (Util::get_task_cmdline(task, true, cmdline) < 0) {
        stats->short_cmdline = "<unknown>";
    } else {
        stats->short_cmdline = *cmdline;
    }
    // 获取完整的 cmdline
    cmdline->clear();
    if (Util::get_task_cmdline(task, false, cmdline) < 0) {
        stats->full_cmdline = "<unknown>";
    } else {
        stats->full_cmdline = *cmdline;
    }
    // // 检测是否有非法值
    // if (!stats->is_valid_value) {
    //     return -1;
    // }
    return 0;
}

}  // namespace iotop_cpp
