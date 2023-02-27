// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <pwd.h>
#include <iostream>
#include "config.h"

namespace iotop_cpp {

// void Config::parse_args(int argc, void* argv[]) {
//     while (true) {
//         int opt = getopt_long(argc, argv, "vhobn:d:p:u:Paktqc", long_options_, nullptr);
//         if (opt < 0) {
//             if (optind < argc) {
//                 for (int i = optind; i < argc; ++i) {
//                     std::cerr << "iotop unknown argument: " << argv[i];
//                 }
//                 exit(EXIT_FAILURE);
//             }
//             break;
//         }
//         switch (opt) {
//         case 'v':
//             std::cout << "iotop " << VERSION << std::endl;
//             exit(EXIT_SUCCESS);
//         case 'h':
//             print_help();
//             exit(EXIT_SUCCESS);
//         case 'o':
//             config_.flags.only = 1;
//             break;
//         case 'b':
//             config_.flags.batch_mode = 1;
//             break;
//         case 'P':
//             config_.flags.processes = 1;
//             break;
//         case 'a':
//             config_.flags.accumulated = 1;
//             break;
//         case 'k':
//             config_.flags.kilobytes = 1;
//             break;
//         case 't':
//             config_.flags.timestamp = 1;
//             config_.flags.batch_mode = 1;
//             break;
//         case 'q':
//             config_.flags.quiet = 1;
//             config_.flags.batch_mode = 1;
//             break;
//         case 'c':
//             config_.flags.fullcmdline = 1;
//             break;
//         case 'n':
//             default_existed_params_.iter = std::atoi(optarg);
//             break;
//         case 'd':
//             default_existed_params_.delay = std::atoi(optarg);
//             break;
//         case 'p':
//             default_existed_params_.pid = std::atoi(optarg);
//             break;
//         case 'u':
//             if (optarg[0] == '+') {
//                 default_existed_params_.user_id = std::atoi(optarg + 1);
//             } else {
//                 struct passwd pwd;
//                 struct passwd *result;
//                 size_t buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
//                 if (buf_size == -1) {
//                     buf_size = 16384;
//                 }
//                 char* buf = malloc(buf_size);
//                 int res = getpwnam_r(optarg, &pwd, buf, buf_size, &result);
//                 if (result == nullptr) {
//                     if (res == 0) {
//                         if (isdigit(optarg[0])) {
//                             default_existed_params_.user_id = std::atoi(optarg);
//                             break;
//                         }
//                         std::cerr << "iotop: user " << optarg << " not found" << std::endl;
//                         exit(EXIT_FAILURE);
//                     } else {
//                         std::cerr << "iotop: getpwnam_r error: " << strerr(res) << std::endl;
//                         exit(EXIT_FAILURE);
//                     }
//                 }
//                 default_existed_params_.user_id = pwd.pw_uid;
//             }
//             break;
//         default:
//             exit(EXIT_FAILURE);
//         }
//     }
// }

// void Config::print_help() {
//     std::cout << "Usage: iotop [OPTION]" << std::endl
//               << std::endl
//               << "DISK READ and DISK WRITE are the block I/O bandwidth used during the sampling" << std::endl
//               << "period. SWAPIN and IO are the percentages of time the thread spent respectively" << std::endl
//               << "while swapping in and waiting on I/O more generally. PRIO is the I/O priority" << std::endl
//               << "at which the thread is running (set using the ionice command)." << std::endl
//               << std::endl
//               << "Controls: left and right arrows to change the sorting column, r to invert the" << std::endl
//               << "sorting order, o to toggle the --only option, p to toggle the --processes" << std::endl
//               << "option, a to toggle the --accumulated option, i to change I/O priority, q to" << std::endl
//               << "quit, any other key to force a refresh" << std::endl;
// }

};  // namespace iotop_cpp
