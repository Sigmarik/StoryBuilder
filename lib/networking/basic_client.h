/**
 * @file basic_client.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Generic TCP/UDP client
 * @version 0.1
 * @date 2024-11-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "basic_interface.h"

template <NetworkProtocol Protocol>
struct NetworkClient : public NetworkConnection<Protocol> {
    NetworkClient(in_addr_t server_addr, in_port_t port);
};
