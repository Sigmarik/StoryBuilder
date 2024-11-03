#include "client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "config.h"
#include "logger/debug.h"
#include "logger/logger.h"

static in_addr_t get_address() {
    in_addr_t address = 0;

    do {
        std::string server_addr_string;

        std::cout << "Server address:\n" << INPUT_PREFIX;
        std::cin >> server_addr_string;

        address = inet_addr(server_addr_string.c_str());
        if (address == (in_addr_t)(-1)) {
            address = 0;
            std::cout << "Cannot resolve IPv4 address from the input provided. "
                         "Please use standard IPv4 notation (P1.P2.P3.P4)."
                      << std::endl;
        } else if (address == 0) {
            std::cout << "Server address cannot be 0.0.0.0." << std::endl;
        }
    } while (address == 0);

    return address;
}

int as_client() { return 0; }