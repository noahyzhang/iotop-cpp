// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <linux/taskstats.h>
#include <easylogging++.h>
#include "src/netlink.h"

namespace iotop_cpp {

int Netlink::init() {
    if (init_sock() < 0) {
        return -1;
    }
    if (init_netlink_family_id() < 0) {
        return -1;
    }
    return 0;
}

int Netlink::init_sock() {
    struct sockaddr_nl addr;
    sock_fd_ = socket(PF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (sock_fd_ < 0) {
        LOG(ERROR) << "create socket failed";
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    if (bind(sock_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        // TODO 日志
        // 如果 bind 失败，退出前需要释放 sock_fd
        close(sock_fd_);
        LOG(ERROR) << "bind failed";
        return -2;
    }
    return 0;
}

int Netlink::init_netlink_family_id() {
    static std::string name(TASKSTATS_GENL_NAME);
    if (send_cmd(GENL_ID_CTRL, getpid(), CTRL_CMD_GETFAMILY, CTRL_ATTR_FAMILY_NAME, (void*)name.c_str(), name.size())) {
        return -1;
    }
    struct MsgTemplate ret_value;
    ssize_t rep_len = recv(sock_fd_, &ret_value, sizeof(ret_value), 0);
    if (rep_len < 0 || !NLMSG_OK((&ret_value.nl_msg), (size_t)rep_len) || ret_value.nl_msg.nlmsg_type == NLMSG_ERROR) {
        LOG(ERROR) << "recv failed";
        return -1;
    }
    struct nlattr* nl_attr = (struct nlattr*)(NLMSG_DATA(&ret_value) + GENL_HDRLEN);
    nl_attr = (struct nlattr*)((char*)nl_attr + NLA_ALIGN(nl_attr->nla_len));
    if (nl_attr->nla_type == CTRL_ATTR_FAMILY_ID) {
        netlink_family_id_ = *(uint16_t*)((char*)(nl_attr) + NLA_HDRLEN);
    }
    return 0;
}

int Netlink::send_cmd(uint16_t nl_msg_type, uint32_t nl_msg_pid,
    uint8_t genl_cmd, uint16_t nla_type, void* nla_data, int nla_len) {
    // 定义要发送的报文
    struct MsgTemplate msg;
    memset(&msg, 0, sizeof(msg));
    // 使用 netlink 提供的宏计算长度，Length of message including header
    msg.nl_msg.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    msg.nl_msg.nlmsg_type = nl_msg_type;
    // It is request message
    msg.nl_msg.nlmsg_flags = NLM_F_REQUEST;
    msg.nl_msg.nlmsg_seq = 0;
    msg.nl_msg.nlmsg_pid = nl_msg_pid;
    msg.genl_msg.cmd = genl_cmd;
    msg.genl_msg.version = 0x1;

    struct nlattr* nl_attr = (struct nlattr*)(NLMSG_DATA(&msg) + GENL_HDRLEN);
    nl_attr->nla_type = nla_type;
    nl_attr->nla_len = nla_len + NLA_HDRLEN;

    memcpy((void*)((char*)nl_attr + NLA_HDRLEN), nla_data, nla_len);
    msg.nl_msg.nlmsg_len += NLMSG_ALIGN(nl_attr->nla_len);

    char* buf = (char*)&msg;
    int buf_len = msg.nl_msg.nlmsg_len;

    struct sockaddr_nl nl_addr;
    memset(&nl_addr, 0, sizeof(nl_addr));
    nl_addr.nl_family = AF_NETLINK;
    ssize_t cur_send_byte = 0;
    while ((cur_send_byte = sendto(sock_fd_, buf,
        buf_len, 0, (struct sockaddr*)&nl_addr, sizeof(nl_addr))) < buf_len) {
        if (cur_send_byte > 0) {
            buf += cur_send_byte;
            buf_len -= cur_send_byte;
        } else {
            // EAGAIN: Try again
            if (errno == EAGAIN) {
                continue;
            }
            LOG(ERROR) << "sendto call failed";
            return -1;
        }
    }
    return 0;
}

} // namespace iotop_cpp
