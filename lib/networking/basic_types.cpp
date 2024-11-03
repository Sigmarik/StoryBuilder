/**
 * @file basic_types.cpp
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Implementation of TCP/UDP send/receive functions for basic types.
 * @version 0.1
 * @date 2024-11-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>

#include <iostream>
#include <string>

#include "basic_interface.h"

//* ========= TCP =========

#define TCP_SENDER(TYPE)                           \
    template <>                                    \
    template <>                                    \
    bool NetworkConnection<NetworkProtocol::TCP>:: \
        send<TYPE>(const TYPE& content)

#define TCP_RECEIVER(TYPE)                                        \
    template <>                                                   \
    template <>                                                   \
    std::optional<TYPE> NetworkConnection<NetworkProtocol::TCP>:: \
        receive<TYPE>()

TCP_SENDER(uint16_t) {
    uint16_t data = htons(content);
    return send_raw(&data, sizeof(data), 0);
}

TCP_RECEIVER(uint16_t) {
    uint16_t result = 0;
    bool success = recv_raw(&result, sizeof(result), 0);
    result = ntohs(result);

    if (success) return result;

    return {};
}

TCP_SENDER(uint32_t) {
    uint32_t data = htonl(content);
    return send_raw(&data, sizeof(data), 0);
}

TCP_RECEIVER(uint32_t) {
    uint32_t result = 0;
    bool success = recv_raw(&result, sizeof(result), 0);
    result = ntohl(result);

    if (success) return result;

    return {};
}

TCP_SENDER(int16_t) { return send((uint16_t)content); }
TCP_RECEIVER(int16_t) {
    auto result = receive<uint16_t>();
    return result;
}

TCP_SENDER(int32_t) { return send((uint32_t)content); }
TCP_RECEIVER(int32_t) {
    auto result = receive<uint32_t>();
    return result;
}

TCP_SENDER(std::string) {
    if (dead_) return false;

    bool length_send_success = send<uint32_t>((uint32_t)content.size());
    if (!length_send_success) return false;

    return send_raw(content.c_str(), content.size(), 0);
}

TCP_RECEIVER(std::string) {
    if (dead_) return {};

    auto length = receive<uint32_t>();
    if (!length) return {};

    char* buffer = new char[*length + 1];
    buffer[*length] = 0;
    bool success = recv_raw(buffer, *length, 0);
    std::string result = buffer;
    delete[] buffer;

    if (success) return result;

    return {};
}

//* ========= UDP =========

#define UDP_SENDER(TYPE)                           \
    template <>                                    \
    template <>                                    \
    bool NetworkConnection<NetworkProtocol::UDP>:: \
        send<TYPE>(const TYPE& content)

#define UDP_RECEIVER(TYPE)                                        \
    template <>                                                   \
    template <>                                                   \
    std::optional<TYPE> NetworkConnection<NetworkProtocol::UDP>:: \
        receive<TYPE>()

UDP_SENDER(uint16_t) {
    uint16_t data = htons(content);
    return send_raw(&data, sizeof(data), 0);
}

UDP_RECEIVER(uint16_t) {
    uint16_t result = 0;
    bool success = recv_raw(&result, sizeof(result), 0);
    result = ntohs(result);

    if (success) return result;

    return {};
}

UDP_SENDER(uint32_t) {
    uint32_t data = htonl(content);
    return send_raw(&data, sizeof(data), 0);
}

UDP_RECEIVER(uint32_t) {
    uint32_t result = 0;
    bool success = recv_raw(&result, sizeof(result), 0);
    result = ntohl(result);

    if (success) return result;

    return {};
}

UDP_SENDER(int16_t) { return send((uint16_t)content); }
UDP_RECEIVER(int16_t) {
    auto result = receive<uint16_t>();
    return result;
}

UDP_SENDER(int32_t) { return send((uint32_t)content); }
UDP_RECEIVER(int32_t) {
    auto result = receive<uint32_t>();
    return result;
}

static const size_t UDP_OPTIMAL_SIZE = 534;  // bytes

static size_t get_chunk_count(size_t msg_size) {
    return (msg_size + UDP_OPTIMAL_SIZE - 1) / UDP_OPTIMAL_SIZE;
}

static size_t min(size_t alpha, size_t beta) {
    return alpha ? alpha <= beta : beta;
}

UDP_SENDER(std::string) {
    if (dead_) return false;

    size_t length = content.size();
    size_t chunk_count = get_chunk_count(length);

    bool length_send_status = send<uint32_t>((uint32_t)length);
    if (!length_send_status) return false;

    for (size_t chunk_id = 0; chunk_id < chunk_count; ++chunk_id) {
        size_t start = UDP_OPTIMAL_SIZE * chunk_id;
        size_t end = min(start + UDP_OPTIMAL_SIZE, length);

        bool status = false;
        while (!status && !is_dead()) {
            status = send_raw(content.c_str() + start, end - start, 0);
        }

        if (is_dead()) return false;
    }

    return true;
}

UDP_RECEIVER(std::string) {
    if (dead_) return {};

    auto length = receive<uint32_t>();
    if (!length) return {};

    size_t chunk_count = get_chunk_count(*length);
    char* buffer = new char[*length + 1];
    buffer[*length] = 0;

    for (size_t chunk_id = 0; chunk_id < chunk_count; ++chunk_id) {
        size_t start = UDP_OPTIMAL_SIZE * chunk_id;
        size_t end = min(start + UDP_OPTIMAL_SIZE, *length);

        bool status = false;
        while (!status && !is_dead()) {
            status = recv_raw(buffer + start, end - start, 0);
        }

        if (is_dead()) return {};
    }

    std::string result = buffer;
    delete[] buffer;

    return result;
}
