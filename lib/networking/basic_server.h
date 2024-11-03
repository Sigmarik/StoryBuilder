/**
 * @file basic_server.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Generic TCP/UDP server
 * @version 0.1
 * @date 2024-11-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <fcntl.h>

#include <functional>
#include <iostream>
#include <map>
#include <thread>

#include "basic_interface.h"

struct NetworkClientInfo {
    int socket = 0;
    sockaddr_in address{};
};

template <NetworkProtocol Protocol>
struct NetworkServer : public NetworkConnection<Protocol> {
    NetworkServer(in_port_t port);
    ~NetworkServer();

    void start_accepting(in_port_t local_port);
    void check_new_connections();
    void stop_accepting();

    using ClientId = int;

    template <class T>
    bool send_to(ClientId client, const T& content) {
        assert(errno == 0);

        if (!clients_.contains(client)) return {};

        NetworkConnection<Protocol>& client_conn = clients_[client];

        if (client_conn.is_dead()) {
            clients_.erase(client);
            on_client_disconnect(client);
            return {};
        }

        return client_conn.template send<T>(content);
    }

    template <class T>
    std::optional<T> receive_from(ClientId client) {
        assert(errno == 0);

        if (!clients_.contains(client)) return {};

        NetworkConnection<Protocol>& client_conn = clients_[client];

        if (client_conn.is_dead()) {
            clients_.erase(client);
            on_client_disconnect(client);
            return {};
        }

        return client_conn.template receive<T>();
    }

    bool is_alive(ClientId client) const {
        assert(errno == 0);

        return clients_.contains(client) && !clients_[client].is_dead();
    }

    void remove_dead() {
        assert(errno == 0);

        for (auto& [client, client_conn] : clients_) {
            if (!client_conn.is_dead()) continue;

            clients_.erase(client);
            on_client_disconnect(client);
        }

        assert(errno == 0);
    }

   protected:
    virtual void on_client_connect(ClientId client) {}
    virtual void on_client_disconnect(ClientId client) {}

   private:
    NetworkClientInfo accept_client();
    void setup_client(NetworkConnection<Protocol>& connection);

    std::map<ClientId, NetworkConnection<Protocol>> clients_{};

    std::jthread conn_listener_{};
    int local_sock_ = 0;

    NetworkConnection<Protocol> client_communicator_{};

    int local_server_ = 0;
};

template <NetworkProtocol Protocol>
inline void NetworkServer<Protocol>::start_accepting(in_port_t local_port) {
    assert(errno == 0);

    local_server_ = socket(AF_INET, SOCK_STREAM, 0);

    auto listen_for_conns = [=, this]() {
        assert(errno == 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(local_port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        bind(local_server_, (sockaddr*)&addr, sizeof(addr));

        assert(errno == 0);

        listen(local_server_, 1);

        assert(errno == 0);

        int loopback = accept(local_server_, nullptr, nullptr);

        assert(errno == 0);

        for (;;) {
            NetworkClientInfo client = accept_client();

            send(loopback, &client, sizeof(client), 0);

            if (errno != 0) break;
        }
    };

    conn_listener_ = std::jthread(listen_for_conns);

    local_sock_ = socket(AF_INET, SOCK_STREAM, 0);

    assert(errno == 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(local_port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sleep(1);

    assert(errno == 0);

    connect(local_sock_, (sockaddr*)&addr, sizeof(addr));

    fcntl(local_sock_, F_SETFL, fcntl(local_sock_, F_GETFL, 0) | O_NONBLOCK);

    assert(errno == 0);

    errno = 0;
}

template <NetworkProtocol Protocol>
inline void NetworkServer<Protocol>::check_new_connections() {
    assert(errno == 0);

    while (true) {
        assert(errno == 0);

        NetworkClientInfo client{};
        recv(local_sock_, &client, sizeof(client), 0);

        if (errno != 0) {
            errno = 0;
            break;
        }

        int client_id = client.socket;

        clients_[client_id] = NetworkConnection<Protocol>();
        NetworkConnection<Protocol>& client_conn = clients_[client_id];
        client_conn.sock_ = client.socket;
        client_conn.conn_addr_ = client.address;

        setup_client(client_conn);

        on_client_connect(client.socket);
    }

    assert(errno == 0);
}

template <NetworkProtocol Protocol>
inline void NetworkServer<Protocol>::stop_accepting() {
    assert(errno == 0);

    close(local_sock_);
    local_sock_ = 0;

    close(local_server_);
    local_server_ = 0;

    assert(errno == 0);
}

template <>
inline NetworkServer<NetworkProtocol::TCP>::NetworkServer(in_port_t port) {
    assert(errno == 0);

    sock_ = socket(AF_INET, SOCK_STREAM, 0);

    assert(errno == 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock_, (sockaddr*)&addr, sizeof(addr));

    listen(sock_, 16);

    assert(errno == 0);
}

template <>
inline NetworkServer<NetworkProtocol::UDP>::NetworkServer(in_port_t port) {
    assert(errno == 0);

    sock_ = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock_, (sockaddr*)&addr, sizeof(addr));

    client_communicator_.sock_ = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in& client_comm = client_communicator_.conn_addr_;

    client_comm.sin_family = AF_INET;
    client_comm.sin_port = 0;
    client_comm.sin_addr.s_addr = INADDR_ANY;

    bind(client_communicator_.sock_, (sockaddr*)&client_comm,
         sizeof(client_comm));

    assert(errno == 0);
}

template <>
NetworkServer<NetworkProtocol::UDP>::~NetworkServer() {
    assert(errno == 0);
    close(client_communicator_.sock_);
    assert(errno == 0);
}

template <>
NetworkServer<NetworkProtocol::TCP>::~NetworkServer() {
    assert(errno == 0);
}

template <>
inline NetworkClientInfo NetworkServer<NetworkProtocol::TCP>::accept_client() {
    assert(errno == 0);
    return (NetworkClientInfo){
        .socket = accept(sock_, nullptr, nullptr),
        .address = {},
    };
}

template <>
void NetworkServer<NetworkProtocol::TCP>::
    setup_client(NetworkConnection<NetworkProtocol::TCP>& connection) {}

template <>
void NetworkServer<NetworkProtocol::UDP>::
    setup_client(NetworkConnection<NetworkProtocol::UDP>& connection) {
    assert(errno == 0);
    connection.set_close_on_destroy(false);
}

template <>
inline NetworkClientInfo NetworkServer<NetworkProtocol::UDP>::accept_client() {
    assert(errno == 0);

    NetworkClientInfo client{};

    receive<uint16_t>();

    client.socket = client_communicator_.sock_;
    client.address = conn_addr_;

    send<uint16_t>(client_communicator_.conn_addr_.sin_port);

    conn_addr_ = (sockaddr_in){};

    assert(errno == 0);

    return client;
}
