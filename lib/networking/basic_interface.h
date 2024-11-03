/**
 * @file basic_interface.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Basic network connection structure.
 * @version 0.1
 * @date 2024-11-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <optional>

#include "protocols.h"

template <NetworkProtocol Protocol>
struct NetworkServer;

template <NetworkProtocol Protocol>
struct NetworkClient;

template <NetworkProtocol Protocol>
struct NetworkConnection {
    NetworkConnection() = default;
    virtual ~NetworkConnection() {
        if (close_on_destroy_) close(sock_);
    }

    NetworkConnection(const NetworkConnection&) = delete;
    NetworkConnection(NetworkConnection&&) = default;

    NetworkConnection& operator=(const NetworkConnection&) = delete;
    NetworkConnection& operator=(NetworkConnection&&) = default;

    bool is_dead() const { return dead_; }

    template <class T>
    bool send(const T& content);

    template <class T>
    std::optional<T> receive();

    friend struct NetworkServer<Protocol>;
    friend struct NetworkClient<Protocol>;

    void set_close_on_destroy(bool value) { close_on_destroy_ = value; }

   protected:
    int sock_ = 0;
    sockaddr_in conn_addr_{};

    void die() { dead_ = true; }

   private:
    bool dead_ = false;

    bool send_raw(const void* buffer, size_t len, int flags);
    bool recv_raw(void* buffer, size_t len, int flags);

    bool should_die();

    bool close_on_destroy_ = true;
};

template <NetworkProtocol Protocol>
ssize_t sys_send(int sock_fd, const void* buf, size_t len, int flags,
                 sockaddr_in address);

template <NetworkProtocol Protocol>
ssize_t sys_recv(int sock_fd, void* buf, size_t len, int flags,
                 sockaddr_in address);

template <NetworkProtocol Protocol>
inline bool NetworkConnection<Protocol>::
    send_raw(const void* buffer, size_t len, int flags) {
    if (dead_) return false;
    if (errno != 0) return false;

    sys_send<Protocol>(sock_, buffer, len, flags, conn_addr_);

    if (errno == 0) return true;

    if (should_die()) {
        dead_ = true;
    }

    errno = 0;

    return false;
}

template <NetworkProtocol Protocol>
inline bool NetworkConnection<Protocol>::
    recv_raw(void* buffer, size_t len, int flags) {
    if (dead_) return false;
    if (errno != 0) return false;

    sys_recv<Protocol>(sock_, buffer, len, flags, conn_addr_);

    if (errno == 0) return true;

    if (should_die()) {
        dead_ = true;
    }

    errno = 0;

    return false;
}

template <NetworkProtocol Protocol>
inline bool NetworkConnection<Protocol>::should_die() {
    bool should_live =
        errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOMEM;
    return !should_live;
}
