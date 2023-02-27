// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <sys/socket.h>
#include <unistd.h>
#include <linux/taskstats.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <iostream>
#include "collect/netlink.h"

namespace iotop_cpp {

int NetlinkIoStats::get_task_io_info(uint64_t task_id, std::shared_ptr<TaskIoInfo> io_stats) {
    // 获取 taskstats 中的数据
    int res = send_netlink_cmd(netlink_family_id_, task_id, TASKSTATS_CMD_GET,
                                 TASKSTATS_CMD_ATTR_PID, &task_id, sizeof(task_id));
    if (__glibc_unlikely(res < 0)) {
        std::cerr << "send_netlink_cmd failed, errno: " << res << std::endl;
        return -2;
    }
    // 接收内核返回的数据
    struct NetlinkMsgTemplate msg;
    ssize_t rep_len = recv(sock_fd_, &msg, sizeof(msg), 0);
    if (rep_len < 0) {
        std::cerr << "recv failed, errno: " << errno << ", " << strerror(errno);
        return -3;
    }
    if (!NLMSG_OK((&msg.nl_msg), (size_t)rep_len)) {
        std::cerr << "recv failed, ret netlink value err" << std::endl;
        return -4;
    }
    if (msg.nl_msg.nlmsg_type == NLMSG_ERROR) {
        // struct nlmsgerr *err = reinterpret_cast<struct nlmsgerr*>(NLMSG_DATA(&msg));
        // if (err->error != -ESRCH) {
        //     std::cerr << "recv reply failed, task_id: " << task_id << ", errno: " << err->error << std::endl;
        // }
        return -5;
    }
    rep_len = GENLMSG_PAYLOAD(&msg.nl_msg);
    struct nlattr *na = reinterpret_cast<struct nlattr*>(GENLMSG_DATA(&msg));
    int len = 0;
    while (len < rep_len) {
        len += NLA_ALIGN(na->nla_len);
        if (na->nla_type == TASKSTATS_TYPE_AGGR_TGID || na->nla_type == TASKSTATS_TYPE_AGGR_PID) {
            int aggr_len = NLA_PAYLOAD(na->nla_len);
            int len2 = 0;

            na = reinterpret_cast<struct nlattr*>(NLA_DATA(na));
            while (len2 < aggr_len) {
                if (na->nla_type == TASKSTATS_TYPE_STATS) {
                    struct taskstats *ts = reinterpret_cast<struct taskstats*>(NLA_DATA(na));
                    io_stats->user_id = ts->ac_uid;
                    io_stats->io_read_char_b = ts->read_char;
                    io_stats->io_write_char_b = ts->write_char;
                    io_stats->io_real_read_b = ts->read_bytes;
                    io_stats->io_real_write_b = ts->write_bytes;
                    io_stats->swapin_delay_total_ns = ts->swapin_delay_total;
                    io_stats->blkio_delay_total_ns = ts->blkio_delay_total;
                }
                len2 += NLA_ALIGN(na->nla_len);
                na = reinterpret_cast<struct nlattr*>(reinterpret_cast<char *>(na) + len2);
            }
        }
        na = reinterpret_cast<struct nlattr*>(reinterpret_cast<char *>(GENLMSG_DATA(&msg)) + len);
    }
    return 0;
}

int NetlinkIoStats::init() {
    // 必须先初始化 sock_fd_，再初始化 netlink_family_id_
    if (init_sock() < 0) {
        return -1;
    }
    if (init_netlink_family_id() < 0) {
        return -2;
    }
    // save_statistical_info_size_ = get_param("resource_detail_size", 0);
    return 0;
}

int NetlinkIoStats::init_sock() {
    struct sockaddr_nl addr;
    sock_fd_ = socket(PF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (sock_fd_ < 0) {
        std::cerr << "create socket failed, errno: " << errno << ", " << strerror(errno) << std::endl;
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    if (bind(sock_fd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        // 如果 bind 失败，退出前需要释放 sock_fd_
        close(sock_fd_);
        std::cerr << "bind failed, errno: " << errno << ", " << strerror(errno) << std::endl;
        return -2;
    }
    int flags = fcntl(sock_fd_, F_GETFL, 0);
    if (fcntl(sock_fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "fcntl failed, errno: " << errno << ", " << strerror(errno) << std::endl;
        return -3;
    }
    return 0;
}

int NetlinkIoStats::init_netlink_family_id() {
    static std::string name(TASKSTATS_GENL_NAME);
    auto res = send_netlink_cmd(GENL_ID_CTRL, getpid(), CTRL_CMD_GETFAMILY, CTRL_ATTR_FAMILY_NAME,
                    reinterpret_cast<void*>(const_cast<char*>(name.c_str())), name.size() + 1);
    if (res < 0) {
        std::cerr << "send_netlink_cmd failed, errno: " << res << std::endl;
        return -1;
    }
    struct NetlinkMsgTemplate ret_value;
    ssize_t rep_len = recv(sock_fd_, &ret_value, sizeof(ret_value), 0);
    if (rep_len < 0) {
        std::cerr << "recv failed, errno: " << errno << ", " << strerror(errno) << std::endl;
        return -2;
    }
    if (!NLMSG_OK((&ret_value.nl_msg), (size_t)rep_len)) {
        std::cerr << "recv failed, ret netlink value err" << std::endl;
        return -3;
    }
    if (ret_value.nl_msg.nlmsg_type == NLMSG_ERROR) {
        std::cerr << "recv failed, ret value nl_msg_type is NLMSG_ERROR" << std::endl;
        return -4;
    }
    struct nlattr *nl_attr = reinterpret_cast<struct nlattr*>(
        reinterpret_cast<char*>(NLMSG_DATA(&ret_value)) + GENL_HDRLEN);
    nl_attr = reinterpret_cast<struct nlattr*>((reinterpret_cast<char*>(nl_attr) + NLA_ALIGN(nl_attr->nla_len)));
    if (nl_attr->nla_type != CTRL_ATTR_FAMILY_ID) {
        std::cerr << "recv failed, ret value nla_type unequal to CTRL_ATTR_FAMILY_ID" << std::endl;
        return -5;
    }
    netlink_family_id_ = *(reinterpret_cast<uint16_t*>((reinterpret_cast<char*>(nl_attr) + NLA_HDRLEN)));
    if (netlink_family_id_ == 0) {
        std::cerr << "get netlink family id is 0, this is valid value" << std::endl;
        return -6;
    }
    return 0;
}

int NetlinkIoStats::send_netlink_cmd(uint16_t nl_msg_type, uint32_t nl_msg_pid,
                      uint8_t genl_cmd, uint16_t nla_type, void *nla_data, int nla_len) {
    // 定义要发送的报文
    struct NetlinkMsgTemplate msg;
    memset(&msg, 0, sizeof(msg));
    memset(msg.buf, 0, sizeof(msg.buf));
    // 使用 netlink 提供的宏计算长度
    msg.nl_msg.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    msg.nl_msg.nlmsg_type = nl_msg_type;
    msg.nl_msg.nlmsg_flags = NLM_F_REQUEST;
    msg.nl_msg.nlmsg_seq = 0;
    msg.nl_msg.nlmsg_pid = nl_msg_pid;
    msg.genl_msg.cmd = genl_cmd;
    // 填充版本号，linux/taskstats.h 中定义
    msg.genl_msg.version = TASKSTATS_GENL_VERSION;

    struct nlattr *nl_attr = reinterpret_cast<struct nlattr*>(GENLMSG_DATA(&msg));
    nl_attr->nla_type = nla_type;
    nl_attr->nla_len = nla_len + NLA_HDRLEN;
    memcpy(NLA_DATA(nl_attr), nla_data, nla_len);
    msg.nl_msg.nlmsg_len += NLMSG_ALIGN(nl_attr->nla_len);

    char *buf = reinterpret_cast<char*>(&msg);
    int buf_len = msg.nl_msg.nlmsg_len;

    struct sockaddr_nl nl_addr;
    memset(&nl_addr, 0, sizeof(nl_addr));
    nl_addr.nl_family = AF_NETLINK;
    ssize_t cur_send_byte = 0;
    while ((cur_send_byte = sendto(sock_fd_, buf, buf_len, 0, reinterpret_cast<struct sockaddr*>(&nl_addr),
        sizeof(nl_addr))) < buf_len) {
        if (cur_send_byte > 0) {
            buf += cur_send_byte;
            buf_len -= cur_send_byte;
        } else {
            // EAGAIN: Try again
            if (errno != EAGAIN) {
                std::cerr << "sendto call failed, errno: " << errno << ", " << strerror(errno) << std::endl;
                return -1;
            }
        }
    }
    return 0;
}

}  // namespace iotop_cpp
