/**
 * @file main.cpp
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Main module of the program
 * @version 0.1
 * @date 2023-07-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "io/main_io.h"
#include "logger/debug.h"
#include "logger/logger.h"

#define MAIN

#include <ctime>

#include "client.h"
#include "config.h"
#include "server.h"
#include "utils/main_utils.h"

int main(const int argc, char** argv) {
    atexit(log_end_program);

    srand(std::time(0));

    Options options;

    set_logging_threshold(LOG_THRESHOLD);
    print_label();

    log_printf(STATUS_REPORTS, "status", "Initializing\n");

    if (argp_parse(&ARG_P, argc, argv, 0, 0, &options) != 0) {
        return EXIT_FAILURE;
    }

    if (options.is_server()) {
        if (options.is_udp()) {
            as_server<NetworkProtocol::UDP>();
        } else {
            as_server<NetworkProtocol::TCP>();
        }
    } else {
        if (options.is_udp()) {
            as_client<NetworkProtocol::UDP>();
        } else {
            as_client<NetworkProtocol::TCP>();
        }
    }

    return errno == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
